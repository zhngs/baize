aux_source_directory(. time_SRCS)

add_library(baize_time ${time_SRCS})

add_subdirectory(test)