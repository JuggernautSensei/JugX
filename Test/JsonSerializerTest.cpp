// JsonSerializer 동작 검증용 테스트 (Google Test)
//
// 에러 모델: ReadTo/ReadFieldTo/Read/ReadField 등은 실패해도 예외를 던지지 않고,
// 각 호출이 독립적으로 성공/실패를 자기 리턴값(bool)으로 알려준다.
// 한 호출이 실패해도 이후의 다른 호출들은 전혀 영향받지 않고 정상적으로 계속 읽힌다.
// HasError()/GetLastError()/GetLastErrorMsg()는 "지금 상태가 깨끗한가"가 아니라 "가장 최근 실패가
// 뭐였나"를 보고하는 전역 진단값이다 - 실패할 때마다 덮어써지고(성공은 건드리지 않음), 특정 호출
// 하나의 성공/실패를 확인하려면 그 호출의 리턴값을 써야 한다.
//
// 배열/오브젝트 순회: Read/ReadTo는 현재 값을 소비하면서 커서를 자동으로 다음으로 전진시킨다.
// 그래서 순회는 그냥 `while (reader.HasNext()) { reader.Read<T>(); }` 로 충분하다.
// HasNext()/GetKey()는 배열과 오브젝트 양쪽에서 다 쓸 수 있다(오브젝트는 값을 키 순서대로 순회).
// 값을 읽지 않고 건너뛰고 싶을 때만 Next()를 명시적으로 쓴다 - Read와 같은 스텝에서 같이 쓰면 안 됨
// (Read 자체가 이미 전진시키므로 이중 전진이 됨).
#include <cstdint>
#include <filesystem>
#include <limits>
#include <string>
#include <string_view>
#include <vector>

#include <gtest/gtest.h>

#include "../Source/JsonReader.h"
#include "../Source/JsonWriter.h"

using namespace jug;

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
        _writer.BeginObject();
        _writer.WriteField("x", x);
        _writer.WriteField("y", y);
        _writer.WriteField("z", z);
        _writer.EndObject();
    }

    void Deserialize(
        JsonReader& _reader)
    {
        _reader.BeginObject();
        _reader.ReadFieldTo("x", x);
        _reader.ReadFieldTo("y", y);
        _reader.ReadFieldTo("z", z);
        _reader.EndObject();
    }

    [[nodiscard]] bool operator==(const Vector3&) const = default;
};

struct Rgba
{
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 0;

    [[nodiscard]] bool operator==(const Rgba&) const = default;
};

void Serialize(
    JsonWriter& _writer,
    const Rgba& _value)
{
    _writer.BeginArray();
    _writer.Write(_value.r);
    _writer.Write(_value.g);
    _writer.Write(_value.b);
    _writer.Write(_value.a);
    _writer.EndArray();
}

void Deserialize(
    JsonReader& _reader,
    Rgba&       _value)
{
    _reader.BeginArray();
    _reader.ReadTo(_value.r);
    _reader.ReadTo(_value.g);
    _reader.ReadTo(_value.b);
    _reader.ReadTo(_value.a);
    _reader.EndArray();
}

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

    [[nodiscard]] static Handle Deserialize(
        JsonReader& _reader)
    {
        return Handle { _reader.Read<uint32_t>() };
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

void Serialize(
    JsonWriter&    _writer,
    const Version& _value)
{
    _writer.BeginArray();
    _writer.Write(_value.GetMajor());
    _writer.Write(_value.GetMinor());
    _writer.EndArray();
}

[[nodiscard]] Version Deserialize(
    JsonReader& _reader,
    JsonTypeTag<Version>)
{
    _reader.BeginArray();
    const int major = _reader.Read<int>();
    const int minor = _reader.Read<int>();
    _reader.EndArray();
    return Version { major, minor };
}

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
        _writer.BeginObject();
        _writer.WriteField("name", name);
        _writer.WriteField("age", age);
        _writer.WriteField("isEmployed", isEmployed);
        _writer.BeginArray("skills");
        for (const std::string& skill: skills)
        {
            _writer.Write(skill);
        }
        _writer.EndArray();
        _writer.WriteField("position", position);
        _writer.EndObject();
    }

    void Deserialize(
        JsonReader& _reader)
    {
        _reader.BeginObject();
        _reader.ReadFieldTo("name", name);
        _reader.ReadFieldTo("age", age);
        _reader.ReadFieldTo("isEmployed", isEmployed);

        _reader.BeginArray("skills");
        skills.clear();
        while (_reader.HasNext())
        {
            skills.push_back(_reader.Read<std::string>());
        }
        _reader.EndArray();

        _reader.ReadFieldTo("position", position);
        _reader.EndObject();
    }

    [[nodiscard]] bool operator==(const Person&) const = default;
};

