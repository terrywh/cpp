cmake_minimum_required(VERSION 3.10)

project(xbond)
set(PROJECT_VERSION "0.1.0")
set(CMAKE_CXX_STANDARD 17)

if (DEFINED ENV{VENDOR_BOOST})
    set(VENDOR_BOOST $ENV{VENDOR_BOOST})
elseif (NOT DEFINED VENDOR_BOOST)
    set(VENDOR_BOOST /data/vendor/boost-1.75)
endif()

if (DEFINED ENV{VENDOR_OPENSSL})
    set(VENDOR_OPENSSL $ENV{VENDOR_OPENSSL})
elseif (NOT DEFINED VENDOR_OPENSSL)
    set(VENDOR_OPENSSL /data/vendor/openssl-1.1)
endif()

add_library(xbond STATIC
    src/log/logger.cpp
    src/net/detail/netlink_route.cpp
    src/snowflake.cpp)
target_include_directories(xbond
    PUBLIC include
    PUBLIC vendor/gsl/include
    PUBLIC vendor/date/include
)
target_include_directories(xbond SYSTEM
    PUBLIC ${VENDOR_BOOST}/include
    PUBLIC ${VENDOR_OPENSSL}/include)
target_link_directories(xbond
    PUBLIC ${VENDOR_BOOST}/lib
    PUBLIC ${VENDOR_OPENSSL}/lib)
target_link_libraries(xbond
    boost_json boost_filesystem boost_context boost_system
    ssl crypto
    pthread)

add_executable(xbond-test
    test/core_coroutine.cpp
    test/core.cpp
    test/encoding.cpp
    test/hash.cpp
    test/log.cpp
    test/main.cpp
    test/net.cpp
    test/strconv.cpp
    test/time.cpp)
# add_dependencies(xbond-test xbond)
target_link_libraries(xbond-test xbond)
