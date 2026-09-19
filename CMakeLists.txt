cmake_minimum_required(VERSION 3.25)

project(JugX
    VERSION 0.1.0
    DESCRIPTION "JugX utility library"
    HOMEPAGE_URL "https://github.com/JuggernautSensei/JugX"
    LANGUAGES CXX
)

# Treat as top-level only when not consumed via add_subdirectory().
if(PROJECT_IS_TOP_LEVEL)
    set(JUGX_DEFAULT_DEV ON)
else()
    set(JUGX_DEFAULT_DEV OFF)
endif()

option(JUGX_BUILD_TESTS      "Build JugX unit tests"  ${JUGX_DEFAULT_DEV})
option(JUGX_BUILD_BENCHMARKS "Build JugX benchmarks"  ${JUGX_DEFAULT_DEV})
option(JUGX_INSTALL          "Generate install rules" ${JUGX_DEFAULT_DEV})

include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(NOT CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
endif()

# ---------------------------------------------------------------------------
# Dependencies
# ---------------------------------------------------------------------------
find_package(SDL3 CONFIG REQUIRED)
find_package(utf8cpp CONFIG REQUIRED)
find_package(yyjson CONFIG REQUIRED)

# parallel-hashmap ships headers only -- vcpkg installs no CMake package for
# it, so locate the header directly instead of find_package().
find_path(JUGX_PHMAP_INCLUDE_DIR "parallel_hashmap/phmap.h" REQUIRED)

# ---------------------------------------------------------------------------
# Library target
# ---------------------------------------------------------------------------
# CONFIGURE_DEPENDS re-globs on build, so adding a .cpp does not require
# editing this file. Swap to an explicit list if reproducibility matters more.
file(GLOB JUGX_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/JugX/*.cpp")
file(GLOB JUGX_HEADERS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/JugX/*.h")

# CMake generates its own PCH translation unit; pch.cpp would be a duplicate.
list(REMOVE_ITEM JUGX_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/JugX/pch.cpp")

add_library(JugX STATIC ${JUGX_SOURCES} ${JUGX_HEADERS})
add_library(JugX::JugX ALIAS JugX)

target_compile_features(JugX PUBLIC cxx_std_20)

target_include_directories(JugX
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}>
        $<INSTALL_INTERFACE:${CMAKE_INSTALL_INCLUDEDIR}>
)

# PUBLIC, not PRIVATE: JugX public headers are not self-contained -- e.g.
# StringHasher.h uses StringView and EnumRefl.h uses JUG_COMPILER_* without
# including anything, so pch.h must be force-included into consumers too.
target_precompile_headers(JugX PUBLIC
    "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/JugX/pch.h>"
    "$<INSTALL_INTERFACE:$<INSTALL_PREFIX>/${CMAKE_INSTALL_INCLUDEDIR}/JugX/pch.h>"
)

# SDL3 and phmap appear in public headers (OS.h, Typedef.h); the rest do not.
target_link_libraries(JugX
    PUBLIC
        SDL3::SDL3
    PRIVATE
        utf8cpp::utf8cpp
        yyjson::yyjson
)

target_include_directories(JugX SYSTEM PUBLIC
    $<BUILD_INTERFACE:${JUGX_PHMAP_INCLUDE_DIR}>
)

# Config.h keys JUG_DEBUG off DEBUG/_DEBUG. MSVC supplies _DEBUG; other
# toolchains do not, so define it explicitly to keep behaviour identical.
target_compile_definitions(JugX PUBLIC $<$<CONFIG:Debug>:_DEBUG>)

# Matches CharacterSet=Unicode from the MSBuild project: OsAPI.h pulls in
# <windows.h>, and SystemError.cpp relies on the -W forms of the Win32 API.
if(WIN32)
    target_compile_definitions(JugX PUBLIC UNICODE _UNICODE)
endif()

target_compile_options(JugX PUBLIC
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8;/permissive-;/Zc:preprocessor>
)
target_compile_options(JugX PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/W3;/MP;/sdl>
    $<$<NOT:$<CXX_COMPILER_ID:MSVC>>:-Wall;-Wextra>
)

set_target_properties(JugX PROPERTIES
    POSITION_INDEPENDENT_CODE ON
    VERSION   ${PROJECT_VERSION}
    SOVERSION ${PROJECT_VERSION_MAJOR}
)

# ---------------------------------------------------------------------------
# Subprojects
# ---------------------------------------------------------------------------
if(JUGX_BUILD_TESTS)
    enable_testing()
    add_subdirectory(Test)
endif()

if(JUGX_BUILD_BENCHMARKS)
    add_subdirectory(Bench)
endif()

# ---------------------------------------------------------------------------
# Install / export
# ---------------------------------------------------------------------------
if(JUGX_INSTALL)
    install(TARGETS JugX
        EXPORT JugXTargets
        ARCHIVE DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        LIBRARY DESTINATION "${CMAKE_INSTALL_LIBDIR}"
        RUNTIME DESTINATION "${CMAKE_INSTALL_BINDIR}"
    )

    install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/JugX/"
        DESTINATION "${CMAKE_INSTALL_INCLUDEDIR}/JugX"
        FILES_MATCHING PATTERN "*.h" PATTERN "*.inl"
    )

    install(EXPORT JugXTargets
        FILE JugXTargets.cmake
        NAMESPACE JugX::
        DESTINATION "${CMAKE_INSTALL_DATADIR}/jugx"
    )

    configure_package_config_file(
        "${CMAKE_CURRENT_SOURCE_DIR}/cmake/JugXConfig.cmake.in"
        "${CMAKE_CURRENT_BINARY_DIR}/JugXConfig.cmake"
        INSTALL_DESTINATION "${CMAKE_INSTALL_DATADIR}/jugx"
    )

    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/JugXConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion
    )

    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/JugXConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/JugXConfigVersion.cmake"
        DESTINATION "${CMAKE_INSTALL_DATADIR}/jugx"
    )
endif()
