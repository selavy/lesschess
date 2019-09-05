# tests/CMakeLists.txt

#
# Unit Test
#
# TODO: make a library version of lesschess to link against
add_executable(unittest
    catch_main.cpp
    "${PROJECT_SOURCE_DIR}/src/move.cpp"
    "${PROJECT_SOURCE_DIR}/src/move.test.cpp"
    "${PROJECT_SOURCE_DIR}/src/position.cpp"
    "${PROJECT_SOURCE_DIR}/src/position.test.cpp"
    "${PROJECT_SOURCE_DIR}/src/evaluate.cpp"
    "${PROJECT_SOURCE_DIR}/src/search.cpp"
    "${PROJECT_SOURCE_DIR}/src/search.test.cpp"
    "${PROJECT_SOURCE_DIR}/src/perft.cpp"
    "${PROJECT_SOURCE_DIR}/src/detail/magic_tables.generated.cpp"
    )
set_target_properties(unittest PROPERTIES CXX_STANDARD 17)
target_include_directories(unittest PUBLIC "${PROJECT_SOURCE_DIR}/third_party/catch")
target_include_directories(unittest PUBLIC "${PROJECT_SOURCE_DIR}/src")

#
# Perft Test
#
add_executable(perft
    catch_main.cpp
    "${PROJECT_SOURCE_DIR}/src/move.cpp"
    "${PROJECT_SOURCE_DIR}/src/position.cpp"
    "${PROJECT_SOURCE_DIR}/src/perft.cpp"
    "${PROJECT_SOURCE_DIR}/src/perft.test.cpp"
    "${PROJECT_SOURCE_DIR}/src/detail/magic_tables.generated.cpp"
    )
set_target_properties(perft PROPERTIES CXX_STANDARD 17)
target_include_directories(perft PUBLIC "${PROJECT_SOURCE_DIR}/third_party/catch")
target_include_directories(perft PUBLIC "${PROJECT_SOURCE_DIR}/src")

#
# Test libfmt
#
add_executable(test_libfmt
    test_libfmt.cpp)
target_include_directories(test_libfmt PUBLIC "${PROJECT_SOURCE_DIR}/third_party/fmt/include")
target_link_libraries(test_libfmt PUBLIC fmt)
