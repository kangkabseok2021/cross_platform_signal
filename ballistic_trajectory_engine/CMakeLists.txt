cmake_minimum_required(VERSION 3.21)
project(BallisticTrajectoryEngine CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(MSVC)
    add_compile_definitions(_USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS)
endif()

# ── Ballistic engine (SHARED — also produces the DLL used by P/Invoke) ───────
add_library(ballistic_engine SHARED
    src/rk4.cpp
    src/engine.cpp
    src/c_api.cpp
)
target_include_directories(ballistic_engine
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
)
target_compile_definitions(ballistic_engine PRIVATE BALLISTIC_ENGINE_EXPORTS)
if(MSVC)
    target_compile_options(ballistic_engine PRIVATE /W3)
else()
    target_compile_options(ballistic_engine PRIVATE -Wall -Wextra)
endif()

# CMake install rules (used by conanfile.py package())
include(GNUInstallDirs)
install(TARGETS ballistic_engine
    EXPORT BallisticEngineTargets
    ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR})
install(DIRECTORY include/ DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
install(EXPORT BallisticEngineTargets
    FILE BallisticEngineConfig.cmake
    NAMESPACE ballistic::
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/ballistic-engine)

# ── GoogleTest (reuse parent's if available) ──────────────────────────────────
if(NOT TARGET GTest::gtest_main)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

# ── Unit tests ────────────────────────────────────────────────────────────────
enable_testing()
add_executable(ballistic_tests
    tests/test_rk4.cpp
    tests/test_engine.cpp
)
target_include_directories(ballistic_tests PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(ballistic_tests PRIVATE ballistic_engine GTest::gtest_main)
if(MSVC)
    target_compile_options(ballistic_tests PRIVATE /W3)
else()
    target_compile_options(ballistic_tests PRIVATE -Wall -Wextra)
endif()

include(GoogleTest)
gtest_discover_tests(ballistic_tests)
