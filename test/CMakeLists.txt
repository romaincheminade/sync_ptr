
	
# Helper.
#################################################
include( AddTest.cmake )
	

# Additional Includes.
#################################################
set( 
	INCLUDE_FOLDERS_IN 
	${BASE_SOURCE_PATH}
	${BASE_SOURCE_PATH}/test
	) # Add any relevant include path here.  


# # Additional Libraries.
#################################################
# set( 
	# LIBRARIES_IN	
	# ) # Add any relevant library here.
	

# Project Folder.
#################################################
set(
	PROJECT_FOLDER_IN
	Tests)


# Examples.	
#################################################
add_new_test(
    Linked 
    ${BASE_SOURCE_PATH}/test/linked_ptr )
	
add_new_test(
    Sync 
    ${BASE_SOURCE_PATH}/test/sync_ptr )
	
