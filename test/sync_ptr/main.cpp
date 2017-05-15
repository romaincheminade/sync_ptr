
#include "mem_sync_ptr.h"


int main(
    int /*argc*/, char ** /*argv*/)
    try
{
    tests::mem_sync_ptr_synchro();
    tests::mem_sync_ptr_release();
    tests::mem_sync_ptr_exchange();
    tests::mem_sync_ptr_allocator();

    return 0;
}
catch (...)
{
    return 1;
}
