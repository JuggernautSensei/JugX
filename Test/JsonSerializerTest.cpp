// JsonSerializer / JsonDeserializer + JsonWriter / JsonReader 동작 검증용 테스트 (Google Test)
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <type_traits>
#include <vector>

#include <gtest/gtest.h>

#include "../Source/JsonDeserializer.h"
#include "../Source/JsonSerializer.h"

using namespace jug;

// ==========================================================
//  설계 불변식 - 값 핸들은 아무것도 소유하지 않는 포인터 뭉치여야 함
// ==========================================================

static_assert(sizeof(JsonReader) == sizeof(void*), "JsonReader must be a single pointer");
static_assert(std::is_trivially_copyable_v<JsonReader>, "JsonReader must be trivially copyable");
static_assert(std::is_trivially_destructible_v<JsonReader>, "JsonReader must own nothing");

static_assert(sizeof(JsonWriter) == 2 * sizeof(void*), "JsonWriter must be exactly (doc, value)");
static_assert(std::is_trivially_copyable_v<JsonWriter>, "JsonWriter must be trivially copyable");
static_assert(std::is_trivially_destructible_v<JsonWriter>, "JsonWriter must own nothing");

static_assert(!std::is_copy_constructible_v<JsonDeserializer>, "JsonDeserializer owns the document");
static_assert(std::is_move_constructible_v<JsonDeserializer>, "JsonDeserializer must be movable");
static_assert(!std::is_copy_constructible_v<JsonSerializer>, "JsonSerializer owns the document");
static_assert(std::is_move_constructible_v<JsonSerializer>, "JsonSerializer must be movable");

// ==========================================================
//  Test Types
// ==========================================================

namespace test
{

struct Vector3
{
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;

    void Serialize(
        JsonWriter& _writer) const
    {
        _writer.SetObject();
        _writer.WriteField("x", x);
        _writer.WriteField("y", y);
        _writer.WriteField("z", z);
    }

    [[nodiscard]] static SerializeResult<Vector3> Deserialize(
        const JsonReader& _reader)
    {
        SerializeResult<float> x = _reader.ReadField<float>("x");
        JUG_DISPATCH_FAILED(x);
        SerializeResult<float> y = _reader.ReadField<float>("y");
        JUG_DISPATCH_FAILED(y);
        SerializeResult<float> z = _reader.ReadField<float>("z");
        JUG_DISPATCH_FAILED(z);
        return Vector3 { x.GetValue(), y.GetValue(), z.GetValue() };
    }

    [[nodiscard]] bool operator==(const Vector3&) const = default;
};

// 내가 소유하지 않은 타입 취급 - JsonAdlSerializer 특수화로만 훅인함.
struct Rgba
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;

    [[nodiscard]] bool operator==(const Rgba&) const = default;
};

class Handle
{
public:
    Handle() = delete;

    explicit Handle(
        const uint32_t _id)
        : m_id(_id)
    {
    }

    [[nodiscard]] uint32_t GetID() const
    {
        return m_id;
    }

    void Serialize(
        JsonWriter& _writer) const
    {
        _writer.Write(m_id);
    }

    [[nodiscard]] static SerializeResult<Handle> Deserialize(
        const JsonReader& _reader)
    {
        SerializeResult<uint32_t> id = _reader.Read<uint32_t>();
        JUG_DISPATCH_FAILED(id);
        return Handle { id.GetValue() };
    }

private:
    uint32_t m_id;
};

class Version
{
public:
    Version() = delete;

    Version(
        const int _major,
        const int _minor)
        : m_major(_major)
        , m_minor(_minor)
    {
    }

    [[nodiscard]] int GetMajor() const
    {
        return m_major;
    }

    [[nodiscard]] int GetMinor() const
    {
        return m_minor;
    }

    [[nodiscard]] bool operator==(const Version&) const = default;

private:
    int m_major;
    int m_minor;
};

struct Person
{
    std::string              name;
    int                      age        = 0;
    bool                     isEmployed = false;
    std::vector<std::string> skills;
    Vector3                  position;

    void Serialize(
        JsonWriter& _writer) const
    {
        _writer.SetObject();
        _writer.WriteField("name", name);
        _writer.WriteField("age", age);
        _writer.WriteField("isEmployed", isEmployed);

        JsonWriter skillArray = _writer.BeginArrayField("skills");
        for (const std::string& skill: skills)
        {
            skillArray.PushBack(skill);
        }

        _writer.WriteField("position", position);
    }

