project(siiCC)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(siicc main.cpp LALR_table_generator.cpp LALR_parser_generator.cpp)

add_executable(test test.cpp siicc_cpp.cpp)