enum class eColor : uint8_t
{
    Red   = 0,
    Green = 1,
    Blue  = 2,
};

[[nodiscard]] JsonReader MakeReader(
    const std::string_view _json)
{
    SerializerResult<JsonReader> readerOr = JsonReader::LoadFromString(_json);
    EXPECT_TRUE(readerOr.HasValue());
    return readerOr.Take();
}
}   // namespace test

// ==========================================================
//  JsonReader
// ==========================================================

TEST(JsonReader, PrimitiveRoundTrip)
{
    JsonWriter writer;
    writer.BeginObject();
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
    writer.EndObject();

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());

    JsonReader reader = test::MakeReader(jsonOr.GetValue());
    EXPECT_TRUE(reader.IsObject());

    reader.BeginObject();
    EXPECT_EQ(reader.GetSize(), 11u);

    EXPECT_EQ(reader.ReadField<bool>("boolTrue"), true);
    EXPECT_EQ(reader.ReadField<bool>("boolFalse"), false);
    EXPECT_EQ(reader.ReadField<int>("int"), -42);
    EXPECT_EQ(reader.ReadField<int64_t>("intMin"), std::numeric_limits<int64_t>::min());
    EXPECT_EQ(reader.ReadField<uint64_t>("uintMax"), std::numeric_limits<uint64_t>::max());
    EXPECT_DOUBLE_EQ(reader.ReadField<double>("double"), 3.141592653589793);
    EXPECT_EQ(reader.ReadField<std::string>("literal"), "hello");
    EXPECT_EQ(reader.ReadField<std::string_view>("string"), "world");
    EXPECT_EQ(std::string_view { reader.ReadField<const char*>("view") }, "view");
    EXPECT_TRUE(reader.ReadField<std::string>("empty").empty());
    EXPECT_EQ(reader.ReadField<test::eColor>("color"), test::eColor::Blue);
    reader.EndObject();

    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, NestedObject)
{
    JsonWriter writer;
    writer.BeginObject();
    writer.BeginObject("outer");
    writer.BeginObject("inner");
    writer.WriteField("depth", 3);
    writer.EndObject();
    writer.EndObject();
    writer.WriteField("sibling", "ok");
    writer.EndObject();

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"outer":{"inner":{"depth":3}},"sibling":"ok"})");

    JsonReader reader = test::MakeReader(jsonOr.GetValue());
    reader.BeginObject();
    reader.BeginObject("outer");
    reader.BeginObject("inner");
    EXPECT_EQ(reader.ReadField<int>("depth"), 3);
    reader.EndObject();
    reader.EndObject();

    // 스코프 복귀 후 형제 필드를 읽을 수 있어야 함
    EXPECT_EQ(reader.ReadField<std::string>("sibling"), "ok");
    reader.EndObject();

    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, Array)
{
    JsonWriter writer;
    writer.BeginArray();
    for (int i = 0; i < 5; ++i)
    {
        writer.Write(i * 10);
    }
    writer.BeginArray();   // 중첩 배열
    writer.Write("nested");
    writer.EndArray();
    writer.BeginObject();   // 배열 안의 객체
    writer.WriteField("k", 1);
    writer.EndObject();
    writer.EndArray();

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"([0,10,20,30,40,["nested"],{"k":1}])");

    JsonReader reader = test::MakeReader(jsonOr.GetValue());
    EXPECT_TRUE(reader.IsArray());

    reader.BeginArray();
    EXPECT_EQ(reader.GetSize(), 7u);

    for (int i = 0; i < 5; ++i)
    {
        EXPECT_TRUE(reader.HasNext());
        EXPECT_EQ(reader.Read<int>(), i * 10);   // Read가 소비와 동시에 커서 전진
    }

    EXPECT_TRUE(reader.HasNext());
    reader.BeginArray();   // 부모 배열의 현재 슬롯을 소비하며 진입
    EXPECT_EQ(reader.GetSize(), 1u);
    EXPECT_EQ(reader.Read<std::string>(), "nested");
    EXPECT_FALSE(reader.HasNext());
    reader.EndArray();

    EXPECT_TRUE(reader.HasNext());
    reader.BeginObject();
    EXPECT_EQ(reader.ReadField<int>("k"), 1);
    reader.EndObject();

    EXPECT_FALSE(reader.HasNext());
    reader.EndArray();

    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, NextSkipsWithoutReading)
{
    JsonReader reader = test::MakeReader(R"([1,2,3])");

    reader.BeginArray();
    reader.Next();                      // 1 건너뜀 (안 읽음)
    EXPECT_EQ(reader.Read<int>(), 2);   // 2부터 읽힘
    reader.Next();                      // 3 건너뜀
    EXPECT_FALSE(reader.HasNext());
    reader.EndArray();

    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, ObjectIteration)
{
    JsonReader reader = test::MakeReader(R"({"a":1,"b":2,"c":3})");

    reader.BeginObject();
    EXPECT_EQ(reader.GetSize(), 3u);

    int sum = 0;
    while (reader.HasNext())
    {
        sum += reader.Read<int>();
    }
    reader.EndObject();

    EXPECT_EQ(sum, 6);
    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, ObjectKeyValueIteration)
{
    JsonReader reader = test::MakeReader(R"({"a":1,"b":2,"c":3})");

    reader.BeginObject();

    std::vector<std::string> keys;
    int                      sum = 0;
    while (reader.HasNext())
    {
        keys.emplace_back(reader.GetKey());   // Read보다 먼저 호출해야 짝이 맞는 키가 나옴
        sum += reader.Read<int>();
    }
    reader.EndObject();

    ASSERT_EQ(keys.size(), 3u);
    EXPECT_EQ(keys[0], "a");
    EXPECT_EQ(keys[1], "b");
    EXPECT_EQ(keys[2], "c");
    EXPECT_EQ(sum, 6);
    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, OptionalField)
{
    JsonReader reader = test::MakeReader(R"({"present":7})");

    reader.BeginObject();
    EXPECT_TRUE(reader.HasField("present"));
    EXPECT_FALSE(reader.HasField("absent"));
    reader.EndObject();
}

