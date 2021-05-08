cmake_minimum_required(VERSION 3.8)

aux_source_directory(./ xmem_test_test_src)
add_executable(xmem_test ${xmem_test_test_src})
target_link_libraries(xmem_test PRIVATE xxbase)
