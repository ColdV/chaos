cmake_minimum_required(VERSION 3.1.2 FATAL_ERROR)

project(chaos)

set(CMAKE_BUILD_TYPE Debug)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release
        CACHE STRING "Set build type to Debug or Release (default Release)" FORCE)
endif()

set(LIBSRC_DIR
    src/common
    #src/common/db/dbmysql
    src/common/log
    src/common/net
    src/common/package
    src/common/template
    src/common/thread)

include_directories(src/common)


SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

INCLUDE(cmake/mysql.cmake)

if(MYSQL_FOUND)
    list(APPEND LIBS ${MYSQL_LIBRARIES})
    list(APPEND LIBS_INCLUDE ${MYSQL_INCLUDE_DIR})
    list(APPEND LIB_DIRS ${MYSQL_LIB_DIR})
endif()

#find_package(MySQL REQUIRED)
#find_library(MYSQL_LIB NAMES libmysql)
#message(STATUS ${LIBS})


#if(UNIX)
#    set(CXX_FLAGS
#     -g
#     # -DVALGRIND
#     -DCHECK_PTHREAD_RETURN_VALUE
#     -D_FILE_OFFSET_BITS=64
#     -Wall
#     -Wextra
#     -Werror
#     -Wconversion
#     -Wno-unused-parameter
#     -Wold-style-cast
#     -Woverloaded-virtual
#     -Wpointer-arith
#     -Wshadow
#     -Wwrite-strings
#     -march=native
#     # -MMD
#     -std=c++11
#     -rdynamic
#     )
#
#     #string(REPLACE ";" " " CMAKE_CXX_FLAGS "${CXX_FLAGS}")
#endif()

if(UNIX)
    add_definitions(-std=c++11 -g)

    set(CMAKE_CXX_FLAGS_DEBUG "-O0")
    set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()

foreach(DIR ${LIBSRC_DIR})
    aux_source_directory(${DIR} SOURCE_FILES)
endforeach()

include_directories(${LIBS_INCLUDE})
link_directories(${LIB_DIRS})


#include(CheckTypeSize)
#include(CheckFunctionExistsEx)
add_executable(chaos src/servers/gamesvr/main.cpp ${SOURCE_FILES})

if(UNIX)
    list(APPEND LIBS pthread rt dl)
endif()

target_link_libraries(chaos ${LIBS})