TEST(JsonReader, ParseErrors)
{
    EXPECT_TRUE(JsonReader::LoadFromString("{ broken").HasError());
    EXPECT_TRUE(JsonReader::LoadFromString("").HasError());
    EXPECT_TRUE(JsonReader::LoadFromString("{\"a\":1,}").HasError());   // 표준 JSON은 트레일링 콤마 불가
}

TEST(JsonReader, TypeMismatchStringAsInt)
{
    JsonReader reader = test::MakeReader(R"({"str":"x"})");
    reader.BeginObject();
    int value = 0;
    reader.ReadFieldTo("str", value);
    EXPECT_EQ(reader.GetLastError(), eSerializerError::TypeMismatch);
}

TEST(JsonReader, TypeMismatchNumberAsBool)
{
    JsonReader reader = test::MakeReader(R"({"big":300})");
    reader.BeginObject();
    bool value = false;
    reader.ReadFieldTo("big", value);
    EXPECT_EQ(reader.GetLastError(), eSerializerError::TypeMismatch);
}

TEST(JsonReader, TypeMismatchNumberAsString)
{
    JsonReader reader = test::MakeReader(R"({"big":300})");
    reader.BeginObject();
    std::string value;
    reader.ReadFieldTo("big", value);
    EXPECT_EQ(reader.GetLastError(), eSerializerError::TypeMismatch);
}

