link_directories(./lib)

add_executable(QuicDiscard QuicDiscard.cc QuicConnection.cc QuicConfig.cc QuicListener.cc)
target_link_libraries(QuicDiscard baize pthread boost_context quiche dl -pedantic -fsanitize=address)