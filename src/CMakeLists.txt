# src/CMakeLists.txt

add_custom_command(OUTPUT magic_tables.generated.cpp magic_tables.generated.h
    COMMAND generate_magic_tables
    DEPENDS generate_magic_tables
    COMMENT "Generate magic tables for move generation"
    )

add_executable(lesschess
    main.cpp
    magic_tables.generated.cpp
    )
set_target_properties(lesschess PROPERTIES CXX_STANDARD 17)
