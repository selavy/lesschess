cmake_minimum_required(VERSION 3.5)

project(lesschess
    VERSION 0.0.1
    LANGUAGES C CXX
    DESCRIPTION "Chess Engine"
    )

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/lib")

add_subdirectory(tools)
add_subdirectory(src)

# add_executable(lesschess src/main.cpp)
# set_target_properties(lesschess PROPERTIES CXX_STANDARD 17)
# add_executable(unittest tests/unittest_main.cpp)
# target_include_directories(unittest PUBLIC "third_party/catch")
# set_target_properties(unittest PROPERTIES CXX_STANDARD 17)
