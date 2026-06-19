cmake_minimum_required(VERSION 3.21)
project(AutomotiveANCEngine CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Header-only library
add_library(anc_engine INTERFACE)
target_include_directories(anc_engine INTERFACE include)

# GTest — fetch standalone so this subdirectory builds without the root CMakeLists
include(FetchContent)
if(NOT TARGET GTest::gtest_main)
  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()
include(GoogleTest)

# Test executables
foreach(suite fxlms imc hybrid_anc fault parity regression)
  add_executable(test_anc_${suite} tests/test_${suite}.cpp)
  target_link_libraries(test_anc_${suite} PRIVATE anc_engine GTest::gtest_main)
  if(MSVC)
    target_compile_options(test_anc_${suite} PRIVATE /W3)
  else()
    target_compile_options(test_anc_${suite} PRIVATE -Wall -Wextra)
  endif()
  gtest_discover_tests(test_anc_${suite})
endforeach()
