cmake_minimum_required(VERSION 3.6)

project(baize CXX)

include_directories(${PROJECT_SOURCE_DIR}/src)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
add_compile_options(
-g

-Wall
-Wextra
-Werror
-Wconversion
-Wno-unused-parameter
-Wold-style-cast
-Woverloaded-virtual
-Wpointer-arith
-Wshadow
-Wwrite-strings

-std=c++14
)

add_subdirectory(src)
