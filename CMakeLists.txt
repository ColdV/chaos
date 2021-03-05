cmake_minimum_required(VERSION 3.1.2 FATAL_ERROR)

project(chaos)

set(CMAKE_BUILD_TYPE Debug)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release
        CACHE STRING "Set build type to Debug or Release (default Release)" FORCE)
endif()


SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)
SET(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)
SET(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_SOURCE_DIR}/cmake)

if(${CMAKE_BUILD_TYPE} STREQUAL "Debug")
	set(PROJECT_LIBS_OUTPUT_DIR ${LIBRARY_OUTPUT_PATH}/Debug)
else()
	set(PROJECT_LIBS_OUTPUT_DIR ${LIBRARY_OUTPUT_PATH}/Release)
endif()

#SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_DEBUG ${PROJECT_SOURCE_DIR}/libs)
#SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY_RELEASE ${PROJECT_SOURCE_DIR}/libs)

#创建libs和include目录以便引用第三方库
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/deps)
file(MAKE_DIRECTORY ${CMAKE_SOURCE_DIR}/libs)
set(PROJECT_DEPS_DIR ${CMAKE_SOURCE_DIR}/deps)
set(PROJECT_LIBS_DIR ${CMAKE_SOURCE_DIR}/libs)


#FindMySQL
INCLUDE(cmake/mysql.cmake)

#FindRedis
find_package(HiRedis)


if(MYSQL_FOUND)
    message(STATUS "mysql found.")
    list(APPEND LIBS ${MYSQL_LIBRARIES})
    list(APPEND LIBS_INCLUDE ${MYSQL_INCLUDE_DIR})
    list(APPEND LIB_DIRS ${MYSQL_LIB_DIR})
    list(APPEND LIBSRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/common/db/dbmysql)
    option(DB_MYSQL_ENABLE "enable mysql." ON)
endif()

if(HiRedis_FOUND)
  message(STATUS "hiredis found.")
  list(APPEND LIBSRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/common/db/dbredis)
  list(APPEND LIBS ${HiRedis_LIBRARY})
  list(APPEND LIBS_INCLUDE ${HiRedis_INCLUDE_DIR})
  option(DB_REDIS_ENABLE "enable redis." ON)
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


if(WIN32)
	set(CMAKE_CXX_FLAGS_DEBUG "/Od /MDd")
	set(CMAKE_CXX_FLAGS_RELEASE "/O2 /MD")
endif()

if(UNIX)
    add_definitions(-std=c++11)
	set(CMAKE_CXX_FLAGS_DEBUG "-O0 -g")
	set(CMAKE_CXX_FLAGS_RELEASE "-O2 -DNDEBUG")
endif()


include_directories(src/common)
include_directories(${LIBS_INCLUDE})
link_directories(${LIB_DIRS})



if(WIN32)
    list(APPEND LIBS ws2_32.lib)
endif()

if(UNIX)
    list(APPEND LIBS pthread rt dl)
endif()


add_subdirectory(src/common)

add_subdirectory(src/servers/gamesvr)
add_subdirectory(src/client)