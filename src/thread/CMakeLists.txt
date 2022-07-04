aux_source_directory(. thread_SRCS)

add_library(baize_thread ${thread_SRCS})

add_subdirectory(test)
