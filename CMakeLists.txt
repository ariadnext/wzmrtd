
cmake_minimum_required( VERSION 2.6 FATAL_ERROR )
project( wzmrtd C )

#Compute exact revision from git if possible
list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmakemodules/")
include(GetGitRevisionDescription)
# rune git describe "--tags"
git_describe(VERSION_STRING "--tags" "--always")

#if not possible use version number we ship instead
IF(NOT VERSION_STRING)
	set(VERSION_STRING "v0.81")
ENDIF(NOT VERSION_STRING)

include( FindPkgConfig )

pkg_search_module( LIBPCSCLITE REQUIRED libpcsclite )


# configure a header file to pass some of the CMake settings
# to the source code
configure_file (
  "${PROJECT_SOURCE_DIR}/Config.h.in"
  "${PROJECT_BINARY_DIR}/Config.h"
  )

include_directories(
    ${CMAKE_SOURCE_DIR}/inc
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}
    ${LIBPCSCLITE_INCLUDE_DIRS}
)

link_directories ( 
    ${LIBPCSCLITE_LIBRARY_DIRS} 
)

if ( ${CMAKE_BUILD_TYPE} MATCHES Debug )
    pkg_search_module( CPPUNIT REQUIRED cppunit )
    include_directories( ${CPPUNIT_INCLUDE_DIRS} ${CMAKE_SOURCE_DIR}/tests )
    link_directories ( ${CPPUNIT_LIBRARY_DIRS} )
endif ( ${CMAKE_BUILD_TYPE} MATCHES Debug )

if ( ${CMAKE_BUILD_TYPE} MATCHES Release )
    add_definitions ( -DNDEBUG )
endif ( ${CMAKE_BUILD_TYPE} MATCHES Release )

set ( CMAKE_C_FLAGS_RELWITHDEBINFO "-g -O2" )
set ( CMAKE_C_FLAGS_DEBUG "-g -O0" )
set ( CMAKE_C_FLAGS_RELEASE "-O2" )
set ( CMAKE_C_FLAGS "-Wall -W" )

add_definitions ( -DDISABLE_SPROX )

add_library(
    wzmrtd
    SHARED
    src/des.c
    src/mrtd_bac.c
    src/mrtd_cbeff.c
    src/mrtd_core.c
    src/mrtd_entries.c
    src/mrtd_errors.c
    src/mrtd_icc.c
    src/mrtd_mrz.c
    src/mrtd_out_raw.c
    src/mrtd_out_xml.c
    src/mrtd_parse.c
    src/pcsc_reader.c
    src/random.c
    src/sha1.c
    src/tlv.c
)

target_link_libraries (
    wzmrtd
    pcsclite
)

#configure_file (
#    wzmrtd.pc.in
#    wzmrtd.pc
#)

install(TARGETS wzmrtd LIBRARY DESTINATION lib)
install(FILES
    include/config.h
    include/ldstags.h
    include/springprox.h
    include/wzmrtd.h
    DESTINATION include/wzmrtd
)

add_executable(
    wzmrtd-tool
    src/main.c
)

target_link_libraries (
    wzmrtd-tool
    wzmrtd
) 