cmake_minimum_required(VERSION 3.20)
project(iris-server LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG        v1.14.0
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG        v3.11.3
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(googletest nlohmann_json)

enable_testing()

add_library(iris_lib STATIC
    src/adapters/tcp_listener.cpp
    src/adapters/file_corpus.cpp
    src/adapters/dispatcher.cpp
    src/request_handler.cpp
)

target_include_directories(iris_lib PUBLIC src)

target_link_libraries(iris_lib PUBLIC
    nlohmann_json::nlohmann_json
)

if(WIN32)
    target_link_libraries(iris_lib PUBLIC ws2_32)
endif()

add_executable(iris_tests
    tests/test_container.cpp
    tests/test_framer.cpp
    tests/test_serializer.cpp
    tests/test_dispatcher.cpp
    tests/test_api_functions.cpp
    tests/test_fibonacci.cpp
    tests/test_to_base.cpp
    tests/test_tfidf.cpp
    tests/test_file_corpus.cpp
)

target_link_libraries(iris_tests PRIVATE
    iris_lib
    GTest::gtest_main
)

include(GoogleTest)
gtest_discover_tests(iris_tests DISCOVERY_MODE PRE_TEST)

# --- Server executable ---
add_executable(iris_server
    src/main.cpp
)

target_link_libraries(iris_server PRIVATE
    iris_lib
)

# Static linking on MinGW to avoid DLL version conflicts
if(MINGW)
    target_link_options(iris_tests PRIVATE -static-libgcc -static-libstdc++ -static)
    target_link_options(iris_server PRIVATE -static-libgcc -static-libstdc++ -static)
endif()
