#需要编译的源文件路径
set(CLIENT_DIRS
	${CMAKE_CURRENT_SOURCE_DIR}
)

foreach(DIR ${CLIENT_DIRS})
    aux_source_directory(${DIR} SOURCE_FILES)
endforeach()

add_executable(client ${SOURCE_FILES})
target_link_libraries(client chaos)