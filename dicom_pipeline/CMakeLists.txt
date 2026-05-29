cmake_minimum_required(VERSION 3.21)
project(DicomAnonymizer CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find Package dependencies
find_package(DCMTK REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(Threads REQUIRED)

if(TARGET DCMTK::DCMTK)
  set(DCMTK_LINK_TARGET DCMTK::DCMTK)
else()
  set(DCMTK_LINK_TARGET ${DCMTK_LIBRARIES})
endif()

# FetchContent nlohmann_json & cpp-httplib
include(FetchContent)

FetchContent_Declare(
  nlohmann_json
  URL https://github.com/nlohmann/json/archive/refs/tags/v3.11.3.tar.gz
  URL_HASH SHA256=0d8ef5af7f9794e3263480193c491549b2ba6cc74bb018906202ada498a79406
)
FetchContent_MakeAvailable(nlohmann_json)

FetchContent_Declare(
  cpp-httplib
  URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.18.3.tar.gz
  URL_HASH SHA256=a0567bcd6c3fe5cef1b329b96245119047f876b49e06cc129a36a7a8dffe173e
)
FetchContent_MakeAvailable(cpp-httplib)

# ── dicom_core — Core anonymization & logging library ──────────
add_library(dicom_core STATIC
    src/DicomReader.cpp
    src/DicomAnonymizer.cpp
    src/ProcessingLogger.cpp
    src/MetricsServer.cpp
)
target_include_directories(dicom_core PUBLIC include ${DCMTK_INCLUDE_DIRS})
target_link_libraries(dicom_core PUBLIC 
    ${DCMTK_LINK_TARGET}
    OpenSSL::SSL
    OpenSSL::Crypto
    Threads::Threads
    nlohmann_json::nlohmann_json
    httplib::httplib
)

if(MSVC)
  target_compile_options(dicom_core PRIVATE /W3)
else()
  target_compile_options(dicom_core PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ── dicom_anonymizer — Command Line Interface ──────────
add_executable(dicom_anonymizer src/main.cpp)
target_link_libraries(dicom_anonymizer PRIVATE dicom_core)

# ── test_dicom — GoogleTest Executable ──────────
add_executable(test_dicom
    tests/test_dicom_reader.cpp
    tests/test_dicom_anonymizer.cpp
    tests/test_processing_logger.cpp
    tests/test_metrics_server.cpp
)
target_link_libraries(test_dicom PRIVATE dicom_core GTest::gtest_main GTest::gtest)
target_include_directories(test_dicom BEFORE PRIVATE 
    "${googletest_SOURCE_DIR}/googletest/include"
    "${googletest_SOURCE_DIR}/googlemock/include"
)
target_compile_definitions(test_dicom PRIVATE TEST_FIXTURE_DIR="${CMAKE_CURRENT_SOURCE_DIR}/tests/fixtures")
include(GoogleTest)
gtest_discover_tests(test_dicom)



