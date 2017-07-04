
#include <cstdio>
#include <iostream>
#include <memory>
#include <utility>

#include <mem/atomic_ptr.h>
#include <test_assert.h>


namespace
{
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
}


int main(
    int /*argc*/, char ** /*argv*/)
    try
{

    class Obj {};
    class Widget : public Obj {};

    {
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

        ao.reset(new Widget);
        test_assert(std::size_t(ao.get()) != 0);
        test_assert(std::size_t(ao.non_atomic_get()) != 0);
        ptr_address = std::size_t(ao.get());
        test_assert(ptr_address == std::size_t(ao.non_atomic_get()));

        ptr.reset(ao.exchange(new Widget));
        test_assert(std::size_t(ao.get()) != 0);
        test_assert(std::size_t(ao.non_atomic_get()) != 0);
        test_assert(std::size_t(ao.get()) != ptr_address);
        test_assert(std::size_t(ao.non_atomic_get()) != ptr_address);
        test_assert(std::size_t(ptr.get()) == ptr_address);
    }

    {
        test_allocator<Obj> allocator_;

        auto obj(mem::allocate_atomic<Obj>(allocator_));

        test_assert(obj);
        test_assert(std::size_t(obj.get()) != 0);
        test_assert(std::size_t(obj.non_atomic_get()) != 0);
    }


    return 0;
}
catch (...)
{
    return 1;
}