    [[nodiscard]] static SerializeResult<Person> Deserialize(
        const JsonReader& _reader)
    {
        SerializeResult<std::string> name = _reader.ReadField<std::string>("name");
        JUG_DISPATCH_FAILED(name);
        SerializeResult<int> age = _reader.ReadField<int>("age");
        JUG_DISPATCH_FAILED(age);
        SerializeResult<bool> isEmployed = _reader.ReadField<bool>("isEmployed");
        JUG_DISPATCH_FAILED(isEmployed);

        SerializeResult<JsonReader> skillArray = _reader.FindField("skills");
        JUG_DISPATCH_FAILED(skillArray);

        std::vector<std::string> skills;
        skills.reserve(skillArray.GetValue().GetSize());
        for (const JsonReader& element: skillArray.GetValue().GetArray())
        {
            SerializeResult<std::string> skill = element.Read<std::string>();
            JUG_DISPATCH_FAILED(skill);
            skills.push_back(skill.Take());
        }

        SerializeResult<Vector3> position = _reader.ReadField<Vector3>("position");
        JUG_DISPATCH_FAILED(position);

        return Person { name.Take(), age.GetValue(), isEmployed.GetValue(), std::move(skills), position.GetValue() };
    }

    [[nodiscard]] bool operator==(const Person&) const = default;
};

enum class eColor : uint8_t
{
    Red   = 0,
    Green = 1,
    Blue  = 2,
};

enum class Suit : uint8_t
{
    Clubs,
    Diamonds,
    Hearts,
    Spades,
};

[[nodiscard]] JsonDeserializer MakeDeserializer(
    const std::string_view _json)
{
    SerializeResult<JsonDeserializer> result = JsonDeserializer::LoadFromString(_json);
    EXPECT_TRUE(result.HasValue());
    return result.Take();
}

}   // namespace test

// JsonAdlSerializer<T> 특수화는 원래 템플릿을 감싸는 네임스페이스 안에서만 선언 가능함 -
// namespace test 안에서 ::jug::JsonAdlSerializer<...> 로 쓰는 건 MSVC 확장이라 clang에서 막힘.
namespace jug
{

template<>
struct JsonAdlSerializer<test::Rgba>
{
    static void Serialize(
        JsonWriter&       _writer,
        const test::Rgba& _value)
    {
        _writer.SetArray();
        _writer.PushBack(_value.r);
        _writer.PushBack(_value.g);
        _writer.PushBack(_value.b);
        _writer.PushBack(_value.a);
    }

    [[nodiscard]] static SerializeResult<test::Rgba> Deserialize(
        const JsonReader& _reader)
    {
        uint8_t* pChannels[] = { nullptr, nullptr, nullptr, nullptr };

        test::Rgba color;
        pChannels[0] = &color.r;
        pChannels[1] = &color.g;
        pChannels[2] = &color.b;
        pChannels[3] = &color.a;

        size_t index = 0;
        for (const JsonReader& element: _reader.GetArray())
        {
            if (index >= 4)
            {
                return Failed { eSerializerError::TypeMismatch };
            }

            SerializeResult<uint8_t> channel = element.Read<uint8_t>();
            JUG_DISPATCH_FAILED(channel);
            *pChannels[index] = channel.GetValue();
            ++index;
        }

        if (index != 4)
        {
            return Failed { eSerializerError::TypeMismatch };
        }
        return color;
    }
};

template<>
struct JsonAdlSerializer<test::Version>
{
    static void Serialize(
        JsonWriter&          _writer,
        const test::Version& _value)
    {
        _writer.SetArray();
        _writer.PushBack(_value.GetMajor());
        _writer.PushBack(_value.GetMinor());
    }

    [[nodiscard]] static SerializeResult<test::Version> Deserialize(
        const JsonReader& _reader)
    {
        JsonReader::ArrayIterator it  = _reader.BeginArray();
        JsonReader::ArrayIterator end = _reader.EndArray();

        if (it == end)
        {
            return Failed { eSerializerError::TypeMismatch };
        }
        SerializeResult<int> major = (*it).Read<int>();
        JUG_DISPATCH_FAILED(major);

        ++it;
        if (it == end)
        {
            return Failed { eSerializerError::TypeMismatch };
        }
        SerializeResult<int> minor = (*it).Read<int>();
        JUG_DISPATCH_FAILED(minor);

        ++it;
        if (it != end)
        {
            return Failed { eSerializerError::TypeMismatch };
        }
        return test::Version { major.GetValue(), minor.GetValue() };
    }
};

template<>
struct JsonAdlSerializer<test::Suit>
{
    static void Serialize(
        JsonWriter&       _writer,
        const test::Suit& _value)
    {
        switch (_value)
        {
            case test::Suit::Clubs: _writer.Write("Clubs"); return;
            case test::Suit::Diamonds: _writer.Write("Diamonds"); return;
            case test::Suit::Hearts: _writer.Write("Hearts"); return;
            case test::Suit::Spades: _writer.Write("Spades"); return;
        }
        JUG_ASSERT(false, "Unknown Suit value");
    }

