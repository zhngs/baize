add_executable(tls_echo tls_echo.cc)
target_link_libraries(tls_echo baize pthread boost_context ssl crypto dl)

add_executable(util_test util_test.cc)
target_link_libraries(util_test baize pthread boost_context ssl crypto dl)