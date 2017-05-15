
#ifndef __TESTS_MEM_LINKED_PTR_H__
#define __TESTS_MEM_LINKED_PTR_H__


namespace tests
{

    /**
    * \brief Test linked_ptr synchronization to sync_ptr.
    * \note Result: Synchronized on all expected path.
    */
    void mem_linked_ptr_synchro(void);
    
    /**
    * \brief Test linked_ptr orphan state.
    * \note Result: When sync_ptr gets reclaimed, linked_ptr is orphan.
    */
    void mem_linked_ptr_orphan(void);

} // namespace tests

#endif // __TESTS_MEM_LINKED_PTR_H__
