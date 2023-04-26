add_subdirectory(test)

aux_source_directory(. kernel_SRCS)
add_library(baize_kernel ${kernel_SRCS})