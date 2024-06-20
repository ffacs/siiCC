project(siiCC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(siicc LALR_main.cpp siicc_EBNF.cpp)

add_executable(BNF_driver_gen EBNF_parser_driver_generator.cpp LALR_table_generator.cpp LALR_parser_generator.cpp)