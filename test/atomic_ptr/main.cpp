
#include <cstdio>
#include <iostream>
#include <memory>

#include <mem/atomic_ptr.h>
#include <test_assert.h>


int main(
    int /*argc*/, char ** /*argv*/)
    try
{

    class Obj {};

    std::size_t ptr_address = 0;

    mem::atomic_ptr<Obj> ao = mem::make_atomic<Obj>();
    ptr_address = std::size_t(ao.get());
    test_assert(ptr_address == std::size_t(ao.non_atomic_get()));

    std::unique_ptr<Obj> ptr(ao.release());
    test_assert(std::size_t(ao.get()) == 0);
    test_assert(std::size_t(ao.non_atomic_get()) == 0);
    test_assert(std::size_t(ptr.get()) == ptr_address);

    ao.reset();
    test_assert(std::size_t(ao.get()) == 0);
    test_assert(std::size_t(ao.non_atomic_get()) == 0);

    ao.reset(new Obj);
    test_assert(std::size_t(ao.get()) != 0);
    test_assert(std::size_t(ao.non_atomic_get()) != 0);
    ptr_address = std::size_t(ao.get());
    test_assert(ptr_address == std::size_t(ao.non_atomic_get()));

    ptr.reset(ao.exchange(new Obj));
    test_assert(std::size_t(ao.get()) != 0);
    test_assert(std::size_t(ao.non_atomic_get()) != 0);
    test_assert(std::size_t(ao.get()) != ptr_address);
    test_assert(std::size_t(ao.non_atomic_get()) != ptr_address);
    test_assert(std::size_t(ptr.get()) == ptr_address);


    return 0;
}
catch (...)
{
    return 1;
}
