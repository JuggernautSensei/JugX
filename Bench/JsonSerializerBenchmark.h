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

#include "../Source/JsonReader.h"
#include "../Source/JsonWriter.h"

// jug::JsonReader/JsonWriter vs rapidjson vs nlohmann::json 벤치마크.
// 셋 다 같은 데이터(BenchItem 배열)를 Write/Parse 함.
// Parse 쪽은 세 라이브러리가 다 똑같은 문자열(BuildCanonicalJson)을 파싱해야 공정해서,
// 어느 라이브러리 Writer 결과물도 안 쓰고 직접 문자열을 만듦.

namespace bench
{

constexpr size_t kItemCount = 2000;

struct BenchItem
{
    int64_t                  id = 0;
    std::string              name;
    double                   score  = 0.0;
    bool                     active = false;
    std::vector<std::string> tags;

    // ----- jug -----
    void Serialize(
        jug::JsonWriter& _writer) const
    {
        _writer.BeginObject();
        _writer.WriteField("id", id);
        _writer.WriteField("name", name);
        _writer.WriteField("score", score);
        _writer.WriteField("active", active);
        _writer.BeginArray("tags");
        for (const std::string& tag: tags)
        {
            _writer.Write(tag);
        }
        _writer.EndArray();
        _writer.EndObject();
    }

    void Deserialize(
        jug::JsonReader& _reader)
    {
        _reader.BeginObject();
        _reader.ReadFieldTo("id", id);
        _reader.ReadFieldTo("name", name);
        _reader.ReadFieldTo("score", score);
        _reader.ReadFieldTo("active", active);

        _reader.BeginArray("tags");
        tags.clear();
        while (_reader.HasNext())
        {
            tags.push_back(_reader.Read<std::string>());
        }
        _reader.EndArray();
        _reader.EndObject();
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
inline std::vector<BenchItem> MakeItems()
{
    std::vector<BenchItem> items;
    items.reserve(kItemCount);
    for (size_t i = 0; i < kItemCount; ++i)
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

// ==========================================================
//  Write
// ==========================================================

inline void BM_Jug_Write(
    benchmark::State& _state)
{
    const std::vector<BenchItem> items = MakeItems();
    for (auto _: _state)
    {
        jug::JsonWriter writer;
        writer.BeginArray();
        for (const BenchItem& item: items)
        {
            writer.Write(item);
        }
        writer.EndArray();

        jug::SerializerResult<std::string> result = writer.SaveToString();
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BM_Jug_Write);

inline void BM_RapidJson_Write(
    benchmark::State& _state)
{
    const std::vector<BenchItem> items = MakeItems();
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
}

BENCHMARK(BM_RapidJson_Write);

inline void BM_Nlohmann_Write(
    benchmark::State& _state)
{
    const std::vector<BenchItem> items = MakeItems();
    for (auto _: _state)
    {
        nlohmann::json j      = items;
        std::string    result = j.dump();
        benchmark::DoNotOptimize(result);
    }
}

BENCHMARK(BM_Nlohmann_Write);

// ==========================================================
//  Parse
// ==========================================================

inline void BM_Jug_Parse(
    benchmark::State& _state)
{
    const std::string json = BuildCanonicalJson(MakeItems());
    for (auto _: _state)
    {
        jug::SerializerResult<jug::JsonReader> readerResult = jug::JsonReader::LoadFromString(json);
        jug::JsonReader&                       reader       = readerResult.GetValue();

        std::vector<BenchItem> items;
        reader.BeginArray();
        while (reader.HasNext())
        {
            items.push_back(reader.Read<BenchItem>());
        }
        reader.EndArray();
        benchmark::DoNotOptimize(items);
    }
}

BENCHMARK(BM_Jug_Parse);

inline void BM_RapidJson_Parse(
    benchmark::State& _state)
{
    const std::string json = BuildCanonicalJson(MakeItems());
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
}

BENCHMARK(BM_RapidJson_Parse);

inline void BM_Nlohmann_Parse(
    benchmark::State& _state)
{
    const std::string json = BuildCanonicalJson(MakeItems());
    for (auto _: _state)
    {
        nlohmann::json         j     = nlohmann::json::parse(json);
        std::vector<BenchItem> items = j.get<std::vector<BenchItem>>();
        benchmark::DoNotOptimize(items);
    }
}

BENCHMARK(BM_Nlohmann_Parse);

}   // namespace bench
