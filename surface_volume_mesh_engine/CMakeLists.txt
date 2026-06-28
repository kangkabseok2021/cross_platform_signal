cmake_minimum_required(VERSION 3.21)
project(SurfaceVolumeMeshEngine CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# GoogleTest
include(FetchContent)
if(NOT TARGET GTest::gtest_main)
  FetchContent_Declare(googletest
    URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
  set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endif()
include(GoogleTest)

# pybind11
FetchContent_Declare(pybind11
  URL https://github.com/pybind/pybind11/archive/refs/tags/v2.13.6.zip)
FetchContent_MakeAvailable(pybind11)

# Core static library
add_library(mesh_engine_lib STATIC
  src/AdvancingFront2D.cpp
  src/DelaunayMesh3D.cpp
  src/MeshQuality.cpp
  src/LaplacianSmoother.cpp
  src/MeshExporter.cpp
)
target_include_directories(mesh_engine_lib PUBLIC include)
target_compile_features(mesh_engine_lib PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(mesh_engine_lib PRIVATE /W3)
else()
  target_compile_options(mesh_engine_lib PRIVATE -Wall -Wextra)
endif()

# Test executable
add_executable(test_mesh_engine tests/test_mesh_engine.cpp)
target_link_libraries(test_mesh_engine PRIVATE mesh_engine_lib GTest::gtest_main)
gtest_discover_tests(test_mesh_engine)

# pybind11 Python module
find_package(Python3 REQUIRED COMPONENTS Interpreter Development)
pybind11_add_module(mesh_engine python/bindings.cpp)
target_link_libraries(mesh_engine PRIVATE mesh_engine_lib)
set_target_properties(mesh_engine PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/python)
