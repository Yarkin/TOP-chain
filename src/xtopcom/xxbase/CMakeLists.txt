cmake_minimum_required(VERSION 3.8)

aux_source_directory(./src SRCS)
add_library(xxbase ${SRCS})

if (ADDRESS_SANITIZER)
   target_link_libraries(xxbase asan)
endif()

if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
    target_link_libraries(xxbase xbase uv  mbedtls mbedcrypto mbedx509 intel_aes64 rt atomic lua pthread dl)
elseif(CMAKE_SYSTEM_NAME STREQUAL "Darwin")
    target_link_libraries(xxbase xbase uv dl pthread)
endif()
