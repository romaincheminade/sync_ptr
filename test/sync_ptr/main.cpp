
#include "mem_sync_ptr.h"

#include <mem/atomic_ptr.h>
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


    class Obj {
    public:
        std::size_t a_;
        std::size_t b_;
        std::size_t c_;
        std::size_t d_;
    };

    std::unique_ptr<Obj> u = std::make_unique<Obj>();
    std::cout << sizeof(u) << std::endl;
    std::cout << sizeof(u.get()) << std::endl;

    mem::atomic_ptr<Obj> a = mem::make_atomic<Obj>();
    std::cout << sizeof(a) << std::endl;
    std::cout << sizeof(a.get()) << std::endl;

    mem::single_ptr<Obj> s;
    std::cout << sizeof(s) << std::endl;
    std::cout << sizeof(s.get()) << std::endl;

    return 0;
}
catch (...)
{
    return 1;
}
