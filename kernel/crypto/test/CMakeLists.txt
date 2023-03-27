add_executable(tls_echo tls_echo.cc)
target_link_libraries(tls_echo baize boost_context ssl crypto dl pthread)

add_executable(util_test util_test.cc)
target_link_libraries(util_test baize boost_context ssl crypto dl pthread)