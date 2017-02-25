
// Main header.
#include "cc_sync_ptr.h"

#include <cassert>


void tests::cc_sync_ptr_synchro(void)
{
    class Obj
    {};
    cc::sync_ptr<Obj>		obj;

    cc::sync_ptr<Obj>		obj1 = cc::make_sync<Obj>();
    cc::sync_ptr<Obj>		obj2(obj1);
    cc::sync_ptr<Obj>		obj3 = cc::make_sync<Obj>();
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

void tests::cc_sync_ptr_release(void)
{
    struct Obj
    {};

    cc::sync_ptr<Obj> ptr = cc::make_sync<Obj>();
    assert(ptr);

    Obj * ptr_out = nullptr;
    bool ret = ptr.release(&ptr_out);
    assert(ptr_out);
    assert(!ptr);

    delete ptr_out;
}

void tests::cc_sync_ptr_exchange(void)
{
    struct Obj
    {};

    cc::sync_ptr<Obj> ptr = cc::make_sync<Obj>();
    assert(ptr);

    Obj * ptr_add   = ptr.get();
    Obj * ptr_out   = nullptr;
    Obj * ptr_in    = new Obj();
    bool ret = ptr.exchange(&ptr_out, ptr_in);

    assert(ret);
    assert(ptr_out == ptr_add);

    assert(ptr);
    assert(ptr.get() == ptr_in);

    delete ptr_out;
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

void tests::cc_sync_ptr_allocator(void)
{
    class Obj
    {};

    // User defined template.

    test_allocator_called = false;
    {
        test_allocator<Obj> allocator_;

        auto obj(
            cc::make_sync_with_allocator<
                Obj> (
                    allocator_));
    }
    assert(test_allocator_called);
}
