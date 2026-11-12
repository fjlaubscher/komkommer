set(TIBDAWN_SRC
    aadata.cpp
    abstract.cpp
    adata.cpp
    aircraft.cpp
    anim.cpp
    audio.cpp
    base.cpp
    bbdata.cpp
    bdata.cpp
    building.cpp
    bullet.cpp
    cargo.cpp
    ccini.cpp
    cdata.cpp
    cell.cpp
    checkbox.cpp
    cheklist.cpp
    colrlist.cpp
    combat.cpp
    confdlg.cpp
    conquer.cpp
    const.cpp
    coord.cpp
    credits.cpp
    debug.cpp
    dial8.cpp
    dialog.cpp
    display.cpp
    door.cpp
    drive.cpp
    drop.cpp
    edit.cpp
    ending.cpp
    event.cpp
    expand.cpp
    facing.cpp
    factory.cpp
    findpath.cpp
    flasher.cpp
    fly.cpp
    foot.cpp
    fuse.cpp
    gamedlg.cpp
    gauge.cpp
    globals.cpp
    goptions.cpp
    gscreen.cpp
    hdata.cpp
    heap.cpp
    help.cpp
    house.cpp
    iconset.cpp
    idata.cpp
    infantry.cpp
    init.cpp
    intro.cpp
    iomap.cpp
    ioobj.cpp
    jshell.cpp
    layer.cpp
    lib.cpp
    list.cpp
    loaddlg.cpp
    logic.cpp
    map.cpp
    mapeddlg.cpp
    mapedit.cpp
    mapedplc.cpp
    mapedsel.cpp
    mapedtm.cpp
    mapsel.cpp
    menus.cpp
    miscasm.cpp
    mission.cpp
    monoc.cpp
    mouse.cpp
    mplayer.cpp
    msgbox.cpp
    msglist.cpp
    nulldlg.cpp
    object.cpp
    odata.cpp
    options.cpp
    overlay.cpp
    power.cpp
    profile.cpp
    queue.cpp
    radar.cpp
    radio.cpp
    reinf.cpp
    rules.cpp
    saveload.cpp
    scenario.cpp
    scenarioini.cpp
    score.cpp
    scroll.cpp
    sdata.cpp
    shapebtn.cpp
    sidebar.cpp
    skirmish.cpp
    slider.cpp
    smudge.cpp
    sounddlg.cpp
    special.cpp
    startup.cpp
    stats.cpp
    super.cpp
    tab.cpp
    tarcom.cpp
    target.cpp
    tdata.cpp
    team.cpp
    teamtype.cpp
    techno.cpp
    template.cpp
    terrain.cpp
    textbtn.cpp
    theme.cpp
    trigger.cpp
    turret.cpp
    txtlabel.cpp
    udata.cpp
    unit.cpp
    visudlg.cpp
    winstub.cpp
)

if(NETWORKING)
    list(APPEND TIBDAWN_NET_SRC
        ipxconn.cpp
        ipxgconn.cpp
        ipxmgr.cpp
        netdlg.cpp
    )
endif()

if(CMAKE_CONFIGURATION_TYPES)
    # Glob all the header files together to add to the project for msvc/xcode.
    # Not ideal as CMake won't notice if you add any until something else prompts a CMake run
    # but this is largely a convenience feature for MSVC/XCode users.
    file(GLOB_RECURSE TIBDAWN_HEADERS "*.h")
    set(_TIBDAWN_SOURCES ${TIBDAWN_SRC})

    foreach(_TIBDAWN_SOURCE IN ITEMS ${_TIBDAWN_SOURCES})
        get_filename_component(_TIBDAWN_SOURCE_PATH "${_TIBDAWN_SOURCE}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _TIBDAWN_GROUP_PATH "${_TIBDAWN_SOURCE_PATH}")
        string(REPLACE "/" "\\" _TIBDAWN_GROUP_PATH "${_TIBDAWN_GROUP_PATH}")
        source_group("Source Files\\${_TIBDAWN_GROUP_PATH}" FILES "${_TIBDAWN_SOURCE}")
    endforeach()

    foreach(_TIBDAWN_HEADER IN ITEMS ${TIBDAWN_HEADERS})
        get_filename_component(_TIBDAWN_HEADER_PATH "${_TIBDAWN_HEADER}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _TIBDAWN_HGROUP_PATH "${_TIBDAWN_HEADER_PATH}")
        string(REPLACE "/" "\\" _TIBDAWN_HGROUP_PATH "${_TIBDAWN_HGROUP_PATH}")
        source_group("Header Files\\${_TIBDAWN_HGROUP_PATH}" FILES "${_TIBDAWN_HEADER}")
    endforeach()
endif()

include(GameTarget)

if(BUILD_REMASTERTD)
    # Definitions: REMASTER_DEFS (_USRDLL and REMASTER_BUILD).
    komkommer_add_game(
        TARGET TiberianDawn
        TYPE shared
        SOURCES
            dllinterface.cpp
            dllinterfaceeditor.cpp
            sidebarglyphx.cpp
            ${TIBDAWN_SRC}
            ${TIBDAWN_HEADERS}
        DEFINITIONS ${REMASTER_DEFS}
        LIBRARIES commonr ${REMASTER_LIBS} ${STATIC_LIBS}
        PRODUCT_NAME "komkommer TiberianDawn DLL"
        VERSION_MAJOR 1
        VERSION_MINOR 7
        ORIGINAL_FILENAME "TiberianDawn.dll"
        RC_PREFIX RemasterTD
        MOD_DIR komkommer_TD
    )
endif()

if(BUILD_VANILLATD)
    # MEGAMAPS applies to this standalone executable.
    set(TIBDAWN_DEFS ${VANILLA_DEFS} MEGAMAPS)
    if(MAP_EDITORTD)
        list(APPEND TIBDAWN_DEFS INTERNAL_VERSION)
    endif()

    komkommer_add_game(
        TARGET KomkommerTD
        TYPE executable
        SOURCES ${TIBDAWN_SRC} ${TIBDAWN_NET_SRC} ${TIBDAWN_HEADERS}
        DEFINITIONS ${TIBDAWN_DEFS}
        LIBRARIES commonv ${VANILLA_LIBS} ${STATIC_LIBS}
        OUTPUT_NAME komkommertd
        ICON "${CMAKE_SOURCE_DIR}/resources/komkommertd_icon.svg"
        PRODUCT_NAME "KomkommerTD"
        VERSION_MAJOR 1
        VERSION_MINOR 0
        ORIGINAL_FILENAME "komkommertd.exe"
        RC_PREFIX KomkommerTD
        BUNDLE_ID com.komkommer.komkommertd
        BUNDLE_NAME KomkommerTD
        BUNDLE_ICON komkommertd_icon.icns
    )
endif()