    [[nodiscard]] static SerializeResult<test::Suit> Deserialize(
        const JsonReader& _reader)
    {
        SerializeResult<std::string> name = _reader.Read<std::string>();
        JUG_DISPATCH_FAILED(name);

        if (name.GetValue() == "Clubs") return test::Suit::Clubs;
        if (name.GetValue() == "Diamonds") return test::Suit::Diamonds;
        if (name.GetValue() == "Hearts") return test::Suit::Hearts;
        if (name.GetValue() == "Spades") return test::Suit::Spades;
        return Failed { eSerializerError::TypeMismatch };
    }
};

}   // namespace jug

// ==========================================================
//  JsonReader
// ==========================================================

TEST(JsonReader, PrimitiveRoundTrip)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();
    writer.WriteField("boolTrue", true);
    writer.WriteField("boolFalse", false);
    writer.WriteField("int", -42);
    writer.WriteField("intMin", std::numeric_limits<int64_t>::min());
    writer.WriteField("uintMax", std::numeric_limits<uint64_t>::max());
    writer.WriteField("double", 3.141592653589793);
    writer.WriteField("literal", "hello");
    writer.WriteField("string", std::string { "world" });
    writer.WriteField("view", std::string_view { "view" });
    writer.WriteField("empty", "");
    writer.WriteField("color", test::eColor::Blue);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());

    JsonDeserializer deserializer = test::MakeDeserializer(jsonOr.GetValue());
    JsonReader       reader       = deserializer.GetReader();
    EXPECT_TRUE(reader.IsObject());
    EXPECT_EQ(reader.GetSize(), 11u);

    EXPECT_EQ(reader.ReadField<bool>("boolTrue").GetValue(), true);
    EXPECT_EQ(reader.ReadField<bool>("boolFalse").GetValue(), false);
    EXPECT_EQ(reader.ReadField<int>("int").GetValue(), -42);
    EXPECT_EQ(reader.ReadField<int64_t>("intMin").GetValue(), std::numeric_limits<int64_t>::min());
    EXPECT_EQ(reader.ReadField<uint64_t>("uintMax").GetValue(), std::numeric_limits<uint64_t>::max());
    EXPECT_DOUBLE_EQ(reader.ReadField<double>("double").GetValue(), 3.141592653589793);
    EXPECT_EQ(reader.ReadField<std::string>("literal").GetValue(), "hello");
    EXPECT_EQ(reader.ReadField<std::string_view>("string").GetValue(), "world");
    EXPECT_EQ(std::string_view { reader.ReadField<const char*>("view").GetValue() }, "view");
    EXPECT_TRUE(reader.ReadField<std::string>("empty").GetValue().empty());
    EXPECT_EQ(reader.ReadField<test::eColor>("color").GetValue(), test::eColor::Blue);
}

TEST(JsonReader, NestedObject)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();
    {
        JsonWriter outer = writer.BeginObjectField("outer");
        JsonWriter inner = outer.BeginObjectField("inner");
        inner.WriteField("depth", 3);
    }
    writer.WriteField("sibling", "ok");

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"outer":{"inner":{"depth":3}},"sibling":"ok"})");

    JsonDeserializer deserializer = test::MakeDeserializer(jsonOr.GetValue());
    JsonReader       reader       = deserializer.GetReader();

    SerializeResult<JsonReader> outer = reader.FindField("outer");
    ASSERT_TRUE(outer.HasValue());
    SerializeResult<JsonReader> inner = outer.GetValue().FindField("inner");
    ASSERT_TRUE(inner.HasValue());
    EXPECT_EQ(inner.GetValue().ReadField<int>("depth").GetValue(), 3);
    EXPECT_EQ(reader.ReadField<std::string>("sibling").GetValue(), "ok");
}

