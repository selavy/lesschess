# src/CMakeLists.txt

# # NOTE(peter): just going to check in the generated files for now
# #              because not having them in the source tree messes
# #              up my other tooling.  Could have CMake generate them
# #              in the source tree and add to the gitignore?
# add_custom_command(OUTPUT magic_tables.generated.cpp magic_tables.generated.h
#     COMMAND generate_magic_tables
#     DEPENDS generate_magic_tables
#     COMMENT "Generate magic tables for move generation"
#     )

add_executable(lesschess
    main.cpp
    move.cpp
    position.cpp
    detail/magic_tables.generated.cpp
    )
set_target_properties(lesschess PROPERTIES CXX_STANDARD 17)
target_include_directories(lesschess PUBLIC
    "${PROJECT_SOURCE_DIR}/third_party/outcome/single-header")
