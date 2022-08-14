add_executable(http_server http_server.cc http_message.cc http_tasklet.cc)
target_link_libraries(http_server baize pthread boost_context)