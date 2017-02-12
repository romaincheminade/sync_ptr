
#include <windows.h>

#include <cassert>

#include "sync_ptr.h"



//=================================================================================================
void sync_ptr_synchro(void)
{
    class Obj
    {};
    eve::mem::sync_ptr<Obj>		obj;

    eve::mem::sync_ptr<Obj>		obj1 = eve::mem::make_sync<Obj>();
    eve::mem::sync_ptr<Obj>		obj2(obj1);
    eve::mem::sync_ptr<Obj>		obj3 = eve::mem::make_sync<Obj>();
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
    assert(obj1 != nullptr);
    assert(obj1 == obj3);
    assert(obj1.count() == obj3.count());
}



//=================================================================================================
static bool test_deleter_called = false;

template<class TType>
struct test_deleter
{
    constexpr test_deleter(
        void)
        noexcept = default;

    constexpr test_deleter(
        test_deleter<TType> const &)
        noexcept = default;

    void operator()(
        TType * p_ptr)
        const noexcept
    {
        static_assert(0 < sizeof(TType),
            "can't delete an incomplete type");

        test_deleter_called = true;
        delete p_ptr;
    }
};

//=================================================================================================
void sync_ptr_deleter(void)
{
    class Obj
    {};

    // User defined template.

    test_deleter_called = false;
    {
        test_deleter<Obj> deleter_;

        auto obj(
            eve::mem::make_sync_with_deleter<
            Obj>(
                deleter_));
    }
    assert(test_deleter_called);

    // Lambda expression.

    bool lambda_called = false;
    {
        std::function<void(Obj*)> deleter_ =
            [&lambda_called](Obj * p_ptr) noexcept
        {
            lambda_called = true;
            delete p_ptr;
        };

        auto obj(
            eve::mem::make_sync_with_deleter<
            Obj>(
                deleter_));
    }
    assert(lambda_called);
}



//=================================================================================================
static bool test_allocator_called = false;

template<class TType>
struct test_allocator
{
    constexpr test_allocator(
        void)
        noexcept = default;

    template<class ...TArg>
    TType * operator()(
        TArg && ...p_args)
        const
    {
        static_assert(0 < sizeof(TType),
            "can't allocate an incomplete type");

        test_allocator_called = true;
        return new TType(std::forward<TArg>(p_args)...);
    }
};

//=================================================================================================
void sync_ptr_allocator(void)
{
    class Obj
    {};

    // User defined template.

    test_allocator_called = false;
    {
        test_allocator<Obj> allocator_;

        auto obj(
            eve::mem::make_sync_with_allocator<
            Obj>(
                allocator_));
    }
    assert(test_allocator_called);

    // Lambda expression.

    bool lambda_called = false;
    {
        auto allocator_ =
            [&lambda_called](void)
        {
            lambda_called = true;
            return new Obj();
        };

        auto obj(
            eve::mem::make_sync_with_allocator<
            Obj>(
                allocator_));
    }
    assert(lambda_called);
}



//=================================================================================================
void sync_ptr_allocator_and_deleter(void)
{
    class Obj
    {};

    // User defined template.

    test_allocator_called = false;
    test_deleter_called = false;
    {
        test_allocator<Obj> allocator_;
        test_deleter<Obj> deleter_;

        auto obj(
            eve::mem::make_sync_with_allocator_and_deleter<
            Obj>(
                allocator_,
                deleter_));
    }
    assert(test_allocator_called);
    assert(test_deleter_called);

    // Lambda expression.

    bool allocator_called = false;
    bool deleter_called = false;
    {
        auto allocator_ =
            [&allocator_called](void)
        {
            allocator_called = true;
            return new Obj();
        };

        auto deleter_ =
            [&deleter_called](Obj * p_ptr)
        {
            deleter_called = true;
            delete p_ptr;
        };

        auto obj(
            eve::mem::make_sync_with_allocator_and_deleter<
            Obj>(
                allocator_,
                deleter_));
    }
    assert(allocator_called);
    assert(deleter_called);
}



void sync_ptr_steal(void)
{
    struct Obj
    {};

    // First Chain.
    eve::mem::sync_ptr<Obj> ptr1 = eve::mem::make_sync<Obj>();
    eve::mem::sync_ptr<Obj> ptr2 = ptr1;
    auto raw_1 = ptr1.get();
    assert(raw_1);
    assert(ptr1 == ptr2);

    // Second Chain.
    eve::mem::sync_ptr<Obj> ptr3 = eve::mem::make_sync<Obj>();
    eve::mem::sync_ptr<Obj> ptr4 = ptr3;
    auto raw_2 = ptr3.get();
    assert(raw_2);
    assert(ptr3 == ptr4);

    // Stealing.
    ptr3.steal(ptr1);
    
    assert(ptr3.get() == raw_1);
    assert(ptr3 == ptr4);

    assert(ptr1.get() == nullptr);
    assert(ptr1 == ptr2);
}



//=================================================================================================
int WINAPI WinMain(																					
    HINSTANCE /*hInstance*/, 																	
    HINSTANCE /*hPrevInstance*/, 																
    LPSTR /*lpCmdLine*/, 																		
    int /*nCmdShow*/) try																		
{			
    // sync_ptr vs std::shared_ptr

    struct Obj
    {};
    std::shared_ptr<Obj> ptr1 = std::make_shared<Obj>();
    std::shared_ptr<Obj> ptr2 = ptr1;
    assert(ptr1 == ptr2); // same behavior
    ptr1.reset(new Obj);
    assert(ptr1 != ptr2); // different behavior

    eve::mem::sync_ptr<Obj> sptr1 = eve::mem::make_sync<Obj>();
    eve::mem::sync_ptr<Obj> sptr2 = sptr1;
    assert(sptr1 == sptr2); // same behavior
    sptr1.reset(new Obj);
    assert(sptr1 == sptr2); // different behavior

    // tests

    sync_ptr_synchro();
    sync_ptr_deleter();
    sync_ptr_allocator();
    sync_ptr_allocator_and_deleter();
    sync_ptr_steal();

    return 0;																					
}																								
catch (...)																						
{				
    return 1;
}
