
# Source Files.
#################################################
set(SRCS
    ${BASE_SOURCE_PATH}/mem/allocation_policy.h
    ${BASE_SOURCE_PATH}/mem/atomic_ptr.h
    ${BASE_SOURCE_PATH}/mem/linked_ptr.h
    ${BASE_SOURCE_PATH}/mem/sync_ptr.h
    )
source_group( "Memory" FILES ${SRCS} )
set( SRC_FILES ${SRC_FILES} ${SRCS} )

