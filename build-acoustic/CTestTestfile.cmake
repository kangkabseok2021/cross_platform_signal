# CMake generated Testfile for 
# Source directory: /Users/kab/Projects/Portfolio/cross_platform_signal
# Build directory: /Users/kab/Projects/Portfolio/cross_platform_signal/build-acoustic
# 
# This file includes the relevant testing commands required for 
# testing this directory and lists subdirectories to be tested as well.
include("/Users/kab/Projects/Portfolio/cross_platform_signal/build-acoustic/test_signal[1]_include.cmake")
include("/Users/kab/Projects/Portfolio/cross_platform_signal/build-acoustic/test_optical[1]_include.cmake")
include("/Users/kab/Projects/Portfolio/cross_platform_signal/build-acoustic/test_ct[1]_include.cmake")
subdirs("_deps/googletest-build")
subdirs("acoustic_propagation_engine")
