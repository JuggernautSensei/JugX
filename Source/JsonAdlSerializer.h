#pragma once

namespace jug
{

class JsonReader;
class JsonWriter;

// 내가 소유하지 않은 타입(third-party 등)을 위한 비침투적 직렬화 커스터마이징 지점.
//   static void            Serialize(JsonWriter&, const T&)
//   static SerializeResult<T> Deserialize(const JsonReader&)
//
// primary는 반드시 비어 있어야 함. "특수화가 있나?"를 concept으로 묻는 순간 JsonAdlSerializer<T>가
// 인스턴스화되는데, 여기에 static_assert를 넣으면 그게 SFINAE가 아니라 하드 에러가 되어서 특수화가
// 없는 아무 타입(int 등)을 probe하는 것만으로 컴파일이 터짐. 특수화가 없을 때의 에러 메시지는
// JsonReader::Read<T>() / JsonWriter::Write<T>()의 마지막 static_assert가 냄.
template<typename T>
struct JsonAdlSerializer
{
};

}   // namespace jug
