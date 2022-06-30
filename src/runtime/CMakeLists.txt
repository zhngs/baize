aux_source_directory(. runtime_SRCS)

add_library(baize_runtime ${runtime_SRCS})

add_subdirectory(test)
