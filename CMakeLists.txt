cmake_minimum_required(VERSION 3.27)

project(Disstortion
        VERSION 0.1.1
        LANGUAGES C CXX
        DESCRIPTION "An amateur distortion plugin in CLAP !")

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

if(WIN32)
    # This enables the use of the __VA_OPTS__ macro definition
    string(APPEND CMAKE_CXX_FLAGS " /Zc:preprocessor")
endif()

if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "Build for 11.0")
    option(USE_SANITIZER "Build and link with ASAN" FALSE)
endif()

include(cmake/get_cpm.cmake)
include(cmake/utils.cmake)

CPMAddPackage("gh:free-audio/clap#main")
CPMAddPackage("gh:free-audio/clap-helpers#main")
CPMAddPackage("gh:cameron314/readerwriterqueue#master")
CPMAddPackage("gh:VitalAudio/visage#main")
CPMAddPackage("gh:nlohmann/json@3.11.3")
CPMAddPackage("gh:gabime/spdlog@1.16.0")

set(DSP_FILES
        src/dsp/BiquadFilter.h
        src/dsp/MultiDisto.cpp
        src/dsp/MultiDisto.h
        src/dsp/OverSampler.cpp
        src/dsp/OverSampler.h
        src/dsp/SmoothedValue.h)

set(GUI_FILES
        src/gui/DisstortionEditor.h
        src/gui/DisstortionEditor.cpp
        src/gui/RotaryKnob.cpp
        src/gui/RotaryKnob.h
        src/gui/ToggleButton.cpp
        src/gui/ToggleButton.h
        src/gui/IParamControl.cpp
        src/gui/IParamControl.h
        src/gui/FilterPanel.cpp
        src/gui/FilterPanel.h
        src/gui/DriveSelector.cpp
        src/gui/DriveSelector.h
        src/gui/FilterSelector.cpp
        src/gui/FilterSelector.h
)

set(PARAM_FILES
        src/params/Parameter.cpp
        src/params/Parameters.cpp
        src/params/IParameterListener.cpp
        src/params/IParameterListener.h
        src/params/ValueMapping.cpp
        src/params/ValueMapping.h
)

set(PRESET_FILES
        src/presets/PresetManager.cpp
        src/presets/PresetManager.h
)

set(UTILS_FILES
        src/utils/Logger.h
        src/utils/Utils.h
        src/utils/Folders.h
        src/utils/Folders.cpp
)

set(SOURCE_FILES
        src/disstortion.h
        src/disstortion.cpp
        src/disstortion-pluginentry.cpp
        ${DSP_FILES}
        ${GUI_FILES}
        ${PARAM_FILES}
        ${PRESET_FILES}
        ${UTILS_FILES}
)

file(GLOB_RECURSE FONT_TTF_FILES fonts/*.ttf)
file(GLOB_RECURSE IMG_FILES img/*.png img/*.svg)
add_embedded_resources(font_resources "disto_fonts.h" "resources::fonts" "${FONT_TTF_FILES}")
add_embedded_resources(img_resources "disto_images.h" "resources::images" "${IMG_FILES}")

file(GLOB_RECURSE SHADER_FILES shaders/[vf]s_*.sc)
add_compile_definitions(SHADERS_FOLDER=\"${CMAKE_CURRENT_SOURCE_DIR}/shaders\")
visage_embed_shaders(shader_resources "disto_shaders.h" "resources::shaders" "${SHADER_FILES}")

# TODO: check https://github.com/baconpaul/six-sines/blob/main/CMakeLists.txt for standalone

add_library(${PROJECT_NAME} MODULE ${SOURCE_FILES})
target_include_directories(${PROJECT_NAME} PUBLIC src)
target_link_libraries(${PROJECT_NAME} PRIVATE
        clap
        clap-helpers
        visage
        readerwriterqueue
        nlohmann_json
        font_resources
        img_resources
        shader_resources
        debug spdlog::spdlog)
target_compile_definitions(${PROJECT_NAME} PRIVATE
        PROJECT_VERSION="${PROJECT_VERSION}"
        USERDATA_DIR="${plugin_folder}"
        PLUGIN_ID_SUFFIX=$<IF:$<CONFIG:Debug>,".debug","">
        $<$<CONFIG:Debug>:DEBUG>)

if (APPLE)
    # Calls all the mac specific bundle and sanitizer stuff
    include(cmake/mac/utils.cmake)
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".clap" PREFIX "")
endif()

option(COPY_AFTER_BUILD "Copy the clap to ~/Library on MACOS, ~/.clap on linux" TRUE)
if (${COPY_AFTER_BUILD})
    copy_after_build(${PROJECT_NAME})
endif()