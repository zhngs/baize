add_subdirectory(example)

aux_source_directory(. http_SRCS)
add_library(baize_http ${http_SRCS})