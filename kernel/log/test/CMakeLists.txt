add_executable(logger_test logger_test.cc)
target_link_libraries(logger_test baize pthread boost_context)

add_executable(async_logging_test async_logging_test.cc)
target_link_libraries(async_logging_test baize pthread boost_context)