TEST(JsonReader, ArrayRange)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetArray();
    for (int i = 0; i < 5; ++i)
    {
        writer.PushBack(i * 10);
    }
    writer.PushBackArray().PushBack("nested");
    writer.PushBackObject().WriteField("k", 1);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"([0,10,20,30,40,["nested"],{"k":1}])");

    JsonDeserializer deserializer = test::MakeDeserializer(jsonOr.GetValue());
    JsonReader       reader       = deserializer.GetReader();
    EXPECT_TRUE(reader.IsArray());
    EXPECT_EQ(reader.GetSize(), 7u);

    int index = 0;
    for (const JsonReader& element: reader.GetArray())
    {
        if (index < 5)
        {
            EXPECT_EQ(element.Read<int>().GetValue(), index * 10);
        }
        else if (index == 5)
        {
            EXPECT_TRUE(element.IsArray());
            EXPECT_EQ((*element.BeginArray()).Read<std::string>().GetValue(), "nested");
        }
        else
        {
            EXPECT_TRUE(element.IsObject());
            EXPECT_EQ(element.ReadField<int>("k").GetValue(), 1);
        }
        ++index;
    }
    EXPECT_EQ(index, 7);
}

TEST(JsonReader, ArrayBeginEnd)
{
    JsonDeserializer deserializer = test::MakeDeserializer("[1,2,3]");
    JsonReader       reader       = deserializer.GetReader();

    int sum   = 0;
    int count = 0;
    for (JsonReader::ArrayIterator it = reader.BeginArray(); it != reader.EndArray(); ++it)
    {
        sum += (*it).Read<int>().GetValue();
        ++count;
    }

    EXPECT_EQ(count, 3);
    EXPECT_EQ(sum, 6);
}

TEST(JsonReader, ObjectRange)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"a":1,"b":2,"c":3})");
    JsonReader       reader       = deserializer.GetReader();

    std::vector<std::string> keys;
    int                      sum = 0;
    for (const auto& [key, value]: reader.GetObject())
    {
        keys.emplace_back(key);
        sum += value.Read<int>().GetValue();
    }

    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], "a");
    EXPECT_EQ(keys[1], "b");
    EXPECT_EQ(keys[2], "c");
    EXPECT_EQ(sum, 6);
}

TEST(JsonReader, ObjectRangeStructuredBinding)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"a":1,"b":2})");
    JsonReader       reader       = deserializer.GetReader();

    int sum = 0;
    for (auto [key, value]: reader.GetObject())
    {
        EXPECT_FALSE(key.empty());
        sum += value.Read<int>().GetValue();
    }
    EXPECT_EQ(sum, 3);
}

TEST(JsonReader, ObjectBeginEnd)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"a":1,"b":2,"c":3})");
    JsonReader       reader       = deserializer.GetReader();

    int count = 0;
    for (JsonReader::ObjectIterator it = reader.BeginObject(); it != reader.EndObject(); ++it)
    {
        ++count;
    }
    EXPECT_EQ(count, 3);
}

TEST(JsonReader, RangeOnWrongTypeIsEmpty)
{
    JsonDeserializer objectDoc = test::MakeDeserializer(R"({"a":1})");
    EXPECT_TRUE(objectDoc.GetReader().BeginArray() == objectDoc.GetReader().EndArray());

    JsonDeserializer arrayDoc = test::MakeDeserializer("[1,2,3]");
    EXPECT_TRUE(arrayDoc.GetReader().BeginObject() == arrayDoc.GetReader().EndObject());
}

TEST(JsonReader, EmptyContainerRangeIsEmpty)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"arr":[],"obj":{}})");
    JsonReader       reader       = deserializer.GetReader();

    JsonReader arr = reader.FindField("arr").GetValue();
    EXPECT_EQ(arr.GetSize(), 0u);
    EXPECT_TRUE(arr.BeginArray() == arr.EndArray());

    JsonReader obj = reader.FindField("obj").GetValue();
    EXPECT_EQ(obj.GetSize(), 0u);
    EXPECT_TRUE(obj.BeginObject() == obj.EndObject());
}

TEST(JsonReader, HasField)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"present":7})");
    JsonReader       reader       = deserializer.GetReader();

    EXPECT_TRUE(reader.HasField("present"));
    EXPECT_FALSE(reader.HasField("absent"));
}

TEST(JsonReader, TypeQueries)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"null":null,"bool":true,"num":1.5,"str":"s","arr":[],"obj":{}})");
    JsonReader       reader       = deserializer.GetReader();

    EXPECT_TRUE(reader.FindField("null").GetValue().IsNull());
    EXPECT_TRUE(reader.FindField("bool").GetValue().IsBool());
    EXPECT_TRUE(reader.FindField("num").GetValue().IsNumber());
    EXPECT_TRUE(reader.FindField("str").GetValue().IsString());
    EXPECT_TRUE(reader.FindField("arr").GetValue().IsArray());
    EXPECT_TRUE(reader.FindField("obj").GetValue().IsObject());

    EXPECT_TRUE(reader.FindField("arr").GetValue().IsContainer());
    EXPECT_FALSE(reader.FindField("num").GetValue().IsContainer());
    EXPECT_FALSE(reader.FindField("num").GetValue().IsString());
}

