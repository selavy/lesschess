# tests/CMakeLists.txt

# set(SRC ${PROJECT_SOURCE_DIR}/src)

add_executable(unittest
    catch_main.cpp
    ../src/move.cpp
    ../src/move.test.cpp
    ../src/position.cpp
    ../src/position.test.cpp
    )
set_target_properties(unittest PROPERTIES CXX_STANDARD 17)
target_include_directories(unittest PUBLIC "${PROJECT_SOURCE_DIR}/third_party/catch")
target_include_directories(unittest PUBLIC
    "${PROJECT_SOURCE_DIR}/third_party/outcome/single-header")
target_include_directories(unittest PUBLIC "${PROJECT_SOURCE_DIR}/src")

add_executable(test_libfmt
    test_libfmt.cpp)
target_include_directories(test_libfmt PUBLIC "${PROJECT_SOURCE_DIR}/third_party/fmt/include")
target_link_libraries(test_libfmt PUBLIC fmt)

add_executable(test_outcome
    test_outcome.cpp)
target_include_directories(test_outcome PUBLIC "${PROJECT_SOURCE_DIR}/third_party/outcome/single-header")
