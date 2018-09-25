if( NOT TARGET Cinder- )
	get_filename_component( Cinder-_SOURCE_PATH "${CMAKE_CURRENT_LIST_DIR}/../../src" ABSOLUTE )
	get_filename_component( CINDER_PATH "${CMAKE_CURRENT_LIST_DIR}/../../../.." ABSOLUTE )

	list( APPEND Cinder-_SOURCES
		${Cinder-_SOURCE_PATH}/Assets.h
		${Cinder-_SOURCE_PATH}/Assets.cpp
		${Cinder-_SOURCE_PATH}/Compute.h
		${Cinder-_SOURCE_PATH}/Compute.cpp
		${Cinder-_SOURCE_PATH}/Environment.h
		${Cinder-_SOURCE_PATH}/Environment.cpp
		${Cinder-_SOURCE_PATH}/EnvironmentFilter.h
		${Cinder-_SOURCE_PATH}/EnvironmentFilter.cpp
	)
	
	add_library( Cinder- ${Cinder-_SOURCES} )

	target_include_directories( Cinder- PUBLIC "${Cinder-_SOURCE_PATH}" )
	target_include_directories( Cinder- SYSTEM BEFORE PUBLIC "${CINDER_PATH}/include" )

	if( NOT TARGET cinder )
		    include( "${CINDER_PATH}/proj/cmake/configure.cmake" )
		    find_package( cinder REQUIRED PATHS
		        "${CINDER_PATH}/${CINDER_LIB_DIRECTORY}"
		        "$ENV{CINDER_PATH}/${CINDER_LIB_DIRECTORY}" )
	endif()
	target_link_libraries( Cinder- PRIVATE cinder )
	
endif()



