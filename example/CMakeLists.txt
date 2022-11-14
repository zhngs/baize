add_executable(tcp_echo tcp_echo.cc)
target_link_libraries(tcp_echo baize boost_context pthread)

add_executable(tcp_discard tcp_discard.cc)
target_link_libraries(tcp_discard baize boost_context pthread)

add_executable(udp_discard udp_discard.cc)
target_link_libraries(udp_discard baize boost_context pthread)

add_executable(baize_file_transfer file_transfer.cc)
target_link_libraries(baize_file_transfer baize boost_context.a pthread)
