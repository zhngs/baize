add_executable(discard_udp discard_udp.cc)
target_link_libraries(discard_udp baize boost_context pthread)

# 用来简单实用ssl的例子
# add_executable(ssl_discard ssl_discard.cc)
# target_link_libraries(ssl_discard ssl crypto baize pthread boost_context)