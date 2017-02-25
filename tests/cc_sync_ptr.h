
#ifndef __TESTS_CC_SYNC_PTR_H__
#define __TESTS_CC_SYNC_PTR_H__

#ifndef __CC_SYNC_PTR_H__
#include "cc/sync_ptr.h"
#endif


namespace tests
{
    /**
    * \brief Test sync_ptr synchronization.
    * \note Result: Synchronized on all expected path.
    */
    void cc_sync_ptr_synchro(void);

    /**
    * \brief Test sync_ptr release.
    * \note Result: Pointer gives ownership and is set to null.
    */
    void cc_sync_ptr_release(void);

    /**
    * \brief Test sync_ptr exchange.
    * \note Result: Pointer gives ownership and is set to target one.
    */
    void cc_sync_ptr_exchange(void);

    /**
    * \brief Test sync_ptr allocator.
    * \note Result: Pointer is allocated.
    */
    void cc_sync_ptr_allocator(void);

} // namespace tests

#endif // __TESTS_CC_SYNC_PTR_H__
