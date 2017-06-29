
cmake_minimum_required(VERSION 3.5)

project( sync_ptr )

set( BASE_SOURCE_PATH ${CMAKE_CURRENT_SOURCE_DIR})
include( ${BASE_SOURCE_PATH}/CMake/ProjectFlags.cmake )

# Source Files.
#################################################
set( SRC_FILES )
include( ${BASE_SOURCE_PATH}/mem/CMakeLists.txt )


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
