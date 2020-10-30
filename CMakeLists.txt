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

SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin) 
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

INCLUDE(cmake/mysql.cmake)

if(MYSQL_FOUND)
    list(APPEND LIBS ${MYSQL_LIBRARIES})
    list(APPEND LIBS_INCLUDE ${MYSQL_INCLUDE_DIR})
    list(APPEND LIB_DIRS ${MYSQL_LIB_DIR})
endif()


if(WIN32)
    #在windows中启用此项将使用IOCP模型否则使用SELECT
    option(IOCP_ENABLE "enable windows iocp." ON)
endif()


if(UNIX)
    #在unix中启用此项将使用EPOLL模型否则使用SELECT
    option(EPOLL_ENABLE "enable unix epoll." ON)
endif()


configure_file (
  "cmake/event_config.h.cmake"
  "${PROJECT_SOURCE_DIR}/src/common/event_config.h"
)

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


add_executable(chaos src/servers/gamesvr/main.cpp ${SOURCE_FILES})

if(UNIX)
    list(APPEND LIBS pthread rt dl)
endif()

target_link_libraries(chaos ${LIBS})