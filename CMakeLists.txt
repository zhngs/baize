cmake_minimum_required(VERSION 3.6)

project(baize CXX)

include_directories(${PROJECT_SOURCE_DIR})
include_directories(${PROJECT_SOURCE_DIR}/kernel)

# boost-context
include_directories(${PROJECT_SOURCE_DIR}/third_party/boost_1_66_0)
link_directories(${PROJECT_SOURCE_DIR}/third_party/boost_1_66_0/stage/lib)

# quiche
include_directories(${PROJECT_SOURCE_DIR}/third_party/quiche/quiche/include)
link_directories(${PROJECT_SOURCE_DIR}/third_party/quiche/target/release)

# openssl
include_directories(${PROJECT_SOURCE_DIR}/third_party/openssl/include)
link_directories(${PROJECT_SOURCE_DIR}/third_party/openssl/lib)

# srtp 
include_directories(${PROJECT_SOURCE_DIR}/third_party/libsrtp-2.4.2/include)
link_directories(${PROJECT_SOURCE_DIR}/third_party/libsrtp-2.4.2)

set(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${PROJECT_BINARY_DIR}/lib)
add_compile_options(
-g
-Wall
-Wextra
# -Werror
-Wconversion
-Wno-unused-parameter
-Wold-style-cast
-Woverloaded-virtual
-Wpointer-arith
-Wshadow
-Wwrite-strings

-std=c++14
)

add_subdirectory(kernel)
add_subdirectory(example)
add_subdirectory(http)
add_subdirectory(webrtc)

# quic的编译选项
# add_subdirectory(quic)
