# add a SetFile POST_BUILD for bundles if you aren't using xcode
function(macos_bundle_flag target)
    if (NOT ${CMAKE_GENERATOR} STREQUAL "Xcode")
        add_custom_command(TARGET ${target} POST_BUILD
                WORKING_DIRECTORY $<TARGET_PROPERTY:${target},LIBRARY_OUTPUT_DIRECTORY>
                COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mac/BundlePkgInfo "$<TARGET_FILE_DIR:${target}>/../PkgInfo")
    endif()

    set_target_properties(${PROJECT_NAME} PROPERTIES
            BUNDLE True
            BUNDLE_EXTENSION clap
            MACOSX_BUNDLE TRUE
            XCODE_ATTRIBUTE_GENERATE_PKGINFO_FILE "YES"
            LIBRARY_OUTPUT_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE_GUI_IDENTIFIER dev.stephanealbanese.${PROJECT_NAME}
            MACOSX_BUNDLE_BUNDLE_NAME ${PROJECT_NAME}
            MACOSX_BUNDLE_BUNDLE_VERSION "0.1"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "0.1"
            MACOSX_BUNDLE_INFO_PLIST ${CMAKE_CURRENT_SOURCE_DIR}/cmake/mac/${PROJECT_NAME}.plist.in
    )
endfunction(macos_bundle_flag)

macos_bundle_flag(${PROJECT_NAME})

target_compile_options(${PROJECT_NAME} PRIVATE
        -Werror
        $<$<BOOL:${USE_SANITIZER}>:-fsanitize=address>
        $<$<BOOL:${USE_SANITIZER}>:-fsanitize=undefined>
)
target_link_options(${PROJECT_NAME} PRIVATE
        $<$<BOOL:${USE_SANITIZER}>:-fsanitize=address>
        $<$<BOOL:${USE_SANITIZER}>:-fsanitize=undefined>
)