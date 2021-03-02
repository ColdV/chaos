list(APPEND LIBSRC_DIR
    ${CMAKE_CURRENT_SOURCE_DIR}
	${CMAKE_CURRENT_SOURCE_DIR}/db
    ${CMAKE_CURRENT_SOURCE_DIR}/log
    ${CMAKE_CURRENT_SOURCE_DIR}/net
    ${CMAKE_CURRENT_SOURCE_DIR}/package
    ${CMAKE_CURRENT_SOURCE_DIR}/template
    ${CMAKE_CURRENT_SOURCE_DIR}/thread
	)
	
foreach(DIR ${LIBSRC_DIR})
    aux_source_directory(${DIR} LIB_SOURCE_FILES)
endforeach()

add_library(chaos ${LIB_SOURCE_FILES})
target_link_libraries(chaos ${LIBS})

install(TARGETS chaos DESTINATION lib)