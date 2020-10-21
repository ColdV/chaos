macro(RESET_MYSQL_VARIABLES)
	unset(MYSQL_LIBRARIES)
	unset(MYSQL_LIBRARIES CACHE)

	unset(MYSQL_INCLUDE_DIR)
	unset(MYSQL_INCLUDE_DIR CACHE)
endmacro()


macro(COULD_NOT_FIND_MYSQL)
	if(NOT MYSQL_LIBRARIES)
		message(STATUS "could not find mysql library.")
	endif()

	if(NOT MYSQL_INCLUDE_DIR)
		message(STATUS "could not find mysql include.")
	endif()

	message(FATAL_ERROR "find mysql failed.")

	RESET_MYSQL_VARIABLES()
endmacro()


find_library(LIBMYSQL NAMES libmysql)
if(LIBMYSQL)
	list(APPEND MYSQL_LIBRARIES libmysql)
endif()

find_library(LIBMYSQLCLIENT NAMES mysqlclient)
if(LIBMYSQLCLIENT)
	list(APPEND MYSQL_LIBRARIES mysqlclient)
endif()

if(NOT LIBMYSQL AND NOT LIBMYSQLCLIENT)
	COULD_NOT_FIND_MYSQL()
endif()

find_path(MYSQL_INCLUDE_DIR NAMES mysql.h)

find_path(MYSQL_ROOT_DIR NAMES include/mysql.h)
set(MYSQL_LIB_DIR ${MYSQL_ROOT_DIR}/lib)

if(MYSQL_LIBRARIES AND MYSQL_INCLUDE_DIR)
	set(MYSQL_FOUND TRUE)
else()
	set(MYSQL_FOUND FALSE)
	COULD_NOT_FIND_MYSQL()
endif()