set(REDALERT_SRC
    aadata.cpp
    abstract.cpp
    adata.cpp
    aircraft.cpp
    anim.cpp
    audio.cpp
    bar.cpp
    base.cpp
    bbdata.cpp
    bdata.cpp
    bench.cpp
    bigcheck.cpp
    building.cpp
    bullet.cpp
    cargo.cpp
    carry.cpp
    ccini.cpp
    ccptr.cpp
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
    crate.cpp
    credits.cpp
    debug.cpp
    dial8.cpp
    dialog.cpp
    dibfile.cpp
    dibutil.cpp
    display.cpp
    door.cpp
    drive.cpp
    drop.cpp
    edit.cpp
    egos.cpp
    ending.cpp
    event.cpp
    expand.cpp
    face.cpp
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
    iconlist.cpp
    iconset.cpp
    idata.cpp
    infantry.cpp
    init.cpp
    intro.cpp
    iomap.cpp
    ioobj.cpp
    jshell.cpp
    layer.cpp
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
    rawolapi.cpp
    reinf.cpp
    rules.cpp
    saveload.cpp
    scenario.cpp
    score.cpp
    scroll.cpp
    sdata.cpp
    seditdlg.cpp
    session.cpp
    shapebtn.cpp
    sidebar.cpp
    skirmish.cpp
    slider.cpp
    smudge.cpp
    sounddlg.cpp
    special.cpp
    startup.cpp
    statbtn.cpp
    stats.cpp
    super.cpp
    tab.cpp
    taction.cpp
    target.cpp
    tdata.cpp
    team.cpp
    teamtype.cpp
    techno.cpp
    template.cpp
    terrain.cpp
    tevent.cpp
    textbtn.cpp
    theme.cpp
    tooltip.cpp
    tracker.cpp
    trigger.cpp
    trigtype.cpp
    txtlabel.cpp
    udata.cpp
    unit.cpp
    vdata.cpp
    version.cpp
    vessel.cpp
    visudlg.cpp
    vortex.cpp
    warhead.cpp
    weapon.cpp
    winstub.cpp
    wolstrng.cpp
)

if(NETWORKING)
    list(APPEND REDALERT_NET_SRC
        ipxconn.cpp
        ipxgconn.cpp
        ipxmgr.cpp
        netdlg.cpp
        sendfile.cpp
    )
endif()

if(CMAKE_CONFIGURATION_TYPES)
    # Glob all the header files together to add to the project for msvc/xcode.
    # Not ideal as CMake won't notice if you add any until something else prompts a CMake run
    # but this is largely a convenience feature for MSVC/XCode users.
    file(GLOB_RECURSE REDALERT_HEADERS "*.h")
    set(_REDALERT_SOURCES ${REDALERT_SRC})

    foreach(_REDALERT_SOURCE IN ITEMS ${_REDALERT_SOURCES})
        get_filename_component(_REDALERT_SOURCE_PATH "${_REDALERT_SOURCE}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _REDALERT_GROUP_PATH "${_REDALERT_SOURCE_PATH}")
        string(REPLACE "/" "\\" _REDALERT_GROUP_PATH "${_REDALERT_GROUP_PATH}")
        source_group("Source Files\\${_REDALERT_GROUP_PATH}" FILES "${_REDALERT_SOURCE}")
    endforeach()

    foreach(_REDALERT_HEADER IN ITEMS ${REDALERT_HEADERS})
        get_filename_component(_REDALERT_HEADER_PATH "${_REDALERT_HEADER}" PATH)
        string(REPLACE "${CMAKE_CURRENT_SOURCE_DIR}" "" _REDALERT_HGROUP_PATH "${_REDALERT_HEADER_PATH}")
        string(REPLACE "/" "\\" _REDALERT_HGROUP_PATH "${_REDALERT_HGROUP_PATH}")
        source_group("Header Files\\${_REDALERT_HGROUP_PATH}" FILES "${_REDALERT_HEADER}")
    endforeach()
endif()

include(GameTarget)

if(BUILD_REMASTERRA)
    # Definitions: REMASTER_DEFS (_USRDLL and REMASTER_BUILD).
    komkommer_add_game(
        TARGET RedAlert
        TYPE shared
        SOURCES
            dllinterface.cpp
            dllinterfaceeditor.cpp
            sidebarglyphx.cpp
            ${REDALERT_SRC}
            ${REDALERT_HEADERS}
        DEFINITIONS ${REMASTER_DEFS}
        LIBRARIES commonr ${REMASTER_LIBS} ${STATIC_LIBS}
        PRODUCT_NAME "komkommer RedAlert DLL"
        VERSION_MAJOR 3
        VERSION_MINOR 3
        ORIGINAL_FILENAME "RedAlert.dll"
        RC_PREFIX RemasterRA
        MOD_DIR komkommer_RA
    )
endif()

if(BUILD_VANILLARA)
    set(REDALERT_DEFS ${VANILLA_DEFS})
    if(MAP_EDITORRA)
        list(APPEND REDALERT_DEFS INTERNAL_VERSION)
    endif()

    komkommer_add_game(
        TARGET KomkommerRA
        TYPE executable
        SOURCES ${REDALERT_SRC} ${REDALERT_NET_SRC} ${REDALERT_HEADERS}
        DEFINITIONS ${REDALERT_DEFS}
        LIBRARIES commonv ${VANILLA_LIBS} ${STATIC_LIBS}
        OUTPUT_NAME komkommerra
        ICON "${CMAKE_SOURCE_DIR}/resources/komkommerra_icon.svg"
        PRODUCT_NAME "KomkommerRA"
        VERSION_MAJOR 1
        VERSION_MINOR 0
        ORIGINAL_FILENAME "komkommerra.exe"
        RC_PREFIX KomkommerRA
        BUNDLE_ID com.komkommer.komkommerra
        BUNDLE_NAME KomkommerRA
        BUNDLE_ICON komkommerra_icon.icns
    )
endif()
