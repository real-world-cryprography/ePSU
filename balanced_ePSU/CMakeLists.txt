cmake_minimum_required(VERSION 3.15)
project(balanced_ePSU)


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

include_directories(pnmcrg)
include_directories(epsu)
file(GLOB_RECURSE SRCS
    pnmcrg/*.cpp
    epsu/*.cpp
)


set(CMAKE_BUILD_TYPE Release)


add_executable(test_balanced_epsu test/test_balanced_epsu.cpp ${SRCS})
target_compile_options(test_balanced_epsu PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(test_balanced_epsu visa::volePSI ${OPENSSL_LIBRARIES} OpenMP::OpenMP_CXX)

# for test
add_executable(test_necrg test/test_necrg.cpp  ${SRCS})
target_compile_options(test_necrg PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(test_necrg visa::volePSI ${OPENSSL_LIBRARIES} OpenMP::OpenMP_CXX)


add_executable(test_pecrg test/test_pecrg.cpp ${SRCS})
target_compile_options(test_pecrg PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(test_pecrg visa::volePSI ${OPENSSL_LIBRARIES} OpenMP::OpenMP_CXX)


add_executable(test_pmcrg test/test_pmcrg.cpp ${SRCS})
target_compile_options(test_pmcrg PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(test_pmcrg visa::volePSI ${OPENSSL_LIBRARIES} OpenMP::OpenMP_CXX)


add_executable(test_pnmcrg test/test_pnmcrg.cpp ${SRCS})
target_compile_options(test_pnmcrg PUBLIC $<$<COMPILE_LANGUAGE:CXX>:-std=c++17> -lpthread -O3)
target_link_libraries(test_pnmcrg visa::volePSI ${OPENSSL_LIBRARIES} OpenMP::OpenMP_CXX)



