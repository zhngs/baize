add_executable(Timestamp_test Timestamp_test.cc)
target_link_libraries(Timestamp_test baize pthread boost_context)

add_executable(Timer_test Timer_test.cc)
target_link_libraries(Timer_test baize boost_context pthread)