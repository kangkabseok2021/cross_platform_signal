# Conan 2 Packaging Guide

## Prerequisites

```bash
pip install conan>=2.0
conan profile detect   # creates a default profile matching your system
```

## Local Build (no Conan)

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build --parallel
ctest --test-dir build --output-on-failure -V
```

## Build and Package with Conan

```bash
# From ballistic_trajectory_engine/
conan create . --build=missing
```

This runs the full lifecycle: configure → build → install → test_package.

## Consuming the Package

After `conan create`, downstream projects can consume it:

```ini
# conanfile.txt
[requires]
ballistic-engine/1.0.0

[generators]
CMakeToolchain
CMakeDeps
```

```bash
conan install . --output-folder=build --build=missing
cmake --preset conan-release
cmake --build build-conan-release
```

In CMakeLists.txt:
```cmake
find_package(ballistic-engine REQUIRED)
target_link_libraries(my_app PRIVATE ballistic::engine)
```

## Conan 1 vs Conan 2 API

| Aspect | Conan 1 | Conan 2 |
|--------|---------|---------|
| Import | `from conans import ConanFile` | `from conan import ConanFile` |
| CMake generators | `cmake_find_package` | `CMakeDeps` + `CMakeToolchain` |
| Layout | manual | `cmake_layout(self)` |
| Install | `copy()` | `cmake.install()` |

The `conanfile.py` in this project uses the **Conan 2** API exclusively.
`exports_sources` ensures the source is bundled in the package recipe, enabling
`conan create` to build from the recipe alone without a VCS checkout.
