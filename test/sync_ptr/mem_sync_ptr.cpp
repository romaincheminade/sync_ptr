
// Main header.
#include "mem_sync_ptr.h"

#include <memory>
#include <new>

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



template<class T>
struct test_allocator
{
    constexpr test_allocator(
        void)
        noexcept = default;

    T * allocate(std::size_t p_num)
    {
        static_assert(0 < sizeof(T), "can't allocate an incomplete type");
        return static_cast<T*>(::operator new(sizeof(T) * p_num));
    }

    void deallocate(T * p_ptr, std::size_t p_num)
    {
        ::operator delete(p_ptr, sizeof(T) * p_num);
    }

    template<
        class TPtr,
        class... TArgs>
    void construct(TPtr * p_ptr, TArgs&&... p_args) const
    {
        ::new ((void *)p_ptr) TPtr(std::forward<TArgs>(p_args)...);
    }
    
    template<class TPtr>
    void destroy(TPtr * p_ptr) const
    {	
        p_ptr->~_Uty();
    }
};

void tests::mem_sync_ptr_allocator(void)
{
    class Obj
    {};

    test_allocator<Obj> allocator_;

    auto obj(
        mem::allocate_sync<
        Obj>(
            allocator_));

    test_assert(obj);
    test_assert(obj.get());
}
