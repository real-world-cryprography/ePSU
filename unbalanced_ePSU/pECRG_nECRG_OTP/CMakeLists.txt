cmake_minimum_required(VERSION 3.15)
project(pECRGnECRGOTP)


FIND_PACKAGE(OpenMP REQUIRED)


add_definitions(-w)


SET(CMAKE_CXX_LINK_EXECUTABLE "${CMAKE_CXX_LINK_EXECUTABLE} -ldl")

set(OPENSSL_ROOT_DIR /usr/local/openssl)
set(OPENSSL_CRYPTO_LIBRARY /usr/local/openssl/lib64/libcrypto.a)
set(OPENSSL_SSL_LIBRARY /usr/local/openssl/lib64/libssl.a)

find_package(OpenSSL REQUIRED)

include_directories(${OPENSSL_INCLUDE_DIR})
link_directories(${OPENSSL_LIBRARIES})

message(STATUS "OpenSSL include directory:"${OPENSSL_INCLUDE_DIR})
message(STATUS "OpenSSL libraries:"${OPENSSL_LIBRARIES})

find_package(volePSI REQUIRED HINTS "./libvolepsi")

include_directories(pnecrg)
include_directories(pecrg_necrg_otp)

file(GLOB_RECURSE SRCS
    pnecrg/*.cpp
    pecrg_necrg_otp/*.cpp
)


set(CMAKE_BUILD_TYPE Release)


add_executable(test_pecrg_necrg_otp test/test_pecrg_necrg_otp.cpp ${SRCS})
target_compile_options(test_pecrg_necrg_otp PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(test_pecrg_necrg_otp  visa::volePSI ${OPENSSL_LIBRARIES} OpenMP::OpenMP_CXX)


add_executable(test_pecrg test/test_pecrg.cpp ${SRCS})
target_compile_options(test_pecrg PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(test_pecrg visa::volePSI ${OPENSSL_LIBRARIES} OpenMP::OpenMP_CXX)

add_executable(test_pnecrg test/test_pnecrg.cpp ${SRCS})
target_compile_options(test_pnecrg PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(test_pnecrg visa::volePSI ${OPENSSL_LIBRARIES} OpenMP::OpenMP_CXX)