TEST(JsonReader, ParseErrors)
{
    EXPECT_TRUE(JsonDeserializer::LoadFromString("{ broken").HasError());
    EXPECT_TRUE(JsonDeserializer::LoadFromString("").HasError());
    EXPECT_TRUE(JsonDeserializer::LoadFromString(R"({"a":1,})").HasError());
}

TEST(JsonReader, ParseOptionAllowsTrailingComma)
{
    SerializeResult<JsonDeserializer> result = JsonDeserializer::LoadFromString(R"({"a":1,})", eJsonLoadOption::AllowTrailingCommas);
    ASSERT_TRUE(result.HasValue());
    EXPECT_EQ(result.GetValue().GetReader().ReadField<int>("a").GetValue(), 1);
}

TEST(JsonReader, TypeMismatchStringAsInt)
{
    JsonDeserializer     deserializer = test::MakeDeserializer(R"({"str":"x"})");
    SerializeResult<int> value        = deserializer.GetReader().ReadField<int>("str");
    ASSERT_TRUE(value.HasError());
    EXPECT_EQ(value.GetError(), eSerializerError::TypeMismatch);
}

TEST(JsonReader, TypeMismatchNumberAsBool)
{
    JsonDeserializer      deserializer = test::MakeDeserializer(R"({"big":300})");
    SerializeResult<bool> value        = deserializer.GetReader().ReadField<bool>("big");
    ASSERT_TRUE(value.HasError());
    EXPECT_EQ(value.GetError(), eSerializerError::TypeMismatch);
}

TEST(JsonReader, TypeMismatchNumberAsString)
{
    JsonDeserializer             deserializer = test::MakeDeserializer(R"({"big":300})");
    SerializeResult<std::string> value        = deserializer.GetReader().ReadField<std::string>("big");
    ASSERT_TRUE(value.HasError());
    EXPECT_EQ(value.GetError(), eSerializerError::TypeMismatch);
}

TEST(JsonReader, TypeMismatchRealAsInt)
{
    JsonDeserializer     deserializer = test::MakeDeserializer(R"({"real":1.5})");
    SerializeResult<int> value        = deserializer.GetReader().ReadField<int>("real");
    ASSERT_TRUE(value.HasError());
    EXPECT_EQ(value.GetError(), eSerializerError::TypeMismatch);
}

TEST(JsonReader, IntAsRealAccepted)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"big":300})");
    EXPECT_DOUBLE_EQ(deserializer.GetReader().ReadField<double>("big").GetValue(), 300.0);
}

TEST(JsonReader, MissingField)
{
    JsonDeserializer     deserializer = test::MakeDeserializer(R"({"a":1})");
    SerializeResult<int> value        = deserializer.GetReader().ReadField<int>("missing");
    ASSERT_TRUE(value.HasError());
    EXPECT_EQ(value.GetError(), eSerializerError::MissingField);
}

TEST(JsonReader, NarrowingRead)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"big":300})");
    EXPECT_EQ(deserializer.GetReader().ReadField<int16_t>("big").GetValue(), 300);
}

TEST(JsonReader, NarrowIntegerAndFloatTypes)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"i8":-12,"u8":200,"i16":-1234,"u16":60000,"i32":-123456,"u32":4000000000,"f":3.5})");
    JsonReader       reader       = deserializer.GetReader();

    EXPECT_EQ(reader.ReadField<int8_t>("i8").GetValue(), -12);
    EXPECT_EQ(reader.ReadField<uint8_t>("u8").GetValue(), 200);
    EXPECT_EQ(reader.ReadField<int16_t>("i16").GetValue(), -1234);
    EXPECT_EQ(reader.ReadField<uint16_t>("u16").GetValue(), 60000);
    EXPECT_EQ(reader.ReadField<int32_t>("i32").GetValue(), -123456);
    EXPECT_EQ(reader.ReadField<uint32_t>("u32").GetValue(), 4000000000u);
    EXPECT_FLOAT_EQ(reader.ReadField<float>("f").GetValue(), 3.5f);
}

TEST(JsonReader, CustomAdlTakesPriorityOverPrimitiveForEnum)
{
    JsonDeserializer            deserializer = test::MakeDeserializer(R"("Spades")");
    SerializeResult<test::Suit> value        = deserializer.GetReader().Read<test::Suit>();
    ASSERT_TRUE(value.HasValue());
    EXPECT_EQ(value.GetValue(), test::Suit::Spades);
}

