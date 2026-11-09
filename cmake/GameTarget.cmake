# komkommer_add_game()
#
# Builds one game target. TYPE selects which kind:
#   executable  standalone komkommertd / komkommerra
#   shared      Remastered TiberianDawn / RedAlert DLL
#
# Arguments are the parts that differ between those four targets.
# Sanitizer flags, the Windows GUI subsystem, and macOS bundle settings
# are applied here so each game listfile does not repeat them.
#
# TARGET             CMake target name (KomkommerTD, TiberianDawn, ...).
# TYPE               executable or shared.
# SOURCES            Source files. The caller chooses standalone vs DLL files.
# DEFINITIONS        Extra compile definitions. This function does not add
#                    MEGAMAPS; the TD standalone call passes that itself.
# LIBRARIES          Libraries to link (commonv or commonr, plus their deps).
# OUTPUT_NAME        On-disk executable name, such as komkommertd.
# ICON               SVG path for a standalone app icon.
# PRODUCT_NAME       Display name in the Windows version resource.
# VERSION_MAJOR      Major number in that Windows resource.
# VERSION_MINOR      Minor number in that Windows resource.
# ORIGINAL_FILENAME  Original file name in that Windows resource.
# RC_PREFIX          Unique prefix for the generated .rc (one per target).
# BUNDLE_ID          macOS bundle id, used when MAKE_BUNDLE is on.
# BUNDLE_NAME        macOS bundle display name.
# BUNDLE_ICON        macOS icon file name inside the bundle.
# MOD_DIR            Remastered mod folder staged next to a DLL.

