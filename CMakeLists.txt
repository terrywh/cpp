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

file(GLOB_RECURSE XBOND_SOURCES RELATIVE ${PROJECT_SOURCE_DIR} "src/*.cpp")
add_library(xbond STATIC ${XBOND_SOURCES})
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
install(TARGETS xbond
    ARCHIVE DESTINATION lib)
install(DIRECTORY include/xbond DESTINATION include)
install(DIRECTORY vendor/gsl/include/gsl DESTINATION include)
install(DIRECTORY vendor/date/include/date DESTINATION include)

file(GLOB_RECURSE XBOND_TEST_SOURCES RELATIVE ${PROJECT_SOURCE_DIR} "test/*.cpp")
add_executable(xbond-test ${XBOND_TEST_SOURCES})
# add_dependencies(xbond-test xbond)
target_link_libraries(xbond-test xbond)