TEST(JsonReader, CustomAdlEnumRejectsUnknownString)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"("Joker")");
    EXPECT_TRUE(deserializer.GetReader().Read<test::Suit>().HasError());
}

TEST(JsonReader, PlainEnumWithoutAdlSpecializationStaysRawInteger)
{
    JsonDeserializer              deserializer = test::MakeDeserializer("2");
    SerializeResult<test::eColor> value        = deserializer.GetReader().Read<test::eColor>();
    ASSERT_TRUE(value.HasValue());
    EXPECT_EQ(value.GetValue(), test::eColor::Blue);
}

TEST(JsonReader, ReadFailureDoesNotBlockOtherReads)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"position":{"x":1,"y":2,"z":"oops"},"sibling":"ok"})");
    JsonReader       reader       = deserializer.GetReader();

    SerializeResult<test::Vector3> posResult = reader.ReadField<test::Vector3>("position");
    ASSERT_TRUE(posResult.HasError());
    EXPECT_EQ(posResult.GetError(), eSerializerError::TypeMismatch);

    EXPECT_EQ(reader.ReadField<std::string>("sibling").GetValue(), "ok");
}

TEST(JsonReader, CustomTypes)
{
    const test::Vector3 position = { 1.0f, -2.5f, 0.125f };
    const test::Rgba    color    = { 255, 128, 0, 255 };
    const test::Handle  handle { 4242 };
    const test::Version version { 3, 7 };

    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();
    writer.WriteField("position", position);
    writer.WriteField("color", color);
    writer.WriteField("handle", handle);
    writer.WriteField("version", version);
    {
        JsonWriter positions = writer.BeginArrayField("positions");
        positions.PushBack(position);
        positions.PushBack(test::Vector3 { 9.0f, 9.0f, 9.0f });
    }

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_NE(jsonOr.GetValue().find(R"("color":[255,128,0,255])"), std::string::npos);
    EXPECT_NE(jsonOr.GetValue().find(R"("version":[3,7])"), std::string::npos);

    JsonDeserializer deserializer = test::MakeDeserializer(jsonOr.GetValue());
    JsonReader       reader       = deserializer.GetReader();

    EXPECT_EQ(reader.ReadField<test::Vector3>("position").GetValue(), position);
    EXPECT_EQ(reader.ReadField<test::Rgba>("color").GetValue(), color);
    EXPECT_EQ(reader.ReadField<test::Handle>("handle").GetValue().GetID(), handle.GetID());
    EXPECT_EQ(reader.ReadField<test::Version>("version").GetValue(), version);

    JsonReader positions = reader.FindField("positions").GetValue();
    EXPECT_EQ(positions.GetSize(), 2u);

    int index = 0;
    for (const JsonReader& element: positions.GetArray())
    {
        const test::Vector3 expected = (index == 0) ? position : test::Vector3 { 9.0f, 9.0f, 9.0f };
        EXPECT_EQ(element.Read<test::Vector3>().GetValue(), expected);
        ++index;
    }
    EXPECT_EQ(index, 2);
}

TEST(JsonReader, PersonEndToEnd)
{
    test::Person person;
    person.name       = "John Doe";
    person.age        = 30;
    person.isEmployed = true;
    person.skills     = { "JavaScript", "Python" };
    person.position   = { 1.0f, 2.0f, 3.0f };

    JsonSerializer serializer;
    serializer.GetWriter().Write(person);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());

    JsonDeserializer              deserializer = test::MakeDeserializer(jsonOr.GetValue());
    SerializeResult<test::Person> personResult = deserializer.GetReader().Read<test::Person>();
    ASSERT_TRUE(personResult.HasValue());
    EXPECT_EQ(personResult.GetValue(), person);
}

TEST(JsonReader, UnicodeAndEscapes)
{
    constexpr std::string_view text = "한글 \"인용\" \역슬래시\ \n줄바꿈\t탭";

    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();
    writer.WriteField("text", text);
    writer.WriteField("한글키", "값");

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());

    JsonDeserializer deserializer = test::MakeDeserializer(jsonOr.GetValue());
    JsonReader       reader       = deserializer.GetReader();
    EXPECT_EQ(reader.ReadField<std::string>("text").GetValue(), text);
    EXPECT_EQ(reader.ReadField<std::string>("한글키").GetValue(), "값");
}

