find_package(Threads REQUIRED)

# ─── EMA core — pure header, no dependencies ──────────────────────────────────
add_library(vt_ema_core INTERFACE)
target_include_directories(vt_ema_core INTERFACE ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(vt_ema_core INTERFACE cxx_std_20)

# ─── Sensor simulator core ───────────────────────────────────────────────────
add_library(vt_sensor_core STATIC src/SensorSimulator.cpp)
target_include_directories(vt_sensor_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_features(vt_sensor_core PUBLIC cxx_std_20)
target_link_libraries(vt_sensor_core PUBLIC Threads::Threads)
if(MSVC)
    target_compile_options(vt_sensor_core PRIVATE /W3)
else()
    target_compile_options(vt_sensor_core PRIVATE -Wall -Wextra)
endif()

# ─── EMA GoogleTests — no Qt, no httplib ─────────────────────────────────────
# GTest::gtest_main is provided by parent CMakeLists; fallback for standalone builds.
if(NOT TARGET GTest::gtest_main)
    include(FetchContent)
    FetchContent_Declare(googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip)
    set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(googletest)
    enable_testing()
    include(GoogleTest)
endif()

add_executable(test_ema tests/test_ema.cpp)
target_link_libraries(test_ema PRIVATE vt_ema_core GTest::gtest_main)
target_compile_features(test_ema PRIVATE cxx_std_20)
if(NOT MSVC)
    target_compile_options(test_ema PRIVATE -Wall -Wextra)
endif()
gtest_discover_tests(test_ema)

# ─── HTTP server + headless binary (optional) ─────────────────────────────────
option(BUILD_VT_HTTP "Build vehicle telemetry HTTP server and headless binary" ON)
if(BUILD_VT_HTTP)
    include(FetchContent)

    FetchContent_Declare(cpp_httplib
        URL https://github.com/yhirose/cpp-httplib/archive/refs/tags/v0.16.0.tar.gz)
    FetchContent_MakeAvailable(cpp_httplib)

    FetchContent_Declare(nlohmann_json
        URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz)
    set(JSON_BuildTests OFF CACHE BOOL "" FORCE)
    set(JSON_Install     OFF CACHE BOOL "" FORCE)
    FetchContent_MakeAvailable(nlohmann_json)

    add_library(vt_http_core STATIC src/TelemetryHttpServer.cpp)
    target_include_directories(vt_http_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(vt_http_core PUBLIC
        vt_sensor_core
        vt_ema_core
        httplib::httplib
        nlohmann_json::nlohmann_json
        Threads::Threads)
    target_compile_features(vt_http_core PUBLIC cxx_std_20)
    if(NOT MSVC)
        target_compile_options(vt_http_core PRIVATE -Wall -Wextra)
    endif()

    # Headless executable (--no-qt mode, used by pytest)
    add_executable(vehicle_telemetry_headless src/main_noqt.cpp)
    target_link_libraries(vehicle_telemetry_headless PRIVATE vt_http_core)
    target_compile_features(vehicle_telemetry_headless PRIVATE cxx_std_20)
    if(NOT MSVC)
        target_compile_options(vehicle_telemetry_headless PRIVATE -Wall -Wextra)
    endif()
endif()

# ─── Qt/QML application (optional) ───────────────────────────────────────────
find_package(Qt6 QUIET COMPONENTS Core Quick Charts)
if(Qt6_FOUND AND BUILD_VT_HTTP)
    add_library(vt_qt_lib STATIC src/TelemetryModel.cpp)
    target_include_directories(vt_qt_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
    target_link_libraries(vt_qt_lib PUBLIC vt_http_core Qt6::Core)
    target_compile_features(vt_qt_lib PUBLIC cxx_std_20)
    set_target_properties(vt_qt_lib PROPERTIES AUTOMOC ON)
    if(NOT MSVC)
        target_compile_options(vt_qt_lib PRIVATE -Wall -Wextra)
    endif()

    qt_add_executable(vehicle_telemetry src/main.cpp)
    set_target_properties(vehicle_telemetry PROPERTIES
        AUTOMOC ON
        WIN32_EXECUTABLE TRUE
        MACOSX_BUNDLE TRUE)
    qt_add_qml_module(vehicle_telemetry
        URI VehicleTelemetry
        VERSION 1.0
        QML_FILES
            qml/main.qml
            qml/GaugeCard.qml
            qml/TelemetryChart.qml)
    target_link_libraries(vehicle_telemetry PRIVATE vt_qt_lib Qt6::Quick Qt6::Charts)
endif()
