cmake_minimum_required(VERSION 3.21)
project(SurveyProcessor CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

if(NOT TARGET GTest::gtest_main)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()
include(GoogleTest)
enable_testing()

add_library(survey_processor STATIC
    src/GradiometerSimulator.cpp
    src/GaussianSmoother.cpp
    src/BaselineSubtractor.cpp
    src/RmsCalculator.cpp
    src/AnomalyDetector.cpp
    src/AnomalyExporter.cpp
)
target_include_directories(survey_processor PUBLIC include)
target_compile_features(survey_processor PUBLIC cxx_std_20)
target_link_libraries(survey_processor PUBLIC Threads::Threads)
if(MSVC)
    target_compile_options(survey_processor PRIVATE /W3)
else()
    target_compile_options(survey_processor PRIVATE -Wall -Wextra)
endif()

add_executable(test_survey_processor tests/test_survey_processor.cpp)
target_link_libraries(test_survey_processor PRIVATE survey_processor GTest::gtest_main)
target_compile_features(test_survey_processor PRIVATE cxx_std_20)
if(NOT MSVC)
    target_compile_options(test_survey_processor PRIVATE -Wall -Wextra)
endif()
gtest_discover_tests(test_survey_processor)
