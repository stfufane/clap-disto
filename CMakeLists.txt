cmake_minimum_required(VERSION 3.27)

project(Disstortion VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option(COPY_AFTER_BUILD "Copy the clap to ~/Library on MACOS, ~/.clap on linux" FALSE)

include(cmake/CPM.cmake)

CPMAddPackage("gh:free-audio/clap#main")
CPMAddPackage("gh:free-audio/clap-helpers#main")
CPMAddPackage("gh:Tracktion/choc#main")
CPMAddPackage("gh:cameron314/readerwriterqueue#master")
CPMAddPackage("gh:VitalAudio/visage#main")
CPMAddPackage("gh:nlohmann/json@3.11.3")

set(SOURCE_FILES
    src/disstortion.cpp
    src/disstortion-pluginentry.cpp
    src/dsp/CubicDrive.cpp)

add_library(${PROJECT_NAME} MODULE
        ${SOURCE_FILES})
set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".clap" PREFIX "")
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")

target_link_libraries(${PROJECT_NAME} PRIVATE
    clap
    clap-helpers
    visage
    choc
    readerwriterqueue
    nlohmann_json)