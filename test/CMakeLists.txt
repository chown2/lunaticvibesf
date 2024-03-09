find_package(plog CONFIG REQUIRED)
find_package(GTest CONFIG REQUIRED)

include(GoogleTest)

add_executable(apptest
    test_main.cpp
    test_config.cpp
    test_db.cpp
    common/test_encoding.cpp
    common/test_fraction.cpp
    common/test_chartformat_bms.cpp
    common/test_path.cpp
    game/test_graphics.cpp
 "game/test_lr2skin.cpp")
target_link_libraries(apptest PUBLIC
    GTest::gtest GTest::gmock)

gtest_discover_tests(apptest
    WORKING_DIRECTORY "$<TARGET_FILE_DIR:apptest>"
)

set_target_properties(apptest PROPERTIES
    CXX_STANDARD 17
    CTEST_OUTPUT_ON_FAILURE TRUE 
    GTEST_COLOR TRUE
)
    
target_link_libraries(apptest 
    PUBLIC gamelib
    PUBLIC plog::plog
)
target_include_directories(apptest PRIVATE
    ${PROJECT_INCLUDE_DIR}
)

# NOTE: we need to copy dependencies before gtest_discover_tests runs, as it will launch the executable, which will fail
# as the libraries weren't installed yet..
add_custom_command(TARGET apptest PRE_LINK
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    $<TARGET_FILE:fmod>
    $<TARGET_FILE_DIR:apptest>
)

add_custom_command(TARGET apptest PRE_LINK
    COMMAND ${CMAKE_COMMAND} -E copy_directory 
    "${CMAKE_CURRENT_SOURCE_DIR}/test"
    "$<TARGET_FILE_DIR:apptest>/test"
)
