
#ifndef __TESTS_MEM_SYNC_PTR_H__
#define __TESTS_MEM_SYNC_PTR_H__

#ifndef __MEMORY_SYNC_PTR_H__
#include "mem/sync_ptr.h"
#endif


namespace tests
{
    /**
    * \brief Test sync_ptr synchronization.
    * \note Result: Synchronized on all expected path.
    */
    void mem_sync_ptr_synchro(void);

    /**
    * \brief Test sync_ptr release.
    * \note Result: Pointer gives ownership and is set to null.
    */
    void mem_sync_ptr_release(void);

    /**
    * \brief Test sync_ptr exchange.
    * \note Result: Pointer gives ownership and is set to target one.
    */
    void mem_sync_ptr_exchange(void);

    /**
    * \brief Test sync_ptr allocator.
    * \note Result: Pointer is allocated.
    */
    void mem_sync_ptr_allocator(void);

} // namespace tests

#endif // __TESTS_MEM_SYNC_PTR_H__
