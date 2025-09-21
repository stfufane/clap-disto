cmake_minimum_required(VERSION 3.27)

project(Disstortion VERSION 0.1.1 LANGUAGES C CXX)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_CXX_STANDARD 20)
if (APPLE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET 11.0 CACHE STRING "Build for 11.0")
    option(USE_SANITIZER "Build and link with ASAN" FALSE)
    option(COPY_AFTER_BUILD "Copy the clap to ~/Library on MACOS, ~/.clap on linux" FALSE)
endif()

set(CMAKE_EXPORT_COMPILE_COMMANDS TRUE)

option(COPY_AFTER_BUILD "Copy the clap to ~/Library on MACOS, ~/.clap on linux" FALSE)

include(cmake/get_cpm.cmake)

CPMAddPackage("gh:free-audio/clap#main")
CPMAddPackage("gh:free-audio/clap-helpers#main")
CPMAddPackage("gh:cameron314/readerwriterqueue#master")
CPMAddPackage("gh:VitalAudio/visage#main")
CPMAddPackage("gh:nlohmann/json@3.11.3")

set(SOURCE_FILES
    src/disstortion.h
    src/disstortion.cpp
    src/disstortion-pluginentry.cpp
    src/helpers/IParamControl.cpp
    src/params/Parameter.cpp
    src/params/Parameters.cpp
    src/gui/DisstortionEditor.h
    src/gui/DisstortionEditor.cpp
    src/gui/RotaryKnob.cpp
    src/dsp/BiquadFilter.cpp
    src/dsp/BiquadFilter.h
    src/dsp/MultiDisto.cpp
    src/dsp/MultiDisto.h
    src/dsp/OverSampler.cpp
    src/dsp/OverSampler.h)

file(GLOB_RECURSE FONT_TTF_FILES fonts/*.ttf)
add_embedded_resources(font_resources "disto_fonts.h" "resources::fonts" "${FONT_TTF_FILES}")

add_library(${PROJECT_NAME} MODULE ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME} PRIVATE
        clap
        clap-helpers
        visage
        readerwriterqueue
        nlohmann_json
        font_resources)
target_compile_definitions(${PROJECT_NAME} PRIVATE PROJECT_VERSION="${PROJECT_VERSION}")

if(APPLE)
    set_target_properties(${PROJECT_NAME} PROPERTIES
            BUNDLE True
            BUNDLE_EXTENSION clap
            MACOSX_BUNDLE_GUI_IDENTIFIER dev.stephanealbanese.${PROJECT_NAME}
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_SOURCE_DIR}/cmake/${PROJECT_NAME}.plist.in
    )

    target_compile_options(${PROJECT_NAME} PRIVATE
            -Werror
            $<$<BOOL:${USE_SANITIZER}>:-fsanitize=address>
            $<$<BOOL:${USE_SANITIZER}>:-fsanitize=undefined>
    )
    target_link_options(${PROJECT_NAME} PRIVATE
            $<$<BOOL:${USE_SANITIZER}>:-fsanitize=address>
            $<$<BOOL:${USE_SANITIZER}>:-fsanitize=undefined>
    )

    if (${COPY_AFTER_BUILD})
        message(STATUS "Will copy plugin after every build" )
        set(products_folder ${CMAKE_BINARY_DIR})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Installing ${products_folder}/${PROJECT_NAME}.clap to ~/Library/Audio/Plug-Ins/CLAP/"
                COMMAND ${CMAKE_COMMAND} -E make_directory "~/Library/Audio/Plug-Ins/CLAP"
                COMMAND ${CMAKE_COMMAND} -E copy_directory "${products_folder}/${PROJECT_NAME}.clap" "~/Library/Audio/Plug-Ins/CLAP/${PROJECT_NAME}.clap"
        )
    endif()
else()
    set_target_properties(${PROJECT_NAME} PROPERTIES SUFFIX ".clap" PREFIX "")
    if(UNIX AND ${COPY_AFTER_BUILD})
        message(STATUS "Will copy plugin after every build" )
        set(products_folder ${CMAKE_BINARY_DIR})
        add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Installing ${products_folder}/${PROJECT_NAME}.clap to ~/.clap"
                COMMAND ${CMAKE_COMMAND} -E make_directory "~/.clap"
                COMMAND ${CMAKE_COMMAND} -E copy "${products_folder}/${PROJECT_NAME}.clap" "~/.clap"
        )
    endif()
endif()

