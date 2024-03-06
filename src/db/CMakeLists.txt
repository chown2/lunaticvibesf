find_package(re2 CONFIG REQUIRED)
find_package(SQLite3 REQUIRED)

add_library(db STATIC
    db_conn.cpp
    db_score.cpp
    db_song.cpp
)

target_include_directories(db PRIVATE
    ${PROJECT_INCLUDE_DIR}
)

set_target_properties(db PROPERTIES
    CXX_STANDARD 17)

target_link_libraries(db
    PUBLIC SQLite::SQLite3
    PUBLIC common
    PUBLIC re2::re2
)

target_precompile_headers(db REUSE_FROM common)
