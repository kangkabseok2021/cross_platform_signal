cmake_minimum_required(VERSION 3.21)
project(SurveyVisualizerApp CXX)

if(NOT TARGET survey_processor)
    message(FATAL_ERROR "survey_processor target required. Build via magnetometer_survey_visualizer/CMakeLists.txt")
endif()

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Sql Quick
             3DCore 3DExtras 3DRender)

qt_add_executable(magnetometer_survey_app
    src/main.cpp
    src/SurveyRepository.cpp
    src/SurveyController.cpp
    src/ScanLineProducer.cpp
    src/SurveyGridGeometry.cpp
)
set_target_properties(magnetometer_survey_app PROPERTIES
    AUTOMOC ON
    WIN32_EXECUTABLE TRUE
    MACOSX_BUNDLE TRUE)
qt_add_qml_module(magnetometer_survey_app
    URI SensysApp
    VERSION 1.0
    QML_FILES qml/main.qml)
target_include_directories(magnetometer_survey_app PRIVATE include
    "${CMAKE_CURRENT_SOURCE_DIR}/../lib/survey_processor/include")
target_link_libraries(magnetometer_survey_app PRIVATE
    survey_processor
    Qt6::Core Qt6::Gui Qt6::Widgets Qt6::Sql Qt6::Quick
    Qt6::3DCore Qt6::3DExtras Qt6::3DRender)
target_compile_features(magnetometer_survey_app PRIVATE cxx_std_20)
if(NOT MSVC)
    target_compile_options(magnetometer_survey_app PRIVATE -Wall -Wextra)
endif()
