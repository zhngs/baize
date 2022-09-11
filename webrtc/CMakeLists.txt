add_subdirectory(example)

# aux_source_directory(. webrtc_SRCS)
# add_library(baize_webrtc ${webrtc_SRCS})

aux_source_directory(./sdp sdp_SRCS)
aux_source_directory(./ice ice_SRCS)
aux_source_directory(./dtls dtls_SRCS)
aux_source_directory(./pc pc_SRCS)
add_library(baize_webrtc ${sdp_SRCS} ${ice_SRCS} ${dtls_SRCS} ${pc_SRCS})