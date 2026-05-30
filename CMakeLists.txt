cmake_minimum_required(VERSION 3.21)
project(CrossPlatformSignalAnalyzer C CXX)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

option(BUILD_GUI "Build Qt 6 dashboard" OFF)

# MSVC: define M_PI and friends from <cmath>
if(MSVC)
  add_compile_definitions(_USE_MATH_DEFINES _CRT_SECURE_NO_WARNINGS)
endif()

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

# Optical positioning core (HAL + toolpath + FSM + telemetry)
add_library(optical_core STATIC
  src/hal/SpindleMotor.cpp
  src/hal/LinearAxis.cpp
  src/hal/PressureSensor.cpp
  src/hal/SafetyInterlock.cpp
  src/optical/ToolpathGenerator.cpp
  src/optical/PositioningFsm.cpp
  src/optical/TelemetryLogger.cpp
)
target_include_directories(optical_core PUBLIC src)
target_link_libraries(optical_core PUBLIC Threads::Threads)
if(MSVC)
  target_compile_options(optical_core PRIVATE /W3)
else()
  target_compile_options(optical_core PRIVATE -Wall -Wextra)
endif()

# CT Reconstruction core — C++20 static library (no Qt dependency)
add_library(ct_core STATIC
  ct_core/src/FilterBank.cpp
  ct_core/src/Reconstructor.cpp
)
target_include_directories(ct_core PUBLIC ct_core/include)
target_compile_features(ct_core PUBLIC cxx_std_20)
if(MSVC)
  target_compile_options(ct_core PRIVATE /W3)
  target_link_libraries(ct_core PRIVATE)
else()
  target_compile_options(ct_core PRIVATE -Wall -Wextra)
  find_package(TBB QUIET)
  if(TBB_FOUND)
    target_link_libraries(ct_core PRIVATE TBB::tbb)
  endif()
endif()

# CT QML viewer (optional — requires Qt 6 with Quick + Concurrent)
option(BUILD_CT_VIEWER "Build CT QML viewer" OFF)
if(BUILD_CT_VIEWER)
  find_package(Qt6 REQUIRED COMPONENTS Quick Concurrent)
  qt_add_executable(ct_viewer
    ct_viewer/main.cpp
    ct_viewer/CtImageProvider.cpp
    ct_viewer/ReconstructionController.cpp
  )
  qt_add_qml_module(ct_viewer
    URI CtViewer
    VERSION 1.0
    QML_FILES ct_viewer/qml/main.qml ct_viewer/qml/FilterControls.qml
  )
  target_link_libraries(ct_viewer PRIVATE ct_core Qt6::Quick Qt6::Concurrent)
  set_target_properties(ct_viewer PROPERTIES
    AUTOMOC ON
    WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE
  )
endif()

option(BUILD_ELECTROSTATIC "Build Electrostatic Monitor QML dashboard" OFF)
if(BUILD_ELECTROSTATIC)
  find_package(Qt6 REQUIRED COMPONENTS Core Network Quick Charts)

  # Electrostatic Monitor
  add_library(monitor_lib STATIC
    electrostatic_monitor/src/AdcSimulator.cpp
    electrostatic_monitor/src/ElectrostaticEngine.cpp
    electrostatic_monitor/src/TelemetryClient.cpp
    electrostatic_monitor/src/MonitorContext.cpp
  )
  target_include_directories(monitor_lib PUBLIC electrostatic_monitor/src)
  target_link_libraries(monitor_lib PUBLIC Qt6::Core Qt6::Network)
  set_target_properties(monitor_lib PROPERTIES AUTOMOC ON)
  if(MSVC)
    target_compile_options(monitor_lib PRIVATE /W3)
  else()
    target_compile_options(monitor_lib PRIVATE -Wall -Wextra)
  endif()

  qt_add_executable(electrostatic_app
    electrostatic_monitor/src/main.cpp
  )
  set_target_properties(electrostatic_app PROPERTIES AUTOMOC ON)
  qt_add_qml_module(electrostatic_app
    URI Eltex
    VERSION 1.0
    QML_FILES
      electrostatic_monitor/qml/main.qml
      electrostatic_monitor/qml/VoltageGauge.qml
      electrostatic_monitor/qml/DecayChart.qml
  )
  target_link_libraries(electrostatic_app PRIVATE monitor_lib Qt6::Quick Qt6::Charts)
  set_target_properties(electrostatic_app PROPERTIES
    WIN32_EXECUTABLE TRUE MACOSX_BUNDLE TRUE
  )
endif()
# Tests
enable_testing()
include(GoogleTest)

add_executable(test_signal
  tests/test_fft.cpp
  tests/test_butterworth.cpp
  tests/test_sources.cpp
)
target_link_libraries(test_signal PRIVATE signal_core GTest::gtest_main)
gtest_discover_tests(test_signal)

add_executable(test_optical
  tests/test_toolpath.cpp
  tests/test_positioning_fsm.cpp
  tests/test_hal.cpp
)
target_link_libraries(test_optical PRIVATE optical_core GTest::gtest_main)
gtest_discover_tests(test_optical)

add_executable(test_ct tests/test_ct.cpp)
target_link_libraries(test_ct PRIVATE ct_core GTest::gtest_main)
target_compile_features(test_ct PRIVATE cxx_std_20)
gtest_discover_tests(test_ct)

if(BUILD_ELECTROSTATIC)
  add_executable(test_electrostatic tests/test_electrostatic.cpp)
  target_link_libraries(test_electrostatic PRIVATE GTest::gtest_main GTest::gtest monitor_lib)
  gtest_discover_tests(test_electrostatic)
endif()

option(BUILD_DICOM "Build DICOM Anonymization & Observability Pipeline" OFF)
if(BUILD_DICOM)
  add_subdirectory(dicom_pipeline)
endif()

option(BUILD_SORTING "Build High-Speed Edge Inference Pipeline for Optical Sorting" OFF)
if(BUILD_SORTING)
  add_subdirectory(edge_sorting_pipeline)
endif()

option(BUILD_AUDIO "Build Real-Time Audio Processing Engine" OFF)
if(BUILD_AUDIO)
  add_subdirectory(realtime_audio_engine)
endif()

option(BUILD_TELEMETRY "Build Embedded Vehicle Telemetry Qt Dashboard" OFF)
if(BUILD_TELEMETRY)
  add_subdirectory(vehicle_telemetry_qt)
endif()

