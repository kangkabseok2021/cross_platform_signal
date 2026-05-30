# ─── audio_core — C++20 threading + DSP, no Qt ───────────────────────────────
add_library(audio_core STATIC
    src/AudioEngine.cpp
)
target_include_directories(audio_core PUBLIC include src)
target_compile_features(audio_core PUBLIC cxx_std_20)
target_link_libraries(audio_core PUBLIC Threads::Threads)
if(MSVC)
    target_compile_options(audio_core PRIVATE /W3)
else()
    target_compile_options(audio_core PRIVATE -Wall -Wextra)
endif()

# ─── test_audio_core — 13 GoogleTests, no Qt ─────────────────────────────────
add_executable(test_audio_core tests/test_audio_core.cpp)
target_link_libraries(test_audio_core PRIVATE audio_core GTest::gtest_main)
target_compile_features(test_audio_core PRIVATE cxx_std_20)
gtest_discover_tests(test_audio_core)

# ─── Qt-dependent targets ─────────────────────────────────────────────────────
find_package(Qt6 QUIET COMPONENTS Core Sql Quick)
if(Qt6_FOUND)
    # Wrapper library: AudioEngineWrapper + PresetRepository + PresetModel
    add_library(audio_qt_lib STATIC
        src/AudioEngineWrapper.cpp
        src/PresetRepository.cpp
        src/PresetModel.cpp
    )
    target_include_directories(audio_qt_lib PUBLIC include src)
    target_compile_features(audio_qt_lib PUBLIC cxx_std_20)
    target_link_libraries(audio_qt_lib PUBLIC audio_core Qt6::Core Qt6::Sql)
    set_target_properties(audio_qt_lib PROPERTIES AUTOMOC ON)
    if(MSVC)
        target_compile_options(audio_qt_lib PRIVATE /W3)
    else()
        target_compile_options(audio_qt_lib PRIVATE -Wall -Wextra)
    endif()

    # Desktop application
    qt_add_executable(audio_app src/main.cpp)
    set_target_properties(audio_app PROPERTIES AUTOMOC ON)
    qt_add_qml_module(audio_app
        URI AudioEngine
        VERSION 1.0
        QML_FILES
            qml/main.qml
            qml/RotaryKnob.qml
            qml/WaveformView.qml
    )
    target_link_libraries(audio_app PRIVATE audio_qt_lib Qt6::Quick)
    set_target_properties(audio_app PROPERTIES
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE   TRUE
    )

    # QTest executable — requires Qt6::Test
    find_package(Qt6 QUIET COMPONENTS Test)
    if(Qt6Test_FOUND)
        add_executable(test_audio_qt tests/test_audio_qt.cpp)
        target_link_libraries(test_audio_qt PRIVATE audio_qt_lib Qt6::Test)
        target_compile_features(test_audio_qt PRIVATE cxx_std_20)
        set_target_properties(test_audio_qt PROPERTIES AUTOMOC ON)
        add_test(NAME test_audio_qt COMMAND test_audio_qt)
        set_tests_properties(test_audio_qt PROPERTIES
            ENVIRONMENT "QT_QPA_PLATFORM=offscreen")
    endif()
endif()
