
cmake_minimum_required(VERSION 3.5)

project( sync_ptr )

# Concurrency.
set(SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/cc/sync_ptr.h
    )
source_group( "Concurrency" FILES ${SRCS} )
set( SOURCE_FILES ${SOURCE_FILES} ${SRCS} )

# Memory.
set(SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mem/sync_ptr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/src/mem/sync_ptr_policy.h
    )
source_group( "Memory" FILES ${SRCS} )
set( SOURCE_FILES ${SOURCE_FILES} ${SRCS} )

# Tests.
set(SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/cc_sync_ptr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/cc_sync_ptr.h
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/mem_sync_ptr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/tests/mem_sync_ptr.h
    )
source_group( "Tests" FILES ${SRCS} )
set( SOURCE_FILES ${SOURCE_FILES} ${SRCS} )

# Main.
set(SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp)
set( SOURCE_FILES ${SOURCE_FILES} ${SRCS} )

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/src)
add_executable( sync_ptr ${SOURCE_FILES} )
