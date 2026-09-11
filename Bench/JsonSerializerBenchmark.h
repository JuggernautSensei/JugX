#pragma once
#include <cstdint>
#include <format>
#include <string>
#include <vector>

#include <benchmark/benchmark.h>
#include <nlohmann/json.hpp>
#include <rapidjson/document.h>
#include <rapidjson/rapidjson.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "../Source/JsonDeserializer.h"
#include "../Source/JsonSerializer.h"

// jug::JsonReader/JsonWriter vs rapidjson vs nlohmann::json 벤치마크.
//
//   Write/Parse    : BenchItem 배열 전체 직렬화/역직렬화. ->Arg(N)으로 원소 개수를 바꿔가며
//                     측정해서 스케일링(선형인지)까지 같이 봄. 세 라이브러리 다 같은 데이터
//                     (BuildCanonicalJson)를 파싱해야 공정해서, Parse 쪽은 어느 라이브러리
//                     Writer 결과물도 안 쓰고 직접 문자열을 만듦.
//   FindField      : 필드 하나 조회하는 비용만 격리해서 잼(파싱은 루프 밖에서 한 번만) -
//                     yyjson/rapidjson은 오브젝트 키를 선형 탐색하고 nlohmann은 기본적으로
//                     std::map(정렬 트리) 기반이라, 필드 개수가 많을 때 이 셋의 차이가 큼.
//   IterateOnly    : 원소를 구조체로 변환하지 않고 그냥 훑기만 하는 비용 - Parse 시간 중
//                     "트리 순회" 몫과 "필드별 Read<T>/타입 변환" 몫을 나눠서 보려는 용도.
//
// SetItemsProcessed/SetBytesProcessed로 ns/op 말고 items/sec, bytes/sec도 같이 리포트함.

namespace bench
{

using jug::Failed;   // JUG_DISPATCH_FAILED expands to unqualified `Failed{...}` - needs this in scope outside namespace jug.

struct BenchItem
{
    int64_t                  id = 0;
    std::string              name;
    double                   score  = 0.0;
    bool                     active = false;
    std::vector<std::string> tags;

    void Serialize(
        jug::JsonWriter& _writer) const
    {
        _writer.SetObject();
        _writer.WriteField("id", id);
        _writer.WriteField("name", name);
        _writer.WriteField("score", score);
        _writer.WriteField("active", active);

        jug::JsonWriter tagArray = _writer.BeginArrayField("tags");
        for (const std::string& tag: tags)
        {
            tagArray.PushBack(tag);
        }
    }

