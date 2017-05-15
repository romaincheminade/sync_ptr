
macro( 
	add_new_test 
	PROJECT_NAME_IN 
	SRC_FOLDER_IN)
		
	# CMake Configuration.
	#######################################################################
	# Find includes in corresponding build directories
	set( CMAKE_INCLUDE_CURRENT_DIR ON )

	# Project.
	#######################################################################

	# Project name.
	set( PROJECT_NAME ${PROJECT_NAME_IN} )
	project( ${PROJECT_NAME} )

	# Source files.
	file(GLOB C_FILES ${SRC_FOLDER_IN}/*.cpp )
	file(GLOB H_FILES ${SRC_FOLDER_IN}/*.h )
	set(SRC_FILES 
		${C_FILES}
		${H_FILES} )
		 
	set_source_files_properties( ${SRC_FILES} PROPERTIES GENERATED true )

	# Include directories
	#######################################################################	
	include_directories(
		${BASE_SOURCE_PATH}
		${SRC_FOLDER_IN} 
		${INCLUDE_FOLDERS_IN} )

	# Create executable.
	#######################################################################
	set( exe_name ${PROJECT_NAME} )
	add_executable( ${exe_name} ${SRC_FILES} )
	
	# Libraries.
	####################################################################### 
	link_directories( ${CMAKE_LIBRARY_OUTPUT_DIRECTORY} )

	target_link_libraries( ${exe_name} ${LIBRARIES_IN} )
	# Executable properties.
	set_target_properties( ${exe_name} PROPERTIES OUTPUT_NAME ${PROJECT_NAME} )
	# Add target to folder.
	set_target_properties( ${exe_name} PROPERTIES FOLDER ${PROJECT_FOLDER_IN} )

	# Test.
	#######################################################################
	add_test( 
		NAME ${PROJECT_NAME}_Test
		COMMAND ${exe_name} )		

endmacro()

