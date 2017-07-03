
#include "mem_sync_ptr.h"

#include <mem/atomic_ptr.h>
#include <mem/single_ptr.h>

#include <cstdio>
#include <iostream>


int main(
    int /*argc*/, char ** /*argv*/)
    try
{
    tests::mem_sync_ptr_synchro();
    tests::mem_sync_ptr_release();
    tests::mem_sync_ptr_exchange();
    tests::mem_sync_ptr_allocator();


    class Obj {};

//    mem::atomic_ptr<Obj> ao;
//    std::cout << std::size_t(ao.get()) << std::endl;
//    std::cout << std::size_t(ao.non_atomic_get()) << std::endl;

     auto o = mem::single_ptr<Obj>::instance();
     auto s = sizeof(o);
     std::cout << s << std::endl;

    return 0;
}
catch (...)
{
    return 1;
}
