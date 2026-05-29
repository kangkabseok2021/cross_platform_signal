cmake_minimum_required(VERSION 3.21)
project(EdgeSortingPipeline CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(Threads REQUIRED)

option(BUILD_WITH_TFLITE "Compile with TensorFlow Lite support" OFF)

set(SORTING_SOURCES
    src/postproc/NMS.cpp
    src/inference/CameraSimulator.cpp
    src/inference/TFLiteEngine.cpp
    src/control/MachineController.cpp
    src/telemetry/TelemetrySender.cpp
)

# ── sorting_core ─────────────────────────────────────────────────────────────
add_library(sorting_core STATIC ${SORTING_SOURCES})
target_include_directories(sorting_core PUBLIC include)
target_link_libraries(sorting_core PUBLIC Threads::Threads)

if(BUILD_WITH_TFLITE)
    target_compile_definitions(sorting_core PUBLIC BUILD_WITH_TFLITE)
    include(cmake/FetchTFLite.cmake OPTIONAL)
    # TFLite link target would be linked here if FetchTFLite is defined.
endif()

if(MSVC)
    target_compile_options(sorting_core PRIVATE /W3)
else()
    target_compile_options(sorting_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── edge_sorting_app ──────────────────────────────────────────────────────────
add_executable(edge_sorting_app src/main.cpp)
target_link_libraries(edge_sorting_app PRIVATE sorting_core)

# ── test_sorting ─────────────────────────────────────────────────────────────
add_executable(test_sorting
    tests/test_nms.cpp
    tests/test_queue.cpp
    tests/test_controller.cpp
    tests/test_telemetry.cpp
    tests/test_pipeline.cpp
)
target_link_libraries(test_sorting PRIVATE sorting_core GTest::gtest_main GTest::gtest)
target_include_directories(test_sorting BEFORE PRIVATE 
    "${googletest_SOURCE_DIR}/googletest/include"
    "${googletest_SOURCE_DIR}/googlemock/include"
)
include(GoogleTest)
gtest_discover_tests(test_sorting)
