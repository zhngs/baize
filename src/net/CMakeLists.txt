aux_source_directory(. net_SRCS)

add_library(baize_net ${net_SRCS})

add_subdirectory(test)
