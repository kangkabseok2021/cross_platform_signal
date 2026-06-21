cmake_minimum_required(VERSION 3.21)
project(MagnetometerSurveyVisualizer CXX)

add_subdirectory(lib/survey_processor)

option(BUILD_SENSYS_APP "Build Qt6 Magnetometer Survey Visualizer App" OFF)
if(BUILD_SENSYS_APP)
    add_subdirectory(app)
endif()
