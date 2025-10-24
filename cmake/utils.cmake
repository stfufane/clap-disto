# The subfolder to store plugin data
# Will be under :
# - ~/Library/Application Support on macOS
# - %AppData on windows
# - ~/.config on Linux
set(plugin_folder "stfefane/${PROJECT_NAME}")

if (APPLE)
    set(clap_dest_folder "~/Library/Audio/Plug-Ins/CLAP")
elseif (UNIX)
    set(clap_dest_folder "~/.clap")
elseif (WIN32)
    set(clap_dest_folder "%LOCALAPPDATA%/Programs/Common/CLAP")
else()
    message(FATAL_ERROR "Unsupported platform")
    return()
endif()

function(copy_after_build target)
    message(STATUS "Will copy plugin to ${clap_dest_folder} after every build" )
    set(debug_suffix "_debug")
    set(dest_file "$<IF:$<CONFIG:Debug>,${target}${debug_suffix},${target}>.clap")
    set(dest_path   "${clap_dest_folder}/${dest_file}")

    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E make_directory "${clap_dest_folder}"
            COMMAND ${CMAKE_COMMAND} -E echo "Removing previous version of the plugin at: ${dest_path}"
            COMMAND ${CMAKE_COMMAND} -E rm -rf "${dest_path}"
    )

    if (APPLE)
        # On macOS the CLAP is a bundle directory
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Signing bundle if configured"
                COMMAND sh ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mac/run_sign.sh "$<TARGET_BUNDLE_DIR:${target}>"
                COMMAND ${CMAKE_COMMAND} -E echo "Installing $<TARGET_BUNDLE_DIR:${target}> to ${dest_path}"
                COMMAND ${CMAKE_COMMAND} -E copy_directory
                        "$<TARGET_BUNDLE_DIR:${target}>"
                        "${dest_path}"
        )
    else()
        # On Linux/Windows the CLAP is a single shared library file named .clap
        add_custom_command(TARGET ${target} POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E echo "Installing $<TARGET_FILE:${target}> to ${dest_path}"
                COMMAND ${CMAKE_COMMAND} -E copy
                        "$<TARGET_FILE:${target}>"
                        "${dest_path}"
        )
    endif()

endfunction()