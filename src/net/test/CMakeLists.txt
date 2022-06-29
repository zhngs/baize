add_executable(socket_test socket_test.cc)
target_link_libraries(socket_test baize_net baize_log baize_time)

add_executable(routine_test routine_test.cc)
target_link_libraries(routine_test baize_net boost_context baize_log baize_time)