TEST(JsonReader, TypeMismatchRealAsInt)
{
    // 실수를 정수로 읽지 않음
    JsonReader reader = test::MakeReader(R"({"real":1.5})");
    reader.BeginObject();
    int value = 0;
    reader.ReadFieldTo("real", value);
    EXPECT_EQ(reader.GetLastError(), eSerializerError::TypeMismatch);
}

TEST(JsonReader, IntAsRealAccepted)
{
    // 실수는 정수 노드도 받아들임
    JsonReader reader = test::MakeReader(R"({"big":300})");
    reader.BeginObject();
    EXPECT_DOUBLE_EQ(reader.ReadField<double>("big"), 300.0);
    reader.EndObject();
    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, MissingField)
{
    JsonReader reader = test::MakeReader(R"({"a":1})");
    reader.BeginObject();
    int value = 0;
    reader.ReadFieldTo("missing", value);
    EXPECT_EQ(reader.GetLastError(), eSerializerError::MissingField);
}

TEST(JsonReader, NarrowingRead)
{
    JsonReader reader = test::MakeReader(R"({"big":300})");
    reader.BeginObject();
    EXPECT_EQ(reader.ReadField<int16_t>("big"), 300);
    reader.EndObject();
    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, ReadFailureDoesNotBlockOtherReads)
{
    // position.z가 실패해도 sibling은 별개 호출이라 정상적으로 읽혀야 함
    JsonReader reader = test::MakeReader(R"({"position":{"x":1,"y":2,"z":"oops"},"sibling":"ok"})");

    reader.BeginObject();

    test::Vector3 pos;
    reader.ReadFieldTo("position", pos);
    EXPECT_EQ(reader.GetLastError(), eSerializerError::TypeMismatch);
    EXPECT_FLOAT_EQ(pos.x, 1.0f);
    EXPECT_FLOAT_EQ(pos.y, 2.0f);

    std::string sibling = "untouched";
    reader.ReadFieldTo("sibling", sibling);
    EXPECT_EQ(sibling, "ok");   // 이전 실패랑 무관하게 정상적으로 읽힘

    reader.EndObject();
    EXPECT_TRUE(reader.HasError());   // GetLastError()는 마지막으로 발생한 문제(TypeMismatch)를 보고함
}

TEST(JsonReader, ErrorDoesNotBreakScopeOrIteration)
{
    JsonReader reader = test::MakeReader(R"({"bad":"x","obj":{"n":1},"arr":[1,2]})");

    reader.BeginObject();
    int value = 0;
    reader.ReadFieldTo("bad", value);
    ASSERT_TRUE(reader.HasError());

    reader.BeginObject("obj");
    int nested = 0;
    reader.ReadFieldTo("n", nested);
    reader.EndObject();
    EXPECT_EQ(nested, 1);   // 앞선 에러와 무관하게 독립적으로 성공함

    // GetValue_(true)는 읽기 성공 여부와 무관하게 항상 커서를 전진시키므로
    // 에러가 나 있어도 while(HasNext()) 순회는 정상적으로 끝까지 돈다.
    reader.BeginArray("arr");
    int sum = 0;
    while (reader.HasNext())
    {
        int item = 0;
        reader.ReadTo(item);
        sum += item;
    }
    reader.EndArray();
    EXPECT_EQ(sum, 3);
    reader.EndObject();

    EXPECT_EQ(reader.GetLastError(), eSerializerError::TypeMismatch);   // 이후 읽기가 전부 성공해서 "bad" 필드의 에러가 마지막 에러로 그대로 남아있음
}

