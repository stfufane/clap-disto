function(copy_after_build target)
    if (APPLE)
        set(clap_dest_folder "~/Library/Audio/Plug-Ins/CLAP/")
    elseif (UNIX)
        set(clap_dest_folder "~/.clap")
    elseif (WIN32)
        set(clap_dest_folder "%LOCALAPPDATA%\\Programs\\Common\\CLAP")
    else()
        message(STATUS "Unsupported platform for plugin copy")
        return()
    endif()

    message(STATUS "Will copy plugin after every build" )
    set(products_folder ${CMAKE_BINARY_DIR})

    add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Installing ${products_folder}/${target}.clap to ${clap_dest_folder}"
            COMMAND ${CMAKE_COMMAND} -E make_directory "${clap_dest_folder}"
            COMMAND ${CMAKE_COMMAND} -E echo "Removing previous version of the plugin"
            COMMAND ${CMAKE_COMMAND} -E rm -r "${clap_dest_folder}${target}.clap"
    )

    if (APPLE)
        # On macos the bundle is technically a directory so the copy command does not work
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${products_folder}/${target}.clap" "${clap_dest_folder}${target}.clap"
        )
    else()
        add_custom_command(TARGET ${target} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy "${products_folder}/${target}.clap" "${clap_dest_folder}${target}.clap"
        )
    endif()

endfunction()