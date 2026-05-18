cmake_minimum_required(VERSION 3.21)
project(CrossPlatformSignalAnalyzer C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_GUI "Build Qt 6 dashboard" OFF)

find_package(Threads REQUIRED)

include(FetchContent)
FetchContent_Declare(googletest
  URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# SQLite amalgamation — bundled for zero external dependency on all platforms.
# third_party/sqlite3.c + sqlite3.h are checked in; no system SQLite needed.
add_library(sqlite3_bundled STATIC third_party/sqlite3.c)
target_include_directories(sqlite3_bundled PUBLIC third_party)
if(UNIX)
  target_link_libraries(sqlite3_bundled PUBLIC dl)
endif()

# Core signal processing library
add_library(signal_core STATIC
  src/FftAnalyser.cpp
  src/ButterworthFilter.cpp
  src/SqliteLogger.cpp
  src/SignalProcessor.cpp
)
target_include_directories(signal_core PUBLIC src third_party)
target_link_libraries(signal_core PUBLIC Threads::Threads sqlite3_bundled)
if(MSVC)
  target_compile_options(signal_core PRIVATE /W3)
else()
  target_compile_options(signal_core PRIVATE -Wall -Wextra)
endif()

# CLI runner
add_executable(signal_analyzer src/main.cpp)
target_link_libraries(signal_analyzer PRIVATE signal_core)

# Qt 6 dashboard (optional — requires Qt installed)
if(BUILD_GUI)
  find_package(Qt6 REQUIRED COMPONENTS Widgets Charts)
  add_executable(signal_dashboard dashboard/main_window.cpp)
  target_link_libraries(signal_dashboard PRIVATE signal_core Qt6::Widgets Qt6::Charts)
  set_target_properties(signal_dashboard PROPERTIES AUTOMOC ON AUTOUIC ON)
endif()

# Tests
enable_testing()
add_executable(test_signal
  tests/test_fft.cpp
  tests/test_butterworth.cpp
  tests/test_sources.cpp
)
target_link_libraries(test_signal PRIVATE signal_core GTest::gtest_main)
include(GoogleTest)
gtest_discover_tests(test_signal)