TEST(JsonReader, CustomTypes)
{
    const test::Vector3 position = { 1.0f, -2.5f, 0.125f };
    const test::Rgba    color    = { 255, 128, 0, 255 };
    const test::Handle  handle { 4242 };
    const test::Version version { 3, 7 };

    JsonWriter writer;
    writer.BeginObject();
    writer.WriteField("position", position);
    writer.WriteField("color", color);
    writer.WriteField("handle", handle);
    writer.WriteField("version", version);
    writer.BeginArray("positions");
    writer.Write(position);
    writer.Write(test::Vector3 { 9.0f, 9.0f, 9.0f });
    writer.EndArray();
    writer.EndObject();

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_NE(jsonOr.GetValue().find(R"("color":[255,128,0,255])"), std::string::npos);
    EXPECT_NE(jsonOr.GetValue().find(R"("version":[3,7])"), std::string::npos);

    JsonReader reader = test::MakeReader(jsonOr.GetValue());
    reader.BeginObject();

    EXPECT_EQ(reader.ReadField<test::Vector3>("position"), position);
    EXPECT_EQ(reader.ReadField<test::Rgba>("color"), color);

    // non-default-constructible + factory 타입(Handle: static Deserialize, Version: 프리 함수 Deserialize)은
    // ReadField<T>(key)로 못 읽는다 - 필드 하나 없어도 T{}로 되돌릴 방법이 없어서 아예 requires로 막아둠.
    // Read<T>()로 커서 위치에서 직접 읽는 형태만 지원 (배열 원소 등, 위의 "positions" 읽기 참고).
    // 위쪽 EXPECT_NE로 handle/version이 정상 직렬화됐는지는 이미 문자열로 확인함.

    reader.BeginArray("positions");
    EXPECT_EQ(reader.GetSize(), 2u);
    EXPECT_EQ(reader.Read<test::Vector3>(), position);
    EXPECT_EQ(reader.Read<test::Vector3>(), (test::Vector3 { 9.0f, 9.0f, 9.0f }));
    reader.EndArray();
    reader.EndObject();

    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, CustomTypeInArrayAdvancesCursor)
{
    JsonWriter writer;
    writer.BeginArray();
    writer.Write(test::Vector3 { 1.0f, 1.0f, 1.0f });
    writer.Write(test::Vector3 { 2.0f, 2.0f, 2.0f });
    writer.Write(test::Vector3 { 3.0f, 3.0f, 3.0f });
    writer.EndArray();

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());

    JsonReader reader = test::MakeReader(jsonOr.GetValue());
    reader.BeginArray();
    EXPECT_FLOAT_EQ(reader.Read<test::Vector3>().x, 1.0f);
    EXPECT_FLOAT_EQ(reader.Read<test::Vector3>().x, 2.0f);
    EXPECT_FLOAT_EQ(reader.Read<test::Vector3>().x, 3.0f);
    EXPECT_FALSE(reader.HasNext());
    reader.EndArray();

    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, PersonEndToEnd)
{
    test::Person person;
    person.name       = "John Doe";
    person.age        = 30;
    person.isEmployed = true;
    person.skills     = { "JavaScript", "Python" };
    person.position   = { 1.0f, 2.0f, 3.0f };

    JsonWriter writer;
    writer.Write(person);

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());

    JsonReader   reader = test::MakeReader(jsonOr.GetValue());
    test::Person readPerson;
    reader.ReadTo(readPerson);

    EXPECT_FALSE(reader.HasError());
    EXPECT_EQ(readPerson, person);
}

TEST(JsonReader, EmptyContainers)
{
    JsonWriter writer;
    writer.BeginObject();
    writer.BeginObject("obj");
    writer.EndObject();
    writer.BeginArray("arr");
    writer.EndArray();
    writer.EndObject();

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"obj":{},"arr":[]})");

    JsonReader reader = test::MakeReader(jsonOr.GetValue());
    reader.BeginObject();
    reader.BeginObject("obj");
    EXPECT_EQ(reader.GetSize(), 0u);
    EXPECT_FALSE(reader.HasNext());
    reader.EndObject();
    reader.BeginArray("arr");
    EXPECT_EQ(reader.GetSize(), 0u);
    EXPECT_FALSE(reader.HasNext());
    reader.EndArray();
    reader.EndObject();

    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, UnicodeAndEscapes)
{
    constexpr std::string_view text = "한글 \"인용\" \\역슬래시\\ \n줄바꿈\t탭";

    JsonWriter writer;
    writer.BeginObject();
    writer.WriteField("text", text);
    writer.WriteField("한글키", "값");
    writer.EndObject();

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());

    JsonReader reader = test::MakeReader(jsonOr.GetValue());
    reader.BeginObject();
    EXPECT_EQ(reader.ReadField<std::string>("text"), text);
    EXPECT_EQ(reader.ReadField<std::string>("한글키"), "값");
    reader.EndObject();

    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, DeepNesting)
{
    constexpr int kDepth = 64;

    JsonWriter writer;
    for (int i = 0; i < kDepth; ++i)
    {
        writer.BeginArray();
    }
    writer.Write(42);
    for (int i = 0; i < kDepth; ++i)
    {
        writer.EndArray();
    }

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());

    JsonReader reader = test::MakeReader(jsonOr.GetValue());
    for (int i = 0; i < kDepth; ++i)
    {
        reader.BeginArray();
    }
    EXPECT_EQ(reader.Read<int>(), 42);
    for (int i = 0; i < kDepth; ++i)
    {
        reader.EndArray();
    }

    EXPECT_FALSE(reader.HasError());
}

