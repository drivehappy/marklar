cmake_minimum_required (VERSION 2.8)
project (marklarc)

include_directories ("${PROJECT_SOURCE_DIR}/src/libmarklarc")
add_subdirectory (src/libmarklarc)

add_subdirectory (tests)

add_executable (marklarc "src/main/main.cpp")
target_link_libraries (marklarc libmarklarc -lboost_program_options -lboost_system -lboost_filesystem)




