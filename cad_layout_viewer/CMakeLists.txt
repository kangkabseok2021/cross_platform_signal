cmake_minimum_required(VERSION 3.21)
project(CadLayoutViewer CXX)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# ── Rust layout_parser ────────────────────────────────────────────────────────
find_program(CARGO cargo REQUIRED)

set(RUST_LIB_DIR  "${CMAKE_CURRENT_BINARY_DIR}/rust")
set(RUST_LIB_FILE "${RUST_LIB_DIR}/liblayout_parser.a")

add_custom_command(
    OUTPUT  "${RUST_LIB_FILE}"
    COMMAND ${CMAKE_COMMAND} -E make_directory "${RUST_LIB_DIR}"
    COMMAND ${CARGO} build --release
            --manifest-path "${CMAKE_CURRENT_SOURCE_DIR}/layout_parser/Cargo.toml"
            --target-dir    "${RUST_LIB_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy
            "${RUST_LIB_DIR}/release/liblayout_parser.a" "${RUST_LIB_FILE}"
    DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/layout_parser/src/lib.rs"
            "${CMAKE_CURRENT_SOURCE_DIR}/layout_parser/Cargo.toml"
    COMMENT "Building Rust layout_parser"
    VERBATIM
)
add_custom_target(rust_layout_parser DEPENDS "${RUST_LIB_FILE}")
add_library(layout_parser_lib STATIC IMPORTED GLOBAL)
add_dependencies(layout_parser_lib rust_layout_parser)
set_target_properties(layout_parser_lib PROPERTIES
    IMPORTED_LOCATION "${RUST_LIB_FILE}")

# ── Eigen (FDM solver) ────────────────────────────────────────────────────────
include(FetchContent)
if(NOT TARGET Eigen3::Eigen)
    FetchContent_Declare(eigen
        GIT_REPOSITORY https://gitlab.com/libeigen/eigen.git
        GIT_TAG        3.4.0
        GIT_SHALLOW    TRUE
    )
    set(BUILD_TESTING       OFF CACHE BOOL "" FORCE)
    set(EIGEN_BUILD_TESTING OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(eigen)
endif()

# ── Qt 6 ─────────────────────────────────────────────────────────────────────
find_package(Qt6 REQUIRED COMPONENTS Widgets Test)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)

# ── Core library (no main, used by app + tests) ───────────────────────────────
add_library(cad_core STATIC
    src/layout_engine.cpp
    src/layout_canvas.cpp
    src/heat_solver.cpp
    src/heatmap_renderer.cpp
    src/hotspot_analyser.cpp
    src/tutorial_wizard.cpp
    src/main_window.cpp
)
target_include_directories(cad_core PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
)
target_link_libraries(cad_core PUBLIC
    layout_parser_lib
    Eigen3::Eigen
    Qt6::Widgets
)
if(WIN32)
    target_link_libraries(cad_core PUBLIC ws2_32 userenv ntdll)
else()
    target_link_libraries(cad_core PUBLIC pthread dl)
endif()
target_compile_options(cad_core PRIVATE -Wall -Wextra)

# ── Main executable ───────────────────────────────────────────────────────────
qt_add_executable(cad_layout_viewer src/main.cpp)
target_link_libraries(cad_layout_viewer PRIVATE cad_core)

# ── GoogleTest (LayoutEngine — no Qt window needed) ───────────────────────────
if(NOT TARGET GTest::gtest_main)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
endif()

enable_testing()
add_executable(test_layout_engine tests/test_layout_engine.cpp)
target_include_directories(test_layout_engine PRIVATE include)
target_link_libraries(test_layout_engine PRIVATE
    cad_core GTest::gtest Qt6::Core)
target_compile_definitions(test_layout_engine PRIVATE
    TEST_DATA_DIR="${CMAKE_CURRENT_SOURCE_DIR}/sample_layout")
add_test(NAME LayoutEngineTests COMMAND test_layout_engine)

# ── QTest (HeatSolver) ────────────────────────────────────────────────────────
add_executable(test_heat_solver tests/test_heat_solver.cpp)
target_include_directories(test_heat_solver PRIVATE include)
target_link_libraries(test_heat_solver PRIVATE
    cad_core Qt6::Test)
add_test(NAME HeatSolverTests
    COMMAND test_heat_solver
    WORKING_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}")
set_tests_properties(HeatSolverTests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
