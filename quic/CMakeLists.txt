add_executable(QuicDiscard QuicDiscard.cc QuicListener.cc QuicConnection.cc QuicConfig.cc)
target_link_libraries(QuicDiscard quiche baize pthread boost_context)