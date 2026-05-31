# ── Legacy C99 acoustic library (intentional hazards) ────────────────────────
add_library(acoustic_legacy STATIC
    legacy/acoustic.c
)
target_include_directories(acoustic_legacy PUBLIC legacy)

# ── C++20 acoustic core library ───────────────────────────────────────────────
add_library(acoustic_core STATIC
    src/AcousticEngine.cpp
)
target_include_directories(acoustic_core PUBLIC include src)
target_compile_features(acoustic_core PUBLIC cxx_std_20)
target_link_libraries(acoustic_core PUBLIC Threads::Threads)
if(MSVC)
    target_compile_options(acoustic_core PRIVATE /W3)
else()
    target_compile_options(acoustic_core PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()

# ── 14 GoogleTests (no Qt dependency) ─────────────────────────────────────────
add_executable(test_acoustic_core
    tests/test_acoustic_core.cpp
)
target_include_directories(test_acoustic_core PRIVATE
    include src legacy
)
target_link_libraries(test_acoustic_core PRIVATE
    acoustic_core acoustic_legacy GTest::gtest_main
)
target_compile_features(test_acoustic_core PRIVATE cxx_std_20)
if(NOT MSVC)
    target_compile_options(test_acoustic_core PRIVATE -Wall -Wextra -Wno-unused-parameter)
endif()
gtest_discover_tests(test_acoustic_core)

# ── Legacy leak demo executables ──────────────────────────────────────────────
add_executable(demo_leak legacy/demo_leak.c)
target_link_libraries(demo_leak PRIVATE acoustic_legacy)
if(NOT MSVC)
    target_link_libraries(demo_leak PRIVATE m)
endif()

add_executable(demo_safe legacy/demo_safe.c)
target_link_libraries(demo_safe PRIVATE acoustic_legacy)
if(NOT MSVC)
    target_link_libraries(demo_safe PRIVATE m)
endif()

# ── Qt/QML heatmap (optional — skipped in CI) ─────────────────────────────────
find_package(Qt6 QUIET COMPONENTS Core Quick)
if(Qt6_FOUND)
    qt_add_executable(acoustic_app
        src/main_qt.cpp
        src/AcousticViewModel.cpp
    )
    set_target_properties(acoustic_app PROPERTIES AUTOMOC ON)
    qt_add_qml_module(acoustic_app
        URI AcousticEngine
        VERSION 1.0
        QML_FILES
            qml/main.qml
            qml/HeatmapView.qml
            qml/SourceMarker.qml
    )
    target_include_directories(acoustic_app PRIVATE include src)
    target_link_libraries(acoustic_app PRIVATE
        acoustic_core Qt6::Core Qt6::Quick
    )
    target_compile_features(acoustic_app PRIVATE cxx_std_20)
    set_target_properties(acoustic_app PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE   TRUE
    )
endif()
