# JugX

**게임과 그래픽스 프로그램을 위한 C++20 유틸리티 라이브러리**

수학, 메모리 관리, 핸들, JSON, 파일 입출력, 앱 루프 등 여러 프로젝트에서 쓰는 공통 기능을 모았습니다. 각 기능의 사용법과 구현은 아래 필터별로 정리했습니다.

## 목차

[빌드 및 프로젝트 연결](#빌드) · [테스트와 벤치마크](#테스트와-벤치마크)

| 기능 | Visual Studio 필터 | 내용 |
| --- | --- | --- |
| [기반 정의](#base) | `Base` | 플랫폼·빌드 설정, 공통 타입, 매크로, assertion |
| [수학과 충돌](#math) | `Math` | 벡터, 행렬, 회전, 변환, 기하, SIMD |
| [메모리 관리](#memory) | `Memroy` | 할당, 버퍼, 메모리 뷰, 풀, 스마트 포인터 |
| [핸들](#handle) | `Handle` | 타입별 식별자, 발급·회수, 값 저장소 |
| [컨테이너](#container) | `Container` | 링 버퍼, LRU 캐시 |
| [선택적 값](#optional) | `Optional` | 값 또는 참조의 부재 표현 |
| [오류와 결과](#error) | `Error` | 오류 코드·범주, 값을 동반하는 결과 |
| [타입 도구](#type) | `Type` | 타입 이름·해시, concepts, 태그, invalid 인덱스 |
| [열거형 도구](#enum) | `Enum` | 이름·값 변환, 순회, 배열, 비트 플래그 |
| [호출 객체](#callable) | `Callable` | 함수·멤버 함수·람다 바인딩 |
| [이벤트](#event) | `Event` | 이벤트 타입 식별과 처리 분기 |
| [직렬화](#serializer) | `Serializer` | JSON 읽기·쓰기와 사용자 타입 연결 |
| [파일](#file) | `File` | 파일 소유권, 읽기·쓰기, 위치 이동 |
| [문자열](#string) | `String` | UTF 변환, 포매팅, 숫자 파싱 |
| [해시](#hash) | `Hash` | 문자열·메모리 해시, 이미 해시된 키 |
| [난수](#random) | `Random` | 생성기, 범위 난수, 벡터 샘플링 |
| [시간](#timer) | `Timer` | 구간·누적 시간 측정과 타임스탬프 |
| [로그](#log) | `Log` | spdlog 호출 매크로 |
| [OS](#os) | `OS` | 네이티브 윈도우 핸들과 시스템 정보 |
| [애플리케이션](#application) | `Application` | SDL3 초기화, 루프, 입력·윈도우 이벤트 |

예제의 네임스페이스는 `jug`입니다. [통합 헤더](JugX/JugX.h)는 `<JugX/JugX.h>`로 포함합니다. 아래 예제는 `JugX::JugX` CMake 타깃을 연결해 공통 헤더 `JugX/pch.h`도 적용한 프로젝트를 기준으로 합니다.

## Base

[Platform.h](JugX/Platform.h)는 OS, 컴파일러, CPU 아키텍처, 엔디언과 C++ 버전을 구분합니다. [Config.h](JugX/Config.h)는 `DEBUG` 또는 `_DEBUG` 정의로 빌드 구성을 결정하고, [Macro.h](JugX/Macro.h)는 클래스의 복사·이동 선언과 컴파일러별 속성 등을 묶습니다.

[Typedef.h](JugX/Typedef.h)의 `String`, `StringView`, `Vector`, `Span`, `FilePath` 등은 표준 라이브러리 타입의 별칭입니다. `HashMap`·`HashSet`은 parallel-hashmap을 사용합니다.

[Assert.h](JugX/Assert.h)의 `JUG_ASSERT`는 호출 위치·조건·메시지를 보고하고 디버거 중단점으로 계약 위반을 드러냅니다. 기본적으로 Debug에서 켜지며, `JUG_ENABLE_ASSERT`로 제어합니다. 검사 여부와 관계없이 함수의 전제조건은 호출자가 보장해야 합니다. [Fatal.h](JugX/Fatal.h)는 계속 실행할 수 없는 오류를 보고하는 경로를 제공합니다.

## Math

### LinearAlgebra — 벡터, 행렬, 회전

| 구성 | 기능 | 구현 |
| --- | --- | --- |
| [Vector](JugX/Vector.h) | 벡터 산술, 내적·외적, 정규화, 보간 | `VEC<T, N>`과 2·3·4차원 특수화. `VEC3` 등은 실수 벡터 별칭 |
| [Matrix](JugX/Matrix.h) | SRT, 뷰·투영, 역행렬, 분해, 점·방향 변환 | `MAT4`는 16바이트 정렬된 64바이트 행렬 |
| [Quaternion](JugX/Quaternion.h) | 회전 생성·합성, 보간, 행렬 변환 | `QUAT`는 16바이트 정렬된 16바이트 값 타입. `AxisAngle`은 축·각도 표현 |

연산은 `jug::Dot(a, b)`, `jug::Normalize(v)` 같은 자유 함수로 제공합니다. `MAT4`는 행 단위로 저장하며 벡터를 왼쪽에 두고 곱하는 방식입니다. `XformPoint()`는 위치를 `(x, y, z, 1)`로 변환하고 w로 나눕니다. `XformVector()`는 `(x, y, z, 0)`으로 변환해 이동 성분을 제외합니다.

```cpp
#include <JugX/JugX.h>

jug::VEC3 MovePoint(jug::VEC3 point, jug::VEC3 offset)
{
    const jug::MAT4 translation = jug::MAT4::MakeTranslation(offset);
    return jug::XformPoint(point, translation);
}
```

### Geometry — 도형과 충돌

[AABB](JugX/AABB.h), [OBB](JugX/OBB.h), [SPHERE](JugX/Sphere.h), [RAY](JugX/Ray.h), [PLANE](JugX/Plane.h), [TRIANGLE](JugX/Triangle.h), [FRUSTUM](JugX/Frustum.h), [INTERVAL](JugX/Interval.h)을 제공합니다. 예를 들어 `AABB`는 중심과 반크기(`extends`)를 저장하고 최소·최대 좌표는 계산해 돌려줍니다.

[Collision.h](JugX/Collision.h)는 도형 조합에 따라 다음 함수를 오버로드합니다.

| 함수 | 의미 |
| --- | --- |
| `ClosestPoint` | 도형 위에서 주어진 점과 가장 가까운 점 |
| `Raycast` | 지정 구간 안에서 광선과 도형의 교차 |
| `Intersect` / `Disjoint` | 접점을 포함한 겹침 판정 / 그 반대 |
| `Contains` | 첫 번째 도형이 두 번째 도형을 완전히 포함하는지 판정 |

박스 간 교차는 AABB·OBB를 중심·반크기·축으로 변환한 공통 표현에서 분리축 검사로 처리합니다. 충돌 검사는 객체 계층을 순회하는 대신, 전달된 도형 값에 대해 계산합니다.

### Base / Misc — SIMD와 보조 타입

[SIMD.h](JugX/SIMD.h)는 SSE·NEON·스칼라 구현에 같은 연산 인터페이스를 제공합니다. 경로는 컴파일 대상과 활성화된 명령어 집합으로 결정되며 `JUG_NO_SIMD`로 비활성화할 수 있습니다. 수학 함수는 `JUG_MATH_API`를 통해 CUDA 컴파일 시 host/device 속성도 사용합니다.

[TRANSFORM](JugX/Transform.h)은 이동·회전·스케일을 따로 저장하고 필요할 때 SRT 행렬로 변환합니다. [Float16](JugX/Float16.h)은 16비트 표현과 32비트 실수 사이의 변환, [RGBA](JugX/RGBA.h)는 색상 저장과 변환, [Math.h](JugX/Math.h)는 스칼라 연산과 상수를 제공합니다.

## Memory

### Alloc / Utils — 할당과 정렬

[Alloc.h](JugX/Alloc.h)는 일반·정렬 힙 할당인 `Malloc`·`Free`와 스택 할당 매크로를 제공합니다. [Align.h](JugX/Align.h)는 정렬 계산을 담당합니다. [MallocAllocator<T>](JugX/MallocAllocator.h)는 이 할당 경로를 STL allocator 인터페이스로 연결하며 할당 실패 시 `std::bad_alloc`을 던집니다.

### Buffer / MemoryView — 소유 버퍼와 비소유 뷰

[Buffer<T>](JugX/Buffer.h)는 메모리를 소유하는 이동 전용 버퍼입니다. 크기 변경과 초기화하지 않는 할당을 제공하며 `ByteBuffer`는 바이트 버퍼 별칭입니다. `kNoInit` 경로를 사용할 때는 객체의 생성과 소멸 조건도 호출자가 맞춰야 합니다.

[MemoryView·MutableMemoryView](JugX/MemoryView.h)는 바이트 포인터와 길이만 저장합니다. 배열·연속 컨테이너·객체를 바이트 범위로 전달할 수 있고, `SubView()`로 일부 구간을 참조합니다. 뷰는 메모리를 소유하지 않으므로 원본보다 오래 유지할 수 없습니다.

### MemoryPool — 용도에 따른 풀

| 타입 | 할당·반환 방식 | 사용 조건 |
| --- | --- | --- |
| [MemoryArena](JugX/MemoryArena.h) | 블록 안에서 커서를 전진해 할당하고 `Reset()`에서 전체 블록 해제 | 개별 반환 없음. 한 번의 할당은 지정 블록 크기 안에 들어가야 함 |
| [FixedBlockMemoryPool](JugX/FixedBlockMemoryPool.h) | 같은 크기의 블록을 할당하고 반납된 블록을 free list로 재사용 | 같은 풀에 정확히 한 번 반환. 동기화는 호출자 책임 |
| [ObjectPool<T>](JugX/ObjectPool.h) | 고정 블록 풀 위에서 `New()`로 생성, `Delete()`로 소멸·반납 | 살아 있는 객체를 풀보다 먼저 정리 |

`MemoryArena::Reset()`은 저장한 객체의 소멸자를 호출하지 않고 메모리를 해제합니다. 객체를 생성해 넣었다면 필요한 소멸 처리를 먼저 수행해야 합니다.

### SmartPointer — 단독 소유와 참조 카운팅

[Scoped<T, Delete>](JugX/Scoped.h)는 이동 전용 단독 소유 포인터입니다. 사용자 deleter를 지정할 수 있어 객체를 풀로 반납하는 경로에도 연결할 수 있습니다.

[Ref<T>](JugX/Ref.h)는 객체 내부의 [RefCounted](JugX/RefCount.h)를 사용하는 intrusive 참조 카운팅 포인터입니다. 복사 시 `Retain()`, 해제 시 `Release()`를 호출하며 별도 control block을 만들지 않습니다. 참조 수는 일반 `int`이므로 여러 스레드에서 공유할 때는 별도의 동기화가 필요합니다.

## Handle

### Handle — 타입이 있는 32비트 식별자

[Handle<T>](JugX/Handle.h)는 **24비트 인덱스 + 8비트 토큰**을 저장합니다. 타입 매개변수로 리소스 종류를 구분하고, `kInvalidHandle`로 invalid 상태를 표현합니다. 핸들은 리소스 주소나 소유권을 담지 않습니다.

[HandleAllocator](JugX/HandleAllocator.h)는 인덱스를 발급하고 회수합니다. 회수된 인덱스를 재사용할 때 토큰이 달라지므로 이전 핸들과 구분할 수 있습니다. 토큰은 8비트 순환 값이며 서로 다른 풀의 식별 정보까지 담지는 않습니다.

### HandleStorage / ResourcePool — 저장과 발급

[HandleStorage<H, V>](JugX/HandleStorage.h)는 sparse 배열로 핸들 인덱스를 dense 위치에 연결하고, 핸들과 값은 각각 연속 배열로 저장합니다. 조회할 때는 저장된 핸들 전체를 비교하므로 토큰도 함께 검사합니다. 삭제는 마지막 요소를 빈자리로 옮기는 swap-and-pop 방식입니다.

[ResourcePool<H, V>](JugX/ResourcePool.h)은 allocator와 storage를 묶어 값 생성과 핸들 발급, 삭제와 핸들 회수를 함께 수행합니다.

```cpp
#include <JugX/JugX.h>

struct TextureTag {};
using TextureHandle = jug::Handle<TextureTag>;

struct TextureInfo
{
    int width;
    int height;
};

void UseTexturePool()
{
    jug::ResourcePool<TextureHandle, TextureInfo> textures;
    const TextureHandle handle = textures.Emplace(TextureInfo { 256, 256 });

    textures.Get(handle).width = 512;
    textures.Erase(handle);
}
```

`Get()`·`Erase()`에는 해당 풀에서 살아 있는 핸들을 전달합니다. 부재를 허용하는 조회는 `GetOrNull()`을 사용합니다. 삽입·삭제로 값의 주소와 순서가 바뀔 수 있으므로 포인터·참조·순회 순서를 유지한다고 가정하면 안 됩니다.

## Container

### RingBuffer

[RingBuffer<T>](JugX/RingBuffer.h)는 연속 할당한 저장 공간에 head와 size를 두고 논리 인덱스를 실제 위치로 변환합니다. 양쪽 끝에서 삽입·삭제하고 논리 순서로 순회할 수 있습니다.

가득 찬 상태에서 `PushBack()`하면 앞 요소를, `PushFront()`하면 뒤 요소를 제거합니다. 최대 요소 수는 `GetMaxSize()`, 내부 저장 공간 크기는 `GetCapacity()`로 구분합니다. 비어 있는 버퍼의 `PopFront()`·`PopBack()`은 계약 위반입니다.

### LruCache

[LruCache<K, V>](JugX/LruCache.h)는 해시 맵으로 리스트 노드를 찾고, 리스트 앞쪽을 최근 사용 항목으로 유지합니다. 용량을 넘으면 뒤쪽 항목을 제거합니다.

`Find()`·`At()`은 조회한 항목을 앞으로 옮기며 `Peek()`는 순서를 바꾸지 않습니다. `Touch()`는 값 조회 없이 최근 사용 상태만 갱신합니다. 키가 없을 수 있으면 `Find()`의 결과를 `End()`와 비교하고, `At()`·`Touch()`에는 존재하는 키를 전달합니다.

## Optional

[Optional<T>](JugX/Optional.h)는 값과 존재 여부를 함께 저장하고 빈 상태를 `kNullopt`로 표현합니다. 일반 타입은 union 안의 객체 수명을 직접 관리하며, trivially copyable 타입은 별도 특수화로 복사·소멸 특성을 유지합니다.

`Optional<T&>`는 포인터 하나로 선택적 참조를 표현합니다. 객체를 소유하지 않으며 임시 객체로부터 생성할 수 없습니다. 복사해도 참조 대상의 수명이 늘어나지는 않습니다.

```cpp
#include <JugX/JugX.h>

jug::Optional<int&> FindPositive(jug::Span<int> values)
{
    for (int& value : values)
    {
        if (value > 0)
        {
            return value;
        }
    }
    return jug::kNullopt;
}
```

빈 상태가 정상적인 결과라면 `HasValue()` 또는 `operator bool()`로 분기합니다. `GetValue()`·`operator*()`·`operator->()`는 값이 있다는 전제에서 사용합니다. 값 타입의 `Take()`는 값을 꺼내고 Optional을 비웁니다.

## Error

[Error](JugX/Error.h)는 정수 오류 코드와 `IErrorCategory` 포인터를 저장합니다. 범주는 이름과 메시지 생성 방식을 담당하고, `ErrorCodeTraits` 또는 `JUG_DEFINE_ERROR_ENUM`으로 사용자 오류 열거형을 연결합니다. [SystemError](JugX/SystemError.h)는 errno·OS 오류를 같은 표현으로 변환합니다.

[Result<T>](JugX/Result.h)는 값·오류를 union에 저장하고 상태로 구분합니다. `Take()`로 값을 꺼내거나 `Reset()`한 뒤에는 Invalid 상태가 됩니다. `Result<void>`는 값 없이 성공·오류만 표현합니다.

파일·JSON 등 외부 입력의 실패는 `HasError()`·`GetError()`로 처리하고, `JUG_RETURN_IF_ERROR`로 호출자에게 반환할 수 있습니다. 내부 함수의 전제조건 위반은 `JUG_ASSERT`로 다룹니다. `Result<T>::GetValue()`에는 값 상태, `GetError()`에는 오류 상태가 요구됩니다.

## Type

[TypeTraits.h](JugX/TypeTraits.h)는 `ArithmeticT`, `PodT`, `CharT` 등의 concepts와 타입 판별 도구를 제공합니다. [Tag.h](JugX/Tag.h)는 `kNoInit` 같은 태그 값과 태그 정의 매크로를 둡니다. [InvalidIndex.h](JugX/InvalidIndex.h)는 정수 타입의 최댓값을 invalid 인덱스로 나타냅니다.

[TypeRefl.h](JugX/TypeRefl.h)의 `NameOf<T>()`·`FullNameOf<T>()`는 컴파일러의 함수 시그니처에서 타입 이름을 추출합니다. `HashOf<T>()`는 전체 이름을 해시합니다. 타입을 별도 레지스트리에 등록하지 않고 컴파일 타임에 계산하며 MSVC·Clang·GCC별 추출 코드를 둡니다.

이 이름과 해시는 컴파일러가 생성한 문자열에 의존합니다. 다른 컴파일러·빌드 사이에 유지할 파일 형식이나 네트워크 식별자로 사용하려면 별도 규약이 필요합니다.

## Enum

### EnumRefl — 이름, 값 변환, 순회

[EnumRefl.h](JugX/EnumRefl.h)는 컴파일 타임에 정수 범위를 탐색해 유효한 열거형 값과 이름을 구성합니다. 별도 등록 매크로 없이 개수·이름 조회, 문자열·정수에서의 변환, 값 순회를 제공합니다.

```cpp
#include <JugX/JugX.h>

enum class eState
{
    Idle,
    Running,
    Finished,
};

static_assert(jug::CountOf<eState>() == 3);
static_assert(jug::NameOf(eState::Running) == "Running");

jug::Optional<eState> ParseState(jug::StringView text)
{
    return jug::ParseAs<eState>(text);
}
```

등록되지 않은 이름·값에 대한 `ParseAs<E>()`는 빈 `Optional`을 반환합니다. `RangeOf<E>()`·`NamesOf<E>()`는 값의 오름차순이며 탐색 범위는 기본 **0~127**입니다. 범위를 바꾸려면 `JUG_ENUM_REFL_RANGE_MIN`·`JUG_ENUM_REFL_RANGE_MAX`를 프로젝트 전체에 일관되게 정의합니다.

### EnumArray / EnumFlags

[ENUM_ARRAY<E, V>](JugX/EnumArray.h)는 열거형 값으로 접근하는 고정 배열입니다. 0부터 연속된 enum은 정숫값으로 직접 접근하고, 값 사이에 간격이 있으면 `GetIndex()`로 배열 위치를 구합니다.

[Flags<E>](JugX/EnumFlags.h)는 enum 값 자체를 비트 마스크로, `IndexedFlags<E>`는 enum 값을 비트 위치로 사용합니다. `Has()`·`HasAny()`·`HasAll()`과 활성화·비활성화, 비트 연산을 제공합니다. `MakeFieldMask()`·`ExtractField()`는 여러 비트에 걸친 택일 필드를 다룹니다.

## Callable

[Callable<Signature, Size, Overflow>](JugX/Callable.h)는 함수 포인터, 멤버 함수, 람다·함수자를 같은 호출 인터페이스로 저장합니다. `Connect()`로 연결하거나 `Bind()`로 시그니처를 추론할 수 있습니다.

기본 내부 저장 공간은 **16바이트**입니다. 크기·정렬·nothrow 이동 조건을 만족하는 함수자는 그 안에 저장하고 나머지는 힙에 저장합니다. 세 번째 인자 `Overflow=false`를 지정하면 내부 저장 조건을 만족하는 함수자만 연결할 수 있습니다. 복사·이동·소멸은 저장 정책에 맞는 관리 함수로 처리합니다.

```cpp
#include <JugX/JugX.h>

int ApplyBias(int value)
{
    jug::Callable<int(int), 16, false> addBias;
    addBias.Connect([bias = 3](int input) { return input + bias; });
    return addBias(value);
}
```

연결하지 않은 Callable을 호출하는 것은 계약 위반입니다. 멤버 함수 바인딩은 대상 객체의 주소만 저장하므로, 호출할 때까지 대상이 살아 있어야 합니다.

## Event

[Event](JugX/Event.h)는 처리 여부와 타입 이름·해시를 제공하는 기반 클래스입니다. `JUG_EVENT_BODY(Type)`가 타입별 메타데이터와 조회 함수를 정의합니다.

[EventDispatcher](JugX/EventDispatcher.h)는 이벤트 하나를 받아 `Dispatch<T>()`의 타입 해시가 일치할 때만 함수를 호출합니다. `SetHandled(true)`로 처리 표시한 이벤트는 이후 dispatch를 건너뜁니다. 리스너 목록이나 이벤트 큐를 소유하지 않고, 현재 이벤트의 타입에 맞는 처리 경로를 선택합니다.

```cpp
#include <JugX/JugX.h>

void HandleQuit(jug::Event& event)
{
    jug::EventDispatcher dispatcher { event };
    dispatcher.Dispatch<jug::QuitEvent>([](jug::QuitEvent& quit)
    {
        quit.SetHandled(true);
    });
}
```

## Serializer

### Json — 문서 소유자와 값 접근자

| 타입 | 역할 |
| --- | --- |
| [JsonSerializer](JugX/JsonSerializer.h) | yyjson mutable 문서 소유, 문자열·파일 저장 |
| `JsonWriter` | 문서 안의 값을 설정하고 객체 필드·배열 요소 쓰기 |
| [JsonDeserializer](JugX/JsonDeserializer.h) | 문자열·파일을 파싱한 yyjson 문서 소유 |
| `JsonReader` | 값·필드 읽기, 배열·객체 순회 |
| [JsonString](JugX/JsonString.h) | 출력 문자열의 소유권과 해제 |

문서 소유자는 이동 전용이고 Reader·Writer는 문서를 소유하지 않는 값 접근자입니다. Reader는 값 포인터 하나, Writer는 문서·값 포인터 두 개를 저장합니다. 문서 소유자보다 오래 유지할 수 없습니다. Reader가 반환한 문자열 뷰도 원본 문서의 수명에 의존합니다.

`JsonWriter`는 `SetObject()`·`SetArray()`로 종류를 정한 뒤 `WriteField()`·`PushBack()`으로 채웁니다. 외부 JSON의 파싱 실패·필드 누락·타입 불일치는 [SerializeError](JugX/SerializeError.h)와 `Result<T>`로 반환합니다.

```cpp
#include <JugX/JugX.h>

jug::Result<int> ReadWidth(jug::StringView json)
{
    auto document = jug::JsonDeserializer::Load(json);
    JUG_RETURN_IF_ERROR(document);
    return document->GetReader().ReadField<int>("width");
}
```

### 사용자 타입 연결

소유한 타입은 아래 인터페이스를 구현해 연결합니다.

- 쓰기: `void Serialize(jug::JsonWriter&) const`
- 읽기: `static jug::Result<T> Deserialize(const jug::JsonReader&)`

타입을 수정할 수 없으면 [JsonAdlSerializer<T>](JugX/JsonAdlSerializer.h)를 특수화합니다. Writer·Reader는 concepts로 해당 인터페이스를 선택합니다. 객체·배열 구성과 사용자 타입의 왕복 변환 예제는 [JSON 테스트](Test/JsonSerializerTest.cpp)에 있습니다.

## File

[FileReaderWriter](JugX/FileReaderWriter.h)는 `FILE*`를 소유하는 이동 전용 기반 클래스입니다. 소멸 시 파일을 닫고, `Seek()`·`Tell()`·`GetSize()`·`GetRemain()` 등 공통 위치 연산을 제공합니다.

[FileReader](JugX/FileReader.h)는 지정 버퍼·구분자 기반 읽기를, [FileWriter](JugX/FileWriter.h)는 바이트·문자열·포맷 출력과 flush를 담당합니다. 팩토리 `Open()`은 실패를 `Result<T>`로 반환하며 읽기·쓰기 결과에는 실제 처리한 바이트 수가 들어갑니다.

전체 파일은 `ReadFile()`로 `ByteBuffer`에 읽거나 `WriteFile()`로 저장합니다. 빈 파일은 빈 버퍼, 빈 데이터 쓰기는 빈 파일로 처리합니다. [File.h](JugX/File.h)는 저수준 seek·tell·파일 잠금·잠금 없는 문자 입출력을 모읍니다.

## String

| 구성 | 기능과 반환 정보 |
| --- | --- |
| [StringEncoder](JugX/StringEncoder.h) | utfcpp 기반 UTF-8·UTF-16 변환. 문자열 반환, append, 지정 버퍼 출력 |
| [StringFormat](JugX/StringFormat.h) | `std::format` 기반 문자열 생성·append·버퍼 출력. `ZFormatTo()`는 널 종료 포함 |
| [StringParser](JugX/StringParser.h) | `std::from_chars` 기반 정수·실수 변환과 `Result<T>` 반환 |

버퍼 출력은 필요한 전체 길이(`totalSize`)와 실제 쓴 길이(`written`)를 구분합니다. 길이가 부족하면 출력이 잘릴 수 있으므로, 전체 결과가 필요한 호출자는 두 값을 비교합니다.

## Hash

[StringHasher.h](JugX/StringHasher.h)의 `HashString()`은 컴파일 타임에도 사용할 수 있는 64비트 FNV-1a 문자열 해시입니다. 타입·열거형 이름의 해시에도 사용합니다.

[MemoryHasher.h](JugX/MemoryHasher.h)는 `Fnv1a`·`Murmur3`를 제공합니다. 여러 바이트 구간을 `Mix()`하고 `Finalize()`로 결과를 얻거나 `Hash<H>()`로 한 번에 계산합니다. [NoopHasher](JugX/NoopHasher.h)는 이미 해시된 키를 `size_t`로 전달해 다시 해시하지 않습니다.

해시는 충돌 가능한 값입니다. 객체를 메모리 뷰로 해시할 때는 패딩 바이트까지 포함한 실제 바이트 표현이 입력이 됩니다.

## Random

[Random.h](JugX/Random.h)는 `RngMwc`, `RngShr3`, `RngMt19937`과 `Random<T>()`를 제공합니다. 생성기가 `uint64_t Generate()`를 제공하면 `RngT` concept을 통해 공통 함수에 연결할 수 있습니다.

생성기에 명시적 시드를 주거나 `kRandomDeviceSeed`로 시드를 만들 수 있습니다. 정수·실수의 범위 지정과 단위 구·반구의 내부 또는 표면 벡터 샘플링 함수도 같은 헤더에 있습니다. 생성기 상태는 객체가 보관하며 범위 인자는 최소값이 최댓값 이하라는 전제를 갖습니다.

## Timer

[Timer](JugX/Timer.h)는 `steady_clock`으로 경과 시간을 재고 내부 카운트를 나노초 단위로 저장합니다. `Lap()`은 직전 구간과 누적 시간을 갱신하며 `Stop()`은 마지막 구간을 반영한 뒤 측정을 멈춥니다. Getter 자체가 현재 시각을 다시 측정하지는 않습니다.

`TimeCast()`·`TimeCastF()`는 단위를 변환하고, `ToReadableTime()`은 크기에 맞는 단위와 값을 묶어 반환합니다. [TimeStamp](JugX/TimeStamp.h)는 `Now()`로 연·월·일·시·분·초 등 달력 시각을 얻습니다.

## Log

[CoreLogger.h](JugX/CoreLogger.h)는 spdlog의 trace·info·warn·error·critical에 대응하는 `JUG_CORE_LOG_*` 매크로를 제공합니다. 포매팅에는 표준 `std::format`을 사용하도록 설정합니다.

로거의 출력 대상과 수명 관리는 `App`의 초기화·종료 과정에서 구성합니다. `AppConfig`로 콘솔·시스템·파일·메모리 출력을 선택할 수 있습니다.

## OS

[OS.h](JugX/OS.h)는 SDL 윈도우 포인터·ID에서 네이티브 윈도우 핸들을 얻는 함수와 OS 버전·CPU 이름 조회를 제공합니다. [OsInclude.h](JugX/OsInclude.h)는 Windows 헤더와 관련 매크로를 모읍니다.

현재 네이티브 윈도우 핸들 구현은 Windows를 대상으로 합니다. 플랫폼 감지 매크로가 존재하는 것과 전체 라이브러리를 해당 OS에서 빌드할 수 있다는 것은 별개입니다.

## Application

[App](JugX/App.h)은 SDL3와 로거를 초기화하고 메인 루프를 실행합니다. 파생 클래스가 `Init()`·`Update(deltaTimeSec)`·`Shutdown()`·`OnEvent()`·`OnSystemEvent()`를 필요한 만큼 구현합니다. 살아 있는 `App` 인스턴스는 하나를 전제로 합니다.

프레임은 **시간 갱신 → SDL 이벤트 처리 → 메인 스레드 명령 실행 → Update** 순서로 진행합니다. 이벤트·명령 처리 중 `Quit()`이 호출되면 그 프레임의 `Update()` 전에 루프를 종료합니다.

[AppEvent.h](JugX/AppEvent.h)와 [KeyCode.h](JugX/KeyCode.h)는 앱·윈도우·키보드·마우스·게임패드 이벤트와 키 코드를 정의합니다. 다른 스레드에서 `ExecuteOnMainThread()`로 전달한 Callable은 mutex로 보호된 대기 목록에 들어가고 메인 루프에서 실행됩니다.

## 빌드

현재 전체 라이브러리 빌드는 **Windows**를 대상으로 합니다. 플랫폼 구분 코드와 다른 컴파일러용 프리셋도 있지만, `OS.cpp`의 네이티브 윈도우 접근 등에는 Windows 전용 구현이 있습니다.

필요한 도구는 다음과 같습니다.

- C++20을 지원하는 MSVC. 포함된 Visual Studio 프로젝트는 `v145` 도구 집합을 사용합니다.
- CMake 3.25 이상, Ninja, vcpkg.
- MSVC 개발자 명령 환경. 아래 명령은 그 환경의 PowerShell에서 실행합니다.

### CMake + vcpkg

[JugX 저장소](https://github.com/JuggernautSensei/JugX)를 받은 뒤, `CMakeLists.txt`가 있는 JugX 루트에서 실행합니다. 다른 프로젝트의 Vendor 안에 들어 있는 경우에도 같은 위치를 기준으로 합니다.

```powershell
$env:VCPKG_ROOT = "C:\path\to\vcpkg"

cmake --preset msvc
cmake --build --preset msvc-debug
ctest --preset msvc-debug
```

프리셋은 vcpkg manifest의 `test`·`bench` 기능을 활성화합니다. 최초 구성 시 의존성을 설치하고 라이브러리, 테스트, 벤치마크 타깃을 생성합니다.

| 의존성 | 용도 |
| --- | --- |
| SDL3 | 앱 루프, 시스템 이벤트, OS 기능 |
| parallel-hashmap | 해시 컨테이너 |
| utfcpp | 문자열 인코딩 변환 |
| yyjson | JSON 파싱·출력 |
| spdlog | 로깅 |
| Google Test | 단위 테스트 |
| Google Benchmark | 벤치마크 |
| nlohmann/json, RapidJSON | 테스트와 JSON 벤치마크 비교 대상 |

### 다른 프로젝트에 연결

사용하는 프로젝트에서 위의 기본 라이브러리 의존성을 찾을 수 있도록 vcpkg toolchain과 manifest를 설정한 뒤 연결합니다.

```cmake
add_subdirectory(Vendor/JugX)

add_executable(MyApp Main.cpp)
target_link_libraries(MyApp PRIVATE JugX::JugX)
```

`Vendor/JugX`는 실제 체크아웃 위치에 맞게 변경합니다. `add_subdirectory()`로 연결할 때는 테스트·벤치마크·설치 옵션이 기본적으로 꺼집니다.

| CMake 옵션 | 단독 빌드 기본값 | 용도 |
| --- | --- | --- |
| `JUGX_BUILD_TESTS` | `ON` | `JugXTest` 생성 |
| `JUGX_BUILD_BENCHMARKS` | `ON` | `JugXBench` 생성 |
| `JUGX_INSTALL` | `ON` | 설치·패키지 export 규칙 생성 |

## 테스트와 벤치마크

[Test/](Test/)에는 수학, JSON, 열거형 리플렉션, 링 버퍼, LRU 캐시 테스트가 있습니다. JSON 테스트는 문서 소유권과 Reader·Writer의 크기·복사 특성도 검사합니다.

[Bench/](Bench/)에는 DirectXMath와의 수학 연산 비교, yyjson 기반 JugX API와 RapidJSON·nlohmann/json의 JSON 작업 비교가 있습니다. 벤치마크는 최적화가 적용된 Release 빌드에서 실행합니다.

```powershell
cmake --build --preset msvc-release
.\build\msvc\bin\Release\JugXBench.exe
```

결과는 컴파일러, CPU, 빌드 옵션과 입력 데이터에 따라 달라집니다. 저장소의 벤치마크 코드로 필요한 연산을 직접 비교할 수 있습니다.
