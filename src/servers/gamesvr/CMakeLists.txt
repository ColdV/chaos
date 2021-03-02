#需要编译的源文件路径
set(GAME_SERVER_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/aoi
	${CMAKE_CURRENT_SOURCE_DIR}/base
)

foreach(DIR ${GAME_SERVER_DIRS})
    aux_source_directory(${DIR} SOURCE_FILES)
endforeach()

add_executable(game_server ${SOURCE_FILES})
target_link_libraries(game_server chaos)