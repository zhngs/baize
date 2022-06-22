aux_source_directory(. log_SRCS)

add_library(baize_log ${log_SRCS})

add_subdirectory(test)