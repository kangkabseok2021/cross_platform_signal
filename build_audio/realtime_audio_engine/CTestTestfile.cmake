# CMake generated Testfile for 
# Source directory: /Users/kab/Projects/Portfolio/cross_platform_signal/realtime_audio_engine
# Build directory: /Users/kab/Projects/Portfolio/cross_platform_signal/build_audio/realtime_audio_engine
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("/Users/kab/Projects/Portfolio/cross_platform_signal/build_audio/realtime_audio_engine/test_audio_core[1]_include.cmake")
add_test([=[test_audio_qt]=] "/Users/kab/Projects/Portfolio/cross_platform_signal/build_audio/realtime_audio_engine/test_audio_qt")
set_tests_properties([=[test_audio_qt]=] PROPERTIES  ENVIRONMENT "QT_QPA_PLATFORM=offscreen" _BACKTRACE_TRIPLES "/Users/kab/Projects/Portfolio/cross_platform_signal/realtime_audio_engine/CMakeLists.txt;63;add_test;/Users/kab/Projects/Portfolio/cross_platform_signal/realtime_audio_engine/CMakeLists.txt;0;")