function(komkommer_add_game)
    set(options "")
    set(one_value
        TARGET
        TYPE
        OUTPUT_NAME
        ICON
        PRODUCT_NAME
        VERSION_MAJOR
        VERSION_MINOR
        ORIGINAL_FILENAME
        RC_PREFIX
        BUNDLE_ID
        BUNDLE_NAME
        BUNDLE_ICON
        MOD_DIR
    )
    set(multi_value SOURCES DEFINITIONS LIBRARIES)
    # GAME_TARGET, GAME_SOURCES, and so on. An unknown keyword is an error
    # so a typo cannot quietly drop a definition such as MEGAMAPS.
    cmake_parse_arguments(GAME "${options}" "${one_value}" "${multi_value}" ${ARGN})

    if(GAME_UNPARSED_ARGUMENTS)
        message(FATAL_ERROR "komkommer_add_game: unknown arguments: ${GAME_UNPARSED_ARGUMENTS}")
    endif()
    if(GAME_TARGET STREQUAL "" OR GAME_TYPE STREQUAL "")
        message(FATAL_ERROR "komkommer_add_game: TARGET and TYPE are required")
    endif()
    if(NOT GAME_TYPE STREQUAL "executable" AND NOT GAME_TYPE STREQUAL "shared")
        message(FATAL_ERROR "komkommer_add_game: TYPE must be \"executable\" or \"shared\"")
    endif()
    if(GAME_PRODUCT_NAME STREQUAL "" OR GAME_VERSION_MAJOR STREQUAL "" OR GAME_VERSION_MINOR STREQUAL ""
        OR GAME_ORIGINAL_FILENAME STREQUAL "" OR GAME_RC_PREFIX STREQUAL "")
        message(FATAL_ERROR "komkommer_add_game(${GAME_TARGET}): PRODUCT_NAME, VERSION_MAJOR, VERSION_MINOR, ORIGINAL_FILENAME and RC_PREFIX are required")
    endif()
    if(GAME_TYPE STREQUAL "executable" AND (GAME_OUTPUT_NAME STREQUAL "" OR GAME_ICON STREQUAL ""
        OR GAME_BUNDLE_ID STREQUAL "" OR GAME_BUNDLE_NAME STREQUAL "" OR GAME_BUNDLE_ICON STREQUAL ""))
        message(FATAL_ERROR "komkommer_add_game(${GAME_TARGET}): executable targets need OUTPUT_NAME, ICON, BUNDLE_ID, BUNDLE_NAME and BUNDLE_ICON")
    endif()
    if(GAME_TYPE STREQUAL "shared" AND GAME_MOD_DIR STREQUAL "")
        message(FATAL_ERROR "komkommer_add_game(${GAME_TARGET}): shared targets need MOD_DIR")
    endif()

    set(game_icon "")
    if(GAME_TYPE STREQUAL "executable")
        include(BuildIcons)
        make_icon(INPUT "${GAME_ICON}" OUTPUT game_icon)
    endif()

    # Windows version resource. Other platforms leave this empty, as before.
    set(game_rc "")
    if(WIN32 OR CMAKE_SYSTEM_NAME STREQUAL "Windows")
        include(ProductVersion)
        set(version_args
            NAME "${GAME_PRODUCT_NAME}"
            BUNDLE "komkommer"
            VERSION_MAJOR ${GAME_VERSION_MAJOR}
            VERSION_MINOR ${GAME_VERSION_MINOR}
            COMPANY_NAME "komkommer"
            COMPANY_COPYRIGHT "Based on code released under GPLv3 by EA"
            ORIGINAL_FILENAME "${GAME_ORIGINAL_FILENAME}"
            RCFILE_PREFIX "${GAME_RC_PREFIX}"
        )
        if(game_icon)
            list(APPEND version_args ICON "${game_icon}")
        endif()
        generate_product_version(game_rc ${version_args})
    endif()

    if(GAME_TYPE STREQUAL "executable")
        # The icon file has to be a source or its custom command never runs.
        add_executable(${GAME_TARGET} ${GAME_SOURCES} ${game_rc} ${game_icon})
    else()
        add_library(${GAME_TARGET} SHARED ${GAME_SOURCES} ${game_rc})
        # GlyphX loads TiberianDawn.dll / RedAlert.dll, so drop the "lib" prefix.
        set_target_properties(${GAME_TARGET} PROPERTIES PREFIX "")
    endif()

    # MAKE_BUNDLE is set by the top-level listfile on Apple.
    # The Windows VERSION_* arguments do not feed these strings; both apps report 1.0.
    if(GAME_TYPE STREQUAL "executable" AND MAKE_BUNDLE)
        set_source_files_properties(${game_icon} PROPERTIES MACOSX_PACKAGE_LOCATION Resources)
        set_target_properties(${GAME_TARGET} PROPERTIES
            MACOSX_BUNDLE True
            MACOSX_BUNDLE_GUI_IDENTIFIER "${GAME_BUNDLE_ID}"
            MACOSX_BUNDLE_BUNDLE_NAME "${GAME_BUNDLE_NAME}"
            MACOSX_BUNDLE_BUNDLE_VERSION "1.0"
            MACOSX_BUNDLE_SHORT_VERSION_STRING "1.0"
            MACOSX_BUNDLE_COPYRIGHT "Based on code released under GPLv3 by EA"
            MACOSX_BUNDLE_ICON_FILE "${GAME_BUNDLE_ICON}"
        )
    endif()

    # _DEBUG only in the Debug configuration. DEFINITIONS are the caller's extras.
    target_compile_definitions(${GAME_TARGET} PUBLIC $<$<CONFIG:Debug>:_DEBUG> ${GAME_DEFINITIONS})
    target_include_directories(${GAME_TARGET} PUBLIC ${CMAKE_SOURCE_DIR} .)
    target_link_libraries(${GAME_TARGET} ${GAME_LIBRARIES})

    if(GAME_TYPE STREQUAL "executable")
        set_target_properties(${GAME_TARGET} PROPERTIES OUTPUT_NAME "${GAME_OUTPUT_NAME}")

        # GUI subsystem, no extra console. MSVC and MinGW spell that differently.
        if(MSVC)
            target_link_options(${GAME_TARGET} PRIVATE /subsystem:windows /ENTRY:mainCRTStartup)
        endif()
        if(WIN32 AND NOT MSVC)
            set_target_properties(${GAME_TARGET} PROPERTIES LINK_FLAGS "-mwindows")
        endif()

        # Standalone dev builds only. The Remastered DLLs are not sanitized.
        if(BUILD_WITH_UBSAN)
            target_compile_options(${GAME_TARGET} PUBLIC
                -fsanitize=undefined,float-divide-by-zero,integer,implicit-conversion,implicit-integer-truncation,implicit-integer-arithmetic-value-change,local-bounds,nullability
                -g3 -fno-omit-frame-pointer)
            target_link_options(${GAME_TARGET} PUBLIC
                -fsanitize=undefined,float-divide-by-zero,integer,implicit-conversion,implicit-integer-truncation,implicit-integer-arithmetic-value-change,local-bounds,nullability)
        endif()
        if(BUILD_WITH_ASAN)
            target_compile_options(${GAME_TARGET} PUBLIC
                -fsanitize=address
                -g3 -fno-omit-frame-pointer)
            target_link_options(${GAME_TARGET} PUBLIC -fsanitize=address)
        endif()
    else()
        add_custom_command(TARGET ${GAME_TARGET} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/resources/remaster_mods/${GAME_MOD_DIR} ${CMAKE_BINARY_DIR}/${GAME_MOD_DIR}
            COMMAND ${CMAKE_COMMAND} -E make_directory ${CMAKE_BINARY_DIR}/${GAME_MOD_DIR}/Data
            COMMAND ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:${GAME_TARGET}> ${CMAKE_BINARY_DIR}/${GAME_MOD_DIR}/Data/
        )
    endif()
endfunction()