TEST(JsonReader, DeepNestingViaHandleReassignment)
{
    constexpr int kDepth = 64;

    std::string json;
    for (int i = 0; i < kDepth; ++i)
        json += "[";
    json += "42";
    for (int i = 0; i < kDepth; ++i)
        json += "]";

    JsonDeserializer deserializer = test::MakeDeserializer(json);
    JsonReader       cursor       = deserializer.GetReader();
    for (int i = 0; i < kDepth; ++i)
    {
        ASSERT_TRUE(cursor.IsArray());
        cursor = *cursor.BeginArray();
    }
    EXPECT_EQ(cursor.Read<int>().GetValue(), 42);
}

TEST(JsonReader, ScalarRoot)
{
    JsonSerializer serializer;
    serializer.GetWriter().Write("bare");

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"("bare")");

    JsonDeserializer deserializer = test::MakeDeserializer(jsonOr.GetValue());
    JsonReader       reader       = deserializer.GetReader();
    EXPECT_TRUE(reader.IsString());
    EXPECT_FALSE(reader.IsContainer());
    EXPECT_EQ(reader.Read<std::string>().GetValue(), "bare");

    JsonDeserializer nullDoc = test::MakeDeserializer("null");
    EXPECT_TRUE(nullDoc.GetReader().IsNull());
}

TEST(JsonReader, MoveSemantics)
{
    JsonDeserializer first  = test::MakeDeserializer(R"({"v":5})");
    JsonDeserializer second = std::move(first);

    EXPECT_EQ(second.GetReader().ReadField<int>("v").GetValue(), 5);
}

TEST(JsonReader, HandleIsFreelyCopyable)
{
    JsonDeserializer deserializer = test::MakeDeserializer(R"({"a":{"b":{"c":7}}})");

    JsonReader a       = deserializer.GetReader().FindField("a").GetValue();
    JsonReader b       = a.FindField("b").GetValue();
    JsonReader copyOfB = b;

    EXPECT_EQ(b.ReadField<int>("c").GetValue(), 7);
    EXPECT_EQ(copyOfB.ReadField<int>("c").GetValue(), 7);
}

// ==========================================================
//  JsonWriter
// ==========================================================

TEST(JsonWriter, NarrowIntegerAndFloatTypes)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();
    writer.WriteField("i8", static_cast<int8_t>(-12));
    writer.WriteField("u8", static_cast<uint8_t>(200));
    writer.WriteField("i16", static_cast<int16_t>(-1234));
    writer.WriteField("u16", static_cast<uint16_t>(60000));
    writer.WriteField("i32", static_cast<int32_t>(-123456));
    writer.WriteField("u32", static_cast<uint32_t>(4000000000u));
    writer.WriteField("f", 3.5f);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"i8":-12,"u8":200,"i16":-1234,"u16":60000,"i32":-123456,"u32":4000000000,"f":3.5})");
}

TEST(JsonWriter, CustomAdlTakesPriorityOverPrimitiveForEnum)
{
    JsonSerializer serializer;
    serializer.GetWriter().Write(test::Suit::Hearts);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"("Hearts")");
}

TEST(JsonWriter, PlainEnumWithoutAdlSpecializationStaysRawInteger)
{
    JsonSerializer serializer;
    serializer.GetWriter().Write(test::eColor::Blue);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), "2");
}

TEST(JsonWriter, Pretty)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();
    writer.WriteField("a", 1);

    SerializeResult<std::string> compactOr = serializer.SaveToString();
    SerializeResult<std::string> prettyOr  = serializer.SaveToString(eJsonSaveOption::Pretty);
    ASSERT_TRUE(compactOr.HasValue());
    ASSERT_TRUE(prettyOr.HasValue());
    EXPECT_EQ(compactOr.GetValue(), R"({"a":1})");
    EXPECT_NE(prettyOr.GetValue().find('\n'), std::string::npos);
    EXPECT_GT(prettyOr.GetValue().length(), compactOr.GetValue().length());
}

TEST(JsonWriter, WriteOptions)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();
    writer.WriteField("k", "한/글");

    EXPECT_EQ(serializer.SaveToString().GetValue(), R"({"k":"한/글"})");
    EXPECT_EQ(serializer.SaveToString(eJsonSaveOption::EscapeSlashes).GetValue(), R"({"k":"한\/글"})");
    EXPECT_EQ(serializer.SaveToString(eJsonSaveOption::NewlineAtEnd).GetValue(), "{\"k\":\"한/글\"}\n");

    const std::string escaped = serializer.SaveToString(eJsonSaveOption::EscapeUnicode).GetValue();
    EXPECT_TRUE(std::ranges::all_of(escaped, [](const char _c) { return static_cast<unsigned char>(_c) < 0x80; }));
    EXPECT_NE(escaped.find("\u"), std::string::npos);

    EXPECT_NE(serializer.SaveToString(eJsonSaveOption::Pretty).GetValue().find("    \"k\""), std::string::npos);
    EXPECT_NE(serializer.SaveToString(eJsonSaveOption::PrettyTwoSpaces).GetValue().find("  \"k\""), std::string::npos);
}

