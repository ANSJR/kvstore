cmake_minimum_required(VERSION 3.20)
project(kvstore LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default
if(NOT CMAKE_BUILD_TYPE)
  set(CMAKE_BUILD_TYPE Debug)
endif()

# ThreadSanitizer switch.
option(KV_TSAN "Build with ThreadSanitizer" OFF)
if(KV_TSAN)
  add_compile_options(-fsanitize=thread -g -O1)
  add_link_options(-fsanitize=thread)
endif()

# Core library: shared by the server and the tests
add_library(kvcore src/Store.cpp)
target_include_directories(kvcore PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(kvcore PRIVATE -Wall -Wextra -Wpedantic)

# Server
add_executable(kvstore src/main.cpp)
target_link_libraries(kvstore PRIVATE kvcore)
target_compile_options(kvstore PRIVATE -Wall -Wextra -Wpedantic)

# Tests
include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.15.2.tar.gz
  DOWNLOAD_EXTRACT_TIMESTAMP TRUE
)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(kvTests tests/StoreTest.cpp)
target_link_libraries(kvTests PRIVATE kvcore GTest::gtest_main)
target_compile_options(kvTests PRIVATE -Wall -Wextra -Wpedantic)

include(GoogleTest)
gtest_discover_tests(kvTests)