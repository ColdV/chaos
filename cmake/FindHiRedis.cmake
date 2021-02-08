
#===========Find hiredis head files===========#
find_path(HiRedis_INCLUDE_DIR hiredis/hiredis.h)
if(NOT HIREDIS_INCLUDE_DIR)
    find_path(HiRedis_INCLUDE_DIR hiredis/hiredis.h PATH ${PROJECT_DEPS_DIR})
endif()


#===========Find hiredis libary===========#
find_library(HiRedis_LIBRARY hiredis)
if(NOT HiRedis_LIBRARY)
    find_library(HiRedis_LIBRARY hiredis PATH ${PROJECT_LIBS_DIR})
endif()


if(HiRedis_INCLUDE_DIR AND HiRedis_LIBRARY)
    set(HiRedis_FOUND TRUE)
endif()