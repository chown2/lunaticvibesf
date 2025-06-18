find_package(PkgConfig REQUIRED)
if(${LVF_GRAPHICS_BACKEND} STREQUAL SDL3)
    pkg_check_modules(SDL3 REQUIRED IMPORTED_TARGET sdl3>=3.2.12)
    pkg_check_modules(SDL3_image REQUIRED IMPORTED_TARGET sdl3-image>=3.2.4)
    pkg_check_modules(SDL3_ttf REQUIRED IMPORTED_TARGET sdl3-ttf>=3.2.2)
    list(
        APPEND
        LVF_GRAPHICS_PUBLIC_TARGETS
        PkgConfig::SDL3
        PkgConfig::SDL3_image
        PkgConfig::SDL3_ttf
    )
endif()
pkg_check_modules(libavcodec REQUIRED IMPORTED_TARGET libavcodec)
pkg_check_modules(libavformat REQUIRED IMPORTED_TARGET libavformat)
pkg_check_modules(libavutil REQUIRED IMPORTED_TARGET libavutil)
find_package(CURL REQUIRED)
find_package(cereal 1.3.2 CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)

if(${LVF_GRAPHICS_BACKEND} STREQUAL SDL3)
    set(LVF_GRAPHICS_BACKEND_SRC
        graphics/SDL2/common_SDL2.cpp
        graphics/SDL2/graph_line_SDL2.cpp
        graphics/SDL2/image_SDL2.cpp
        graphics/SDL2/input_SDL2.cpp
        graphics/SDL2/ttf_SDL2.cpp
        graphics/SDL2/window_SDL2.cpp
        input/platform/sdl2/input_sdl2.cpp
    )
endif()

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/git_version.cpp ${CMAKE_BINARY_DIR}/_dummy.cpp
    COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_SOURCE_DIR}/git_version.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_library(
    gamelib
    ${CMAKE_BINARY_DIR}/git_version.cpp
    runtime/state.cpp
    chart/chart.cpp
    chart/chart_bms.cpp
    chart/chart_load_task.cpp
    ${LVF_GRAPHICS_BACKEND_SRC}
    graphics/dxa.cpp
    graphics/sprite.cpp
    graphics/sprite_bar_entry.cpp
    graphics/sprite_bargraph.cpp
    graphics/sprite_button.cpp
    graphics/sprite_cursor.cpp
    graphics/sprite_gauge_grid.cpp
    graphics/sprite_graph.cpp
    graphics/sprite_imagetext.cpp
    graphics/sprite_lane.cpp
    graphics/sprite_number.cpp
    graphics/sprite_on_mouse.cpp
    graphics/sprite_option.cpp
    graphics/sprite_slider.cpp
    graphics/sprite_text.cpp
    graphics/sprite_video.cpp
    graphics/texture_bms_bga.cpp
    graphics/texture_dynamic.cpp
    graphics/texture_video.cpp
    graphics/video.cpp
    input/input_mgr.cpp
    input/platform/windows/input_dinput8.cpp
    input/platform/windows/input_windows.cpp
    input/input_wrapper.cpp
    ruleset/ruleset_bms.cpp
    ruleset/ruleset_bms_auto.cpp
    ruleset/ruleset_bms_replay.cpp
    ruleset/ruleset_network.cpp
    ruleset/ruleset_bms_network.cpp
    scene/scene.cpp
    scene/scene_context.cpp
    scene/scene_decide.cpp
    scene/scene_mgr.cpp
    scene/scene_play.cpp
    scene/scene_pre_select.cpp
    scene/scene_result.cpp
    scene/scene_course_result.cpp
    scene/scene_select.cpp
    scene/scene_keyconfig.cpp
    scene/scene_play_course_trans.cpp
    scene/scene_customize.cpp
    scene/scene_select_imgui.cpp
    scene/scene_play_retry_trans.cpp
    scene/scene_exit_trans.cpp
    scene/scene_play_imgui.cpp
    skin/skin.cpp
    skin/skin_lr2.cpp
    skin/skin_lr2_bargraph.cpp
    skin/skin_lr2_button_callbacks.cpp
    skin/skin_lr2_slider_callbacks.cpp
    skin/skin_lr2_converters.cpp
    skin/skin_lr2_dst.cpp
    skin/skin_lr2_debug.cpp
    skin/skin_lr2_number.cpp
    skin/skin_lr2_number_animation.cpp
    skin/skin_mgr.cpp
    sound/sound_fmod.cpp
    sound/sound_mgr.cpp
    sound/sound_sample.cpp
    sound/soundset.cpp
    sound/soundset_lr2.cpp
    replay/replay_chart.cpp
    runtime/generic_info.cpp
    runtime/i18n.cpp
    arena/arena_data.cpp
    arena/arena_internal.cpp
    arena/arena_host.cpp
    arena/arena_client.cpp
)

target_include_directories(gamelib PUBLIC ${PROJECT_INCLUDE_DIR})

set_target_properties(gamelib PROPERTIES CXX_STANDARD 23)

target_link_libraries(
    gamelib
    PUBLIC ${LVF_GRAPHICS_PUBLIC_TARGETS}
    PUBLIC cereal::cereal
    PUBLIC common
    PUBLIC config
    PUBLIC CURL::libcurl
    PUBLIC db
    PUBLIC fmod
    PUBLIC imgui::imgui
    PUBLIC PkgConfig::libavcodec
    PUBLIC PkgConfig::libavformat
    PUBLIC PkgConfig::libavutil
    PUBLIC re2::re2
    PRIVATE tinyfiledialogs
)

target_precompile_headers(gamelib REUSE_FROM common)

#############################################################

# WIN32 - graphical application/don't spawn a console and use WinMain as entry point (Windows only).
add_executable(LunaticVibesF WIN32 game.cpp LunaticVibesF.exe.manifest)
target_link_libraries(LunaticVibesF PRIVATE gamelib PRIVATE tinyfiledialogs)
set_target_properties(LunaticVibesF PROPERTIES CXX_STANDARD 23)
install(TARGETS LunaticVibesF COMPONENT Game)

add_custom_command(
    TARGET LunaticVibesF
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different $<TARGET_FILE:fmod>
        $<TARGET_FILE_DIR:LunaticVibesF>
)
install(IMPORTED_RUNTIME_ARTIFACTS fmod COMPONENT Game LIBRARY DESTINATION bin)

if(MSVC)
    install(
        FILES $<TARGET_PDB_FILE:LunaticVibesF>
        DESTINATION bin
        COMPONENT Symbols
    )
endif()

add_custom_command(
    TARGET LunaticVibesF
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/res
        $<TARGET_FILE_DIR:LunaticVibesF>/gamedata/resources
)
# NOTE: trailing slash is significant.
install(
    DIRECTORY ${CMAKE_SOURCE_DIR}/res/
    DESTINATION bin/gamedata/resources
    COMPONENT Game
)

add_custom_command(
    TARGET LunaticVibesF
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/LICENSE_3RD_PARTY
        $<TARGET_FILE_DIR:LunaticVibesF>/gamedata/resources/LICENSE_3RD_PARTY
)
install(
    FILES ${CMAKE_SOURCE_DIR}/LICENSE_3RD_PARTY
    DESTINATION bin
    COMPONENT Game
)
