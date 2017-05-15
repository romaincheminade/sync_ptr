
// Main header.
#include "mem_sync_ptr.h"

#include <memory>

#include <mem/sync_ptr.h>
#include <test_assert.h>


void tests::mem_sync_ptr_synchro(void)
{
    class Obj
    {};
    mem::sync_ptr<Obj>		obj;

    mem::sync_ptr<Obj>		obj1 = mem::make_sync<Obj>();
    mem::sync_ptr<Obj>		obj2(obj1);
    mem::sync_ptr<Obj>		obj3 = mem::make_sync<Obj>();
    test_assert(obj1);
    test_assert(obj2);
    test_assert(obj3);

    test_assert(obj1.get() == obj2.get());
    test_assert(obj1.count() == obj2.count());
    test_assert(obj1 != obj3);

    obj1.reset(new Obj());
    test_assert(obj1 == obj2);
    test_assert(obj1.count() == obj2.count());

    obj1 = obj3;
    test_assert(obj1 == obj3);
    test_assert(obj1.count() == obj3.count());
    test_assert(obj1 != obj2);

    obj1.reset(new Obj());
    test_assert(obj1.get() != nullptr);
    test_assert(obj1 == obj3);
    test_assert(obj1.count() == obj3.count());
}

void tests::mem_sync_ptr_release(void)
{
    struct Obj
    {};

    mem::sync_ptr<Obj> ptr = mem::make_sync<Obj>();
    test_assert(ptr);

    Obj * raw = ptr.release();
    test_assert(raw);
    test_assert(!ptr);

    delete raw;
}

void tests::mem_sync_ptr_exchange(void)
{
    struct Obj
    {};

    mem::sync_ptr<Obj> ptr = mem::make_sync<Obj>();
    test_assert(ptr);

    Obj * ptr_add = ptr.get();
    Obj * raw = new Obj();
    std::unique_ptr<Obj> ptr_xc(ptr.exchange(raw));

    test_assert(ptr_xc);
    test_assert(ptr_xc.get() == ptr_add);

    test_assert(ptr);
    test_assert(ptr.get() == raw);
}



static bool test_allocator_called = false;

template<class TType>
struct test_allocator
{
    constexpr test_allocator(
        void)
        noexcept = default;

    template<class ...TArg>
    TType * allocate(
        TArg && ...p_args)
        const
    {
        static_assert(0 < sizeof(TType),
            "can't allocate an incomplete type");

        test_allocator_called = true;
        return new TType(std::forward<TArg>(p_args)...);
    }
};

void tests::mem_sync_ptr_allocator(void)
{
    class Obj
    {};

    // User defined template.

    test_allocator_called = false;
    {
        test_allocator<Obj> allocator_;

        auto obj(
            mem::make_sync_with_allocator<
                Obj> (
                    allocator_));
    }
    test_assert(test_allocator_called);
}
