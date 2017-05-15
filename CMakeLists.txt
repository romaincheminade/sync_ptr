
cmake_minimum_required(VERSION 3.5)

project( sync_ptr )

set( BASE_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include( ${BASE_SOURCE_PATH}/CMake/ProjectFlags.cmake )

# Source Files.
#################################################
set(SRCS
    ${BASE_SOURCE_PATH}/src/mem/linked_ptr.h
    ${BASE_SOURCE_PATH}/src/mem/sync_ptr.h
    ${BASE_SOURCE_PATH}/src/mem/sync_ptr_policy.h
    )
source_group( "Memory" FILES ${SRCS} )
set( SRC_FILES ${SRC_FILES} ${SRCS} )


# Create Library.
#################################################
add_library(${PROJECT_NAME} STATIC ${SRC_FILES})
set_target_properties( ${PROJECT_NAME} PROPERTIES LINKER_LANGUAGE CXX)


# Tests.
#################################################
option( BUILD_TESTS "Add test project(s) to the build." ON )
if( BUILD_TESTS )
	enable_testing()
	add_subdirectory( test )

endif()
