cmake_minimum_required(VERSION 3.21)
project(cv_ws_telemetry CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

find_package(OpenCV REQUIRED COMPONENTS core imgproc videoio)
find_package(Boost   REQUIRED COMPONENTS system)
find_package(nlohmann_json REQUIRED)

# ── image_processor ──────────────────────────────────────────────────────────
add_library(image_processor STATIC src/ImageProcessor.cpp)
target_include_directories(image_processor PUBLIC include)
target_link_libraries(image_processor PUBLIC opencv_core opencv_imgproc)
if(NOT MSVC)
    target_compile_options(image_processor PRIVATE -Wall -Wextra)
endif()

# ── GoogleTest ────────────────────────────────────────────────────────────────
include(FetchContent)
if(NOT TARGET GTest::gtest_main)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP ON)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()
include(CTest)
enable_testing()
include(GoogleTest)

# ── test_image_processor ──────────────────────────────────────────────────────
add_executable(test_image_processor tests/test_image_processor.cpp)
target_link_libraries(test_image_processor PRIVATE image_processor opencv_imgproc GTest::gtest_main)
gtest_discover_tests(test_image_processor)

# ── serializer ────────────────────────────────────────────────────────────────
add_library(serializer STATIC src/Serializer.cpp)
target_include_directories(serializer PUBLIC include)
target_link_libraries(serializer PUBLIC nlohmann_json::nlohmann_json)
if(NOT MSVC)
    target_compile_options(serializer PRIVATE -Wall -Wextra)
endif()

# ── test_serializer ───────────────────────────────────────────────────────
add_executable(test_serializer tests/test_serializer.cpp)
target_link_libraries(test_serializer PRIVATE serializer GTest::gtest_main)
gtest_discover_tests(test_serializer)
