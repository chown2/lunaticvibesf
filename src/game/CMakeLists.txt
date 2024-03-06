if (${GRAPHICS_BACKEND} STREQUAL SDL2)
    find_package(SDL2 CONFIG REQUIRED)
    find_package(SDL2_image CONFIG REQUIRED)
    find_package(SDL2_ttf CONFIG REQUIRED)
    find_package(sdl2-gfx CONFIG REQUIRED)
endif()
if (${VIDEO_ENABLED})
    find_package(FFMPEG REQUIRED)
endif()
find_package(cereal CONFIG REQUIRED)
find_package(imgui CONFIG REQUIRED)
find_package(re2 CONFIG REQUIRED)
find_package(CURL REQUIRED)

if (${GRAPHICS_BACKEND} STREQUAL SDL2)
    set(GRAPHICS_BACKEND_SRC
        graphics/SDL2/common_SDL2.cpp
        graphics/SDL2/input.cpp
        graphics/SDL2/rect_SDL2.cpp
        graphics/SDL2/ttf_SDL2.cpp
        graphics/SDL2/window_SDL2.cpp
    )
endif()

add_custom_command(
    OUTPUT ${CMAKE_BINARY_DIR}/git_version.cpp
           ${CMAKE_BINARY_DIR}/_dummy.cpp
    COMMAND ${CMAKE_COMMAND} -P 
            ${CMAKE_CURRENT_SOURCE_DIR}/git_version.cmake
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_library(gamelib
    ${CMAKE_BINARY_DIR}/git_version.cpp
    runtime/state.cpp
    chart/chart.cpp
    chart/chart_bms.cpp
    ${GRAPHICS_BACKEND_SRC}
    graphics/sprite.cpp
    graphics/sprite_bar_entry.cpp
    graphics/sprite_graph.cpp
    graphics/sprite_imagetext.cpp
    graphics/sprite_lane.cpp
    graphics/sprite_video.cpp
    graphics/texture_extra.cpp
    graphics/video.cpp
    graphics/dxa.cpp
    input/input_mgr.cpp
    input/input_mgr_sdl.cpp
    input/input_dinput8.cpp
    input/input_windows.cpp
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
    skin/skin_lr2_button_callbacks.cpp
    skin/skin_lr2_slider_callbacks.cpp
    skin/skin_lr2_converters.cpp
    skin/skin_lr2_dst.cpp
    skin/skin_lr2_debug.cpp
    skin/skin_mgr.cpp
    sound/sound_fmod.cpp
    sound/sound_fmod_callback.cpp
    sound/sound_mgr.cpp
    sound/sound_sample.cpp  
    sound/soundset.cpp
    sound/soundset_lr2.cpp
    replay/replay_chart.cpp
    runtime/i18n.cpp 
    arena/arena_data.cpp
    arena/arena_internal.cpp
    arena/arena_host.cpp
    arena/arena_client.cpp
)


target_include_directories(gamelib PUBLIC 
    ${PROJECT_INCLUDE_DIR}
)

set_target_properties(gamelib PROPERTIES
    CXX_STANDARD 17
)

target_link_libraries(gamelib
    PUBLIC common
    PUBLIC config
    PUBLIC db
    PUBLIC CURL::libcurl
    PUBLIC cereal::cereal
    PUBLIC fmod
    PUBLIC imgui::imgui
    PUBLIC re2::re2
)

if (${GRAPHICS_BACKEND} STREQUAL SDL2)
    target_link_libraries(gamelib
        PUBLIC $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>
        PUBLIC $<IF:$<TARGET_EXISTS:SDL2_image::SDL2_image>,SDL2_image::SDL2_image,SDL2_image::SDL2_image-static>
        PUBLIC $<IF:$<TARGET_EXISTS:SDL2_ttf::SDL2_ttf>,SDL2_ttf::SDL2_ttf,SDL2_ttf::SDL2_ttf-static>
        PUBLIC SDL2::SDL2_gfx
        PUBLIC SDL2::SDL2main
    )
endif()

if (${VIDEO_ENABLED})
    target_include_directories(gamelib PUBLIC ${FFMPEG_INCLUDE_DIRS})
    target_link_directories(gamelib PUBLIC ${FFMPEG_LIBRARY_DIRS})
    target_link_libraries(gamelib PUBLIC ${FFMPEG_LIBRARIES})
else()
    target_compile_definitions(gamelib PUBLIC -DVIDEO_DISABLED)
endif()

target_precompile_headers(gamelib REUSE_FROM common)

#############################################################

if (WIN32)
    add_executable(LunaticVibes game.cpp LunaticVibes.exe.manifest)
else()
    add_executable(LunaticVibes game.cpp)
endif()
target_link_libraries(LunaticVibes PRIVATE gamelib)
set_target_properties(LunaticVibes PROPERTIES
    CXX_STANDARD 17
)

add_custom_command(TARGET LunaticVibes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:fmod>
    $<TARGET_FILE_DIR:LunaticVibes>
)

add_custom_command(TARGET LunaticVibes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/res
    $<TARGET_FILE_DIR:LunaticVibes>/gamedata/resources
)

add_custom_command(TARGET LunaticVibes POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    ${CMAKE_SOURCE_DIR}/LICENSE_3RD_PARTY
    $<TARGET_FILE_DIR:LunaticVibes>/gamedata/resources/LICENSE_3RD_PARTY
)