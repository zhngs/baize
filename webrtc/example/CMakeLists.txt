add_executable(webrtc_sfu_server webrtc_sfu_server.cc)
target_link_libraries(webrtc_sfu_server baize_webrtc baize_http baize srtp2 pthread boost_context ssl crypto dl)