TEST(JsonWriter, ManualObjectField)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();

    JsonWriter nested = writer.BeginObjectField("nested");
    nested.WriteField("a", 1);
    nested.WriteField("b", 2);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"nested":{"a":1,"b":2}})");
}

TEST(JsonWriter, ManualArrayField)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();

    JsonWriter nested = writer.BeginArrayField("nested");
    nested.PushBack("item");
    nested.PushBack(789);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"nested":["item",789]})");
}

TEST(JsonWriter, NullAndNonFiniteReal)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();
    writer.WriteField("null", nullptr);
    writer.WriteField("inf", std::numeric_limits<double>::infinity());
    writer.WriteField("nan", std::numeric_limits<double>::quiet_NaN());

    EXPECT_TRUE(serializer.SaveToString().HasError());

    SerializeResult<std::string> asNullOr = serializer.SaveToString(eJsonSaveOption::InfAndNanAsNull);
    ASSERT_TRUE(asNullOr.HasValue());
    EXPECT_EQ(asNullOr.GetValue(), R"({"null":null,"inf":null,"nan":null})");

    SerializeResult<std::string> allowOr = serializer.SaveToString(eJsonSaveOption::AllowInfAndNan);
    ASSERT_TRUE(allowOr.HasValue());
    EXPECT_EQ(allowOr.GetValue(), R"({"null":null,"inf":Infinity,"nan":NaN})");
}

TEST(JsonWriter, MoveSemantics)
{
    JsonSerializer first;
    {
        JsonWriter writer = first.GetWriter();
        writer.SetObject();
        writer.WriteField("moved", 1);
    }

    JsonSerializer second = std::move(first);

    SerializeResult<std::string> jsonOr = second.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"moved":1})");
}

TEST(JsonWriter, DeepNestingViaHandleReassignment)
{
    constexpr int kDepth = 64;

    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetArray();

    JsonWriter cursor = writer.PushBackArray();
    for (int i = 1; i < kDepth; ++i)
    {
        cursor = cursor.PushBackArray();
    }
    cursor.PushBack(42);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());

    std::string expected;
    for (int i = 0; i < kDepth + 1; ++i)
        expected += "[";
    expected += "42";
    for (int i = 0; i < kDepth + 1; ++i)
        expected += "]";
    EXPECT_EQ(jsonOr.GetValue(), expected);
}

TEST(JsonWriter, HandleIsFreelyCopyable)
{
    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();

    JsonWriter arr  = writer.BeginArrayField("a");
    JsonWriter copy = arr;

    arr.PushBack(1);
    copy.PushBack(2);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"a":[1,2]})");
}

TEST(JsonWriter, GetWriterReturnsExistingRoot)
{
    JsonSerializer serializer;
    serializer.GetWriter().SetObject();

    serializer.GetWriter().WriteField("a", 1);
    serializer.GetWriter().WriteField("b", 2);

    SerializeResult<std::string> jsonOr = serializer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"a":1,"b":2})");
}

// ==========================================================
//  File
// ==========================================================

TEST(JsonSerializer, FileRoundTrip)
{
    const std::filesystem::path path = std::filesystem::temp_directory_path() / std::filesystem::path { u8"jugx_한글_test.json" };

    JsonSerializer serializer;
    JsonWriter     writer = serializer.GetWriter();
    writer.SetObject();
    writer.WriteField("saved", true);
    writer.WriteField("count", 12345);
    ASSERT_EQ(serializer.SaveToFile(path, eJsonSaveOption::Pretty), eSerializerError::None);
    ASSERT_TRUE(std::filesystem::exists(path));

    SerializeResult<JsonDeserializer> result = JsonDeserializer::LoadFromFile(path);
    ASSERT_TRUE(result.HasValue());
    {
        JsonReader reader = result.GetValue().GetReader();
        EXPECT_EQ(reader.ReadField<bool>("saved").GetValue(), true);
        EXPECT_EQ(reader.ReadField<int>("count").GetValue(), 12345);
    }

    std::filesystem::remove(path);
    EXPECT_TRUE(JsonDeserializer::LoadFromFile(path).HasError());
}
