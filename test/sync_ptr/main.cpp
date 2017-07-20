
#include "mem_sync_ptr.h"

#include <mem/atomic_ptr.h>
#include <mem/single_ptr.h>
#include <mem/stack_ptr.h>

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

    std::atomic<Obj*> std_a;
    bool is_lock_free = std_a.is_lock_free();
    std::cout << "is lock free: " << is_lock_free << std::endl;
    std::cout << sizeof(std_a) << '\n' << std::endl;

    std::unique_ptr<Obj> u = std::make_unique<Obj>();
    std::cout << sizeof(u) << std::endl;
    std::cout << sizeof(u.get()) << '\n' << std::endl;

    mem::atomic_ptr<Obj> a = mem::make_atomic<Obj>();
    std::cout << sizeof(a) << std::endl;
    std::cout << sizeof(a.get()) << '\n' << std::endl;

    mem::single_ptr<Obj> s;
    std::cout << sizeof(s) << std::endl;
    std::cout << sizeof(s.get()) << '\n' << std::endl;


    class Widget {
    public:
        std::int32_t i_;

        explicit Widget(std::int32_t p_i) noexcept
            : i_{ p_i }
        {}
    };
    mem::stack_ptr<Widget> sp(1);
    std::cout << sizeof(sp) << std::endl;
    std::cout << sp.get().i_ << std::endl;
    std::cout << sp->i_ << std::endl;
    sp.reset(2);
    std::cout << sp->i_ << std::endl;
    std::cout << std::size_t(&sp) << '\n' << std::endl;

    return 0;
}
catch (...)
{
    return 1;
}
