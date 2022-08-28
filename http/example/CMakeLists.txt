add_executable(http_hello http_hello.cc)
target_link_libraries(http_hello baize_http baize pthread boost_context)

add_executable(https_hello https_hello.cc)
target_link_libraries(https_hello baize_http baize pthread boost_context ssl crypto dl)