TEST(JsonReader, ScalarRoot)
{
    JsonWriter writer;
    writer.Write("bare");

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"("bare")");

    JsonReader reader = test::MakeReader(jsonOr.GetValue());
    EXPECT_TRUE(reader.IsValue());
    EXPECT_EQ(reader.Read<std::string>(), "bare");
    EXPECT_FALSE(reader.HasError());

    JsonReader nullReader = test::MakeReader("null");
    EXPECT_TRUE(nullReader.IsNull());
}

TEST(JsonReader, QueriesWorkInErrorState)
{
    JsonReader reader = test::MakeReader(R"({"a":"str"})");

    reader.BeginObject();
    int value = 0;
    reader.ReadFieldTo("a", value);
    ASSERT_TRUE(reader.HasError());

    EXPECT_FALSE(reader.IsNull());
    EXPECT_FALSE(reader.IsArray());
    EXPECT_FALSE(reader.IsObject());
    EXPECT_FALSE(reader.IsContainer());
    reader.EndObject();
}

TEST(JsonReader, MoveSemantics)
{
    JsonReader first  = test::MakeReader(R"({"v":5})");
    JsonReader second = std::move(first);

    second.BeginObject();
    EXPECT_EQ(second.ReadField<int>("v"), 5);
    second.EndObject();

    EXPECT_FALSE(second.HasError());
}

TEST(JsonReader, MoveSemanticsPreservesErrorState)
{
    JsonReader first = test::MakeReader(R"({"a":"x"})");
    first.BeginObject();
    int value = 0;
    first.ReadFieldTo("a", value);
    ASSERT_TRUE(first.HasError());

    JsonReader second = std::move(first);
    EXPECT_EQ(second.GetLastError(), eSerializerError::TypeMismatch);
}

// ==========================================================
//  JsonWriter
// ==========================================================

TEST(JsonWriter, Pretty)
{
    JsonWriter writer;
    writer.BeginObject();
    writer.WriteField("a", 1);
    writer.EndObject();

    SerializerResult<std::string> compactOr = writer.SaveToString();
    SerializerResult<std::string> prettyOr  = writer.SaveToString(eJsonWriteOption::Pretty);
    ASSERT_TRUE(compactOr.HasValue());
    ASSERT_TRUE(prettyOr.HasValue());
    EXPECT_EQ(compactOr.GetValue(), R"({"a":1})");
    EXPECT_NE(prettyOr.GetValue().find('\n'), std::string::npos);
    EXPECT_GT(prettyOr.GetValue().length(), compactOr.GetValue().length());
}

TEST(JsonWriter, WriteKeyThenValue)
{
    JsonWriter writer;
    writer.BeginObject();
    writer.WriteKey("nested");
    writer.BeginArray();
    writer.Write("item");
    writer.Write(789);
    writer.EndArray();
    writer.EndObject();

    SerializerResult<std::string> jsonOr = writer.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"nested":["item",789]})");
}

