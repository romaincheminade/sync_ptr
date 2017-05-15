
// Main header.
#include "mem_sync_ptr.h"

#include <cassert>
#include <memory>


void tests::mem_sync_ptr_synchro(void)
{
    class Obj
    {};
    mem::sync_ptr<Obj>		obj;

    mem::sync_ptr<Obj>		obj1 = mem::make_sync<Obj>();
    mem::sync_ptr<Obj>		obj2(obj1);
    mem::sync_ptr<Obj>		obj3 = mem::make_sync<Obj>();
    assert(obj1);
    assert(obj2);
    assert(obj3);

    assert(obj1.get() == obj2.get());
    assert(obj1.count() == obj2.count());
    assert(obj1 != obj3);

    obj1.reset(new Obj());
    assert(obj1 == obj2);
    assert(obj1.count() == obj2.count());

    obj1 = obj3;
    assert(obj1 == obj3);
    assert(obj1.count() == obj3.count());
    assert(obj1 != obj2);

    obj1.reset(new Obj());
    assert(obj1.get() != nullptr);
    assert(obj1 == obj3);
    assert(obj1.count() == obj3.count());
}

void tests::mem_sync_ptr_release(void)
{
    struct Obj
    {};

    mem::sync_ptr<Obj> ptr = mem::make_sync<Obj>();
    assert(ptr);

    Obj * raw = ptr.release();
    assert(raw);
    assert(!ptr);

    delete raw;
}

void tests::mem_sync_ptr_exchange(void)
{
    struct Obj
    {};

    mem::sync_ptr<Obj> ptr = mem::make_sync<Obj>();
    assert(ptr);

    Obj * ptr_add = ptr.get();
    Obj * raw = new Obj();
    std::unique_ptr<Obj> ptr_xc(ptr.exchange(raw));

    assert(ptr_xc);
    assert(ptr_xc.get() == ptr_add);

    assert(ptr);
    assert(ptr.get() == raw);
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
    assert(test_allocator_called);
}