    [[nodiscard]] static jug::SerializeResult<BenchItem> Deserialize(
        const jug::JsonReader& _reader)
    {
        jug::SerializeResult<int64_t> id = _reader.ReadField<int64_t>("id");
        JUG_DISPATCH_FAILED(id);
        jug::SerializeResult<std::string> name = _reader.ReadField<std::string>("name");
        JUG_DISPATCH_FAILED(name);
        jug::SerializeResult<double> score = _reader.ReadField<double>("score");
        JUG_DISPATCH_FAILED(score);
        jug::SerializeResult<bool> active = _reader.ReadField<bool>("active");
        JUG_DISPATCH_FAILED(active);

        jug::SerializeResult<jug::JsonReader> tagArray = _reader.FindField("tags");
        JUG_DISPATCH_FAILED(tagArray);

        std::vector<std::string> tags;
        tags.reserve(tagArray.GetValue().GetSize());
        for (const jug::JsonReader& element: tagArray.GetValue().GetArray())
        {
            jug::SerializeResult<std::string> tag = element.Read<std::string>();
            JUG_DISPATCH_FAILED(tag);
            tags.push_back(tag.Take());
        }

        return BenchItem { id.GetValue(), name.Take(), score.GetValue(), active.GetValue(), std::move(tags) };
    }
};

// ----- nlohmann (ADL to_json/from_json) -----
inline void to_json(
    nlohmann::json&  j,
    const BenchItem& item)
{
    j = nlohmann::json {
        {     "id",     item.id },
        {   "name",   item.name },
        {  "score",  item.score },
        { "active", item.active },
        {   "tags",   item.tags },
    };
}

inline void from_json(
    const nlohmann::json& j,
    BenchItem&            item)
{
    j.at("id").get_to(item.id);
    j.at("name").get_to(item.name);
    j.at("score").get_to(item.score);
    j.at("active").get_to(item.active);
    j.at("tags").get_to(item.tags);
}

// ----- rapidjson (수동 변환, ADL 관례가 없어서 직접 함수로) -----
inline rapidjson::Value ToRapidJson(
    const BenchItem&                    _item,
    rapidjson::Document::AllocatorType& _alloc)
{
    rapidjson::Value v(rapidjson::kObjectType);
    v.AddMember("id", _item.id, _alloc);
    v.AddMember("name", rapidjson::Value(_item.name.c_str(), static_cast<rapidjson::SizeType>(_item.name.size()), _alloc), _alloc);
    v.AddMember("score", _item.score, _alloc);
    v.AddMember("active", _item.active, _alloc);

    rapidjson::Value tags(rapidjson::kArrayType);
    for (const std::string& tag: _item.tags)
    {
        tags.PushBack(rapidjson::Value(tag.c_str(), static_cast<rapidjson::SizeType>(tag.size()), _alloc), _alloc);
    }
    v.AddMember("tags", tags, _alloc);
    return v;
}

inline BenchItem FromRapidJson(
    const rapidjson::Value& _v)
{
    BenchItem item;
    item.id     = _v["id"].GetInt64();
    item.name   = _v["name"].GetString();
    item.score  = _v["score"].GetDouble();
    item.active = _v["active"].GetBool();
    for (const auto& tag: _v["tags"].GetArray())
    {
        item.tags.emplace_back(tag.GetString());
    }
    return item;
}

// ----- 테스트 데이터 -----
inline std::vector<BenchItem> MakeItems(
    const size_t _count)
{
    std::vector<BenchItem> items;
    items.reserve(_count);
    for (size_t i = 0; i < _count; ++i)
    {
        BenchItem item;
        item.id     = static_cast<int64_t>(i);
        item.name   = std::format("item_{}", i);
        item.score  = static_cast<double>(i) * 1.5 + 0.25;
        item.active = (i % 2 == 0);
        item.tags   = { "tag_a", "tag_b", "tag_c" };
        items.push_back(std::move(item));
    }
    return items;
}

inline std::string BuildCanonicalJson(
    const std::vector<BenchItem>& _items)
{
    std::string out;
    out.reserve(_items.size() * 96);
    out += '[';
    bool bFirst = true;
    for (const BenchItem& item: _items)
    {
        if (!bFirst)
        {
            out += ',';
        }
        bFirst = false;

        out += std::format(
            R"({{"id":{},"name":"{}","score":{},"active":{},"tags":[)",
            item.id,
            item.name,
            item.score,
            item.active);

        for (size_t i = 0; i < item.tags.size(); ++i)
        {
            if (i != 0)
            {
                out += ',';
            }
            out += '"';
            out += item.tags[i];
            out += '"';
        }
        out += "]}";
    }
    out += ']';
    return out;
}

inline std::string BuildWideObjectJson(
    const size_t _fieldCount)
{
    std::string out = "{";
    for (size_t i = 0; i < _fieldCount; ++i)
    {
        if (i != 0)
        {
            out += ',';
        }
        out += std::format(R"("field_{}":{})", i, i);
    }
    out += '}';
    return out;
}

// ==========================================================
//  Write
// ==========================================================

inline void BM_Jug_Write(
    benchmark::State& _state)
{
    const size_t                  count = static_cast<size_t>(_state.range(0));
    const std::vector<BenchItem>  items = MakeItems(count);
    const int64_t                 bytesPerOp = static_cast<int64_t>(BuildCanonicalJson(items).size());

    for (auto _: _state)
    {
        jug::JsonSerializer serializer;
        jug::JsonWriter     writer = serializer.GetWriter();
        writer.SetArray();
        for (const BenchItem& item: items)
        {
            writer.PushBack(item);
        }

        jug::SerializeResult<std::string> result = serializer.SaveToString();
        benchmark::DoNotOptimize(result);
    }

    _state.SetItemsProcessed(_state.iterations() * static_cast<int64_t>(count));
    _state.SetBytesProcessed(_state.iterations() * bytesPerOp);
}

BENCHMARK(BM_Jug_Write)->Arg(100)->Arg(1000)->Arg(10000);

inline void BM_RapidJson_Write(
    benchmark::State& _state)
{
    const size_t                 count      = static_cast<size_t>(_state.range(0));
    const std::vector<BenchItem> items      = MakeItems(count);
    const int64_t                bytesPerOp = static_cast<int64_t>(BuildCanonicalJson(items).size());

    for (auto _: _state)
    {
        rapidjson::Document doc(rapidjson::kArrayType);
        auto&               alloc = doc.GetAllocator();
        for (const BenchItem& item: items)
        {
            doc.PushBack(ToRapidJson(item, alloc), alloc);
        }

        rapidjson::StringBuffer                    buffer;
        rapidjson::Writer<rapidjson::StringBuffer> jsonWriter(buffer);
        doc.Accept(jsonWriter);

        std::string result = buffer.GetString();
        benchmark::DoNotOptimize(result);
    }

    _state.SetItemsProcessed(_state.iterations() * static_cast<int64_t>(count));
    _state.SetBytesProcessed(_state.iterations() * bytesPerOp);
}

BENCHMARK(BM_RapidJson_Write)->Arg(100)->Arg(1000)->Arg(10000);

inline void BM_Nlohmann_Write(
    benchmark::State& _state)
{
    const size_t                 count      = static_cast<size_t>(_state.range(0));
    const std::vector<BenchItem> items      = MakeItems(count);
    const int64_t                bytesPerOp = static_cast<int64_t>(BuildCanonicalJson(items).size());

    for (auto _: _state)
    {
        nlohmann::json j      = items;
        std::string    result = j.dump();
        benchmark::DoNotOptimize(result);
    }

    _state.SetItemsProcessed(_state.iterations() * static_cast<int64_t>(count));
    _state.SetBytesProcessed(_state.iterations() * bytesPerOp);
}

BENCHMARK(BM_Nlohmann_Write)->Arg(100)->Arg(1000)->Arg(10000);

// ==========================================================
//  Parse
// ==========================================================

inline void BM_Jug_Parse(
    benchmark::State& _state)
{
    const size_t      count = static_cast<size_t>(_state.range(0));
    const std::string json  = BuildCanonicalJson(MakeItems(count));

    for (auto _: _state)
    {
        jug::SerializeResult<jug::JsonDeserializer> result = jug::JsonDeserializer::LoadFromString(json);
        const jug::JsonReader                       reader = result.GetValue().GetReader();

        std::vector<BenchItem> items;
        items.reserve(count);
        for (const jug::JsonReader& element: reader.GetArray())
        {
            if (jug::SerializeResult<BenchItem> item = element.Read<BenchItem>())
            {
                items.push_back(item.Take());
            }
        }
        benchmark::DoNotOptimize(items);
    }

    _state.SetItemsProcessed(_state.iterations() * static_cast<int64_t>(count));
    _state.SetBytesProcessed(_state.iterations() * static_cast<int64_t>(json.size()));
}

BENCHMARK(BM_Jug_Parse)->Arg(100)->Arg(1000)->Arg(10000);

inline void BM_RapidJson_Parse(
    benchmark::State& _state)
{
    const size_t      count = static_cast<size_t>(_state.range(0));
    const std::string json  = BuildCanonicalJson(MakeItems(count));

    for (auto _: _state)
    {
        rapidjson::Document doc;
        doc.Parse(json.c_str());

        std::vector<BenchItem> items;
        items.reserve(doc.Size());
        for (const auto& v: doc.GetArray())
        {
            items.push_back(FromRapidJson(v));
        }
        benchmark::DoNotOptimize(items);
    }

    _state.SetItemsProcessed(_state.iterations() * static_cast<int64_t>(count));
    _state.SetBytesProcessed(_state.iterations() * static_cast<int64_t>(json.size()));
}

BENCHMARK(BM_RapidJson_Parse)->Arg(100)->Arg(1000)->Arg(10000);

inline void BM_Nlohmann_Parse(
    benchmark::State& _state)
{
    const size_t      count = static_cast<size_t>(_state.range(0));
    const std::string json  = BuildCanonicalJson(MakeItems(count));

    for (auto _: _state)
    {
        nlohmann::json         j     = nlohmann::json::parse(json);
        std::vector<BenchItem> items = j.get<std::vector<BenchItem>>();
        benchmark::DoNotOptimize(items);
    }

    _state.SetItemsProcessed(_state.iterations() * static_cast<int64_t>(count));
    _state.SetBytesProcessed(_state.iterations() * static_cast<int64_t>(json.size()));
}

BENCHMARK(BM_Nlohmann_Parse)->Arg(100)->Arg(1000)->Arg(10000);

// ==========================================================
//  IterateOnly - 구조체 변환 없이 그냥 훑기만(Parse 시간 중 "트리 순회" 몫만 격리)
// ==========================================================

inline void BM_Jug_IterateOnly(
    benchmark::State& _state)
{
    constexpr size_t  kCount = 10000;
    const std::string json   = BuildCanonicalJson(MakeItems(kCount));

    for (auto _: _state)
    {
        jug::SerializeResult<jug::JsonDeserializer> result = jug::JsonDeserializer::LoadFromString(json);
        const jug::JsonReader                       reader = result.GetValue().GetReader();

        size_t count = 0;
        for (const jug::JsonReader& element: reader.GetArray())
        {
            benchmark::DoNotOptimize(element);
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }

    _state.SetItemsProcessed(_state.iterations() * static_cast<int64_t>(kCount));
}

BENCHMARK(BM_Jug_IterateOnly);

inline void BM_RapidJson_IterateOnly(
    benchmark::State& _state)
{
    constexpr size_t  kCount = 10000;
    const std::string json   = BuildCanonicalJson(MakeItems(kCount));

    for (auto _: _state)
    {
        rapidjson::Document doc;
        doc.Parse(json.c_str());

        size_t count = 0;
        for (const auto& v: doc.GetArray())
        {
            benchmark::DoNotOptimize(v);
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }

    _state.SetItemsProcessed(_state.iterations() * static_cast<int64_t>(kCount));
}

BENCHMARK(BM_RapidJson_IterateOnly);

inline void BM_Nlohmann_IterateOnly(
    benchmark::State& _state)
{
    constexpr size_t  kCount = 10000;
    const std::string json   = BuildCanonicalJson(MakeItems(kCount));

    for (auto _: _state)
    {
        nlohmann::json j = nlohmann::json::parse(json);

        size_t count = 0;
        for (const auto& v: j)
        {
            benchmark::DoNotOptimize(v);
            ++count;
        }
        benchmark::DoNotOptimize(count);
    }

    _state.SetItemsProcessed(_state.iterations() * static_cast<int64_t>(kCount));
}

BENCHMARK(BM_Nlohmann_IterateOnly);

// ==========================================================
//  FindField - 필드 하나 조회하는 비용만 격리(파싱은 루프 밖에서 한 번만)
// ==========================================================

inline void BM_Jug_FindField(
    benchmark::State& _state)
{
    constexpr size_t  kFieldCount = 1000;
    const std::string json        = BuildWideObjectJson(kFieldCount);
    const std::string key         = std::format("field_{}", kFieldCount / 2);   // 중간 키 - 선형 탐색 시 최악에 가까움

    jug::SerializeResult<jug::JsonDeserializer> docResult = jug::JsonDeserializer::LoadFromString(json);
    const jug::JsonReader                       reader    = docResult.GetValue().GetReader();

    for (auto _: _state)
    {
        jug::SerializeResult<int> value = reader.ReadField<int>(key);
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK(BM_Jug_FindField);

inline void BM_RapidJson_FindField(
    benchmark::State& _state)
{
    constexpr size_t  kFieldCount = 1000;
    const std::string json        = BuildWideObjectJson(kFieldCount);
    const std::string key         = std::format("field_{}", kFieldCount / 2);

    rapidjson::Document doc;
    doc.Parse(json.c_str());

    for (auto _: _state)
    {
        auto it    = doc.FindMember(key.c_str());
        int  value = (it != doc.MemberEnd()) ? it->value.GetInt() : 0;
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK(BM_RapidJson_FindField);

inline void BM_Nlohmann_FindField(
    benchmark::State& _state)
{
    constexpr size_t  kFieldCount = 1000;
    const std::string json        = BuildWideObjectJson(kFieldCount);
    const std::string key         = std::format("field_{}", kFieldCount / 2);

    const nlohmann::json j = nlohmann::json::parse(json);

    for (auto _: _state)
    {
        int value = j.at(key).get<int>();
        benchmark::DoNotOptimize(value);
    }
}

BENCHMARK(BM_Nlohmann_FindField);

}   // namespace bench