TEST(JsonWriter, NullAndNonFiniteReal)
{
    JsonWriter writer;
    writer.BeginObject();
    writer.WriteField("null", nullptr);
    writer.WriteField("inf", std::numeric_limits<double>::infinity());
    writer.WriteField("nan", std::numeric_limits<double>::quiet_NaN());
    writer.EndObject();

    // 기본값은 표준 JSON을 따르므로 inf/nan이 있으면 출력 자체가 실패함
    EXPECT_TRUE(writer.SaveToString().HasError());

    // inf/nan을 어떻게 내보낼지는 플래그가 결정함
    SerializerResult<std::string> asNullOr = writer.SaveToString(eJsonWriteOption::InfAndNanAsNull);
    ASSERT_TRUE(asNullOr.HasValue());
    EXPECT_EQ(asNullOr.GetValue(), R"({"null":null,"inf":null,"nan":null})");

    SerializerResult<std::string> allowOr = writer.SaveToString(eJsonWriteOption::AllowInfAndNan);
    ASSERT_TRUE(allowOr.HasValue());
    EXPECT_EQ(allowOr.GetValue(), R"({"null":null,"inf":Infinity,"nan":NaN})");
}

TEST(JsonWriter, WriteOptions)
{
    JsonWriter writer;
    writer.BeginObject();
    writer.WriteField("k", "한/글");
    writer.EndObject();

    EXPECT_EQ(writer.SaveToString().GetValue(), R"({"k":"한/글"})");
    EXPECT_EQ(writer.SaveToString(eJsonWriteOption::EscapeSlashes).GetValue(), R"({"k":"한\/글"})");
    EXPECT_EQ(writer.SaveToString(eJsonWriteOption::NewlineAtEnd).GetValue(), "{\"k\":\"한/글\"}\n");

    // EscapeUnicode는 비ASCII를 \uXXXX로, LowercaseHex는 그 hex를 소문자로 바꿈
    EXPECT_EQ(writer.SaveToString(eJsonWriteOption::EscapeUnicode).GetValue(), R"({"k":"\uD55C/\uAE00"})");
    EXPECT_EQ(writer.SaveToString({ eJsonWriteOption::EscapeUnicode, eJsonWriteOption::LowercaseHex }).GetValue(),
              R"({"k":"\ud55c/\uae00"})");

    // PrettyTwoSpaces는 Pretty를 덮어씀
    EXPECT_NE(writer.SaveToString(eJsonWriteOption::Pretty).GetValue().find("    \"k\""), std::string::npos);
    EXPECT_NE(writer.SaveToString(eJsonWriteOption::PrettyTwoSpaces).GetValue().find("  \"k\""), std::string::npos);
}

TEST(JsonWriter, MoveSemantics)
{
    JsonWriter writerA;
    writerA.BeginObject();
    writerA.WriteField("moved", 1);
    writerA.EndObject();

    JsonWriter writerB = std::move(writerA);

    SerializerResult<std::string> jsonOr = writerB.SaveToString();
    ASSERT_TRUE(jsonOr.HasValue());
    EXPECT_EQ(jsonOr.GetValue(), R"({"moved":1})");
}

// ==========================================================
//  File
// ==========================================================

TEST(JsonSerializer, FileRoundTrip)
{
    const std::filesystem::path path = std::filesystem::temp_directory_path() / std::filesystem::path { u8"jugx_한글_test.json" };

    JsonWriter writer;
    writer.BeginObject();
    writer.WriteField("saved", true);
    writer.WriteField("count", 12345);
    writer.EndObject();
    ASSERT_EQ(writer.SaveToFile(path, eJsonWriteOption::Pretty), eSerializerError::None);
    ASSERT_TRUE(std::filesystem::exists(path));

    SerializerResult<JsonReader> readerOr = JsonReader::LoadFromFile(path);
    ASSERT_TRUE(readerOr.HasValue());
    {
        JsonReader reader = readerOr.Take();
        reader.BeginObject();
        EXPECT_EQ(reader.ReadField<bool>("saved"), true);
        EXPECT_EQ(reader.ReadField<int>("count"), 12345);
        reader.EndObject();
        EXPECT_FALSE(reader.HasError());
    }

    std::filesystem::remove(path);
    EXPECT_TRUE(JsonReader::LoadFromFile(path).HasError());   // 없는 파일
}
