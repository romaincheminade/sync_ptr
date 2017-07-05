
#include "mem_sync_ptr.h"

#include <mem/single_ptr.h>

#include <cstdio>
#include <iostream>
#include <memory>


int main(
    int /*argc*/, char ** /*argv*/)
    try
{
    tests::mem_sync_ptr_synchro();
    tests::mem_sync_ptr_release();
    tests::mem_sync_ptr_exchange();
    tests::mem_sync_ptr_allocator();


    class Obj {};

    mem::single_ptr<Obj> o;
    std::cout << sizeof(o) << std::endl;
    std::cout << sizeof(o.get()) << std::endl;

    return 0;
}
catch (...)
{
    return 1;
}
