# tests/CMakeLists.txt

# set(SRC ${PROJECT_SOURCE_DIR}/src)

add_executable(unittest
    catch_main.cpp
    ../src/move.cpp
    ../src/move.test.cpp
    )
set_target_properties(unittest PROPERTIES CXX_STANDARD 17)
target_include_directories(unittest PUBLIC "${PROJECT_SOURCE_DIR}/third_party/catch")
target_include_directories(unittest PUBLIC "${PROJECT_SOURCE_DIR}/src")
