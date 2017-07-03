
#ifndef __MEM_SINGLE_PTR_H__
#define __MEM_SINGLE_PTR_H__


#include <atomic>
#include <cassert>
#include <cstdint>
#include <memory>
#include <mutex>
#include <utility>

#ifndef __MEM_ALLOCATION_POLICY_H__
#include "mem/allocation_policy.h"
#endif

#ifndef __MEM_ATOMIC_PTR_H__
#include "mem/atomic_ptr.h"
#endif


namespace mem
{

    template <class T>
    class single_ptr final
    {

    private:
        using single_ptr_t = single_ptr<T>;

    private:
        static single_ptr<T> *      instance_;
        static bool                 destroyed_;
        T *                         ptr_;


    private:
        single_ptr(void) noexcept
            : ptr_{ new T }
        {}

        ~single_ptr(void) noexcept
        {
            instance_ = nullptr;
            destroyed_ = true;
        }


    public:
        single_ptr(single_ptr_t && p_rhs) noexcept = delete;
        single_ptr(single_ptr_t const & p_rhs) noexcept = delete;
        single_ptr_t & operator=(single_ptr_t && p_rhs) noexcept = delete;
        single_ptr_t & operator=(single_ptr_t const & p_rhs) & noexcept = delete;


    private:
        static void create(void)
        {
            static single_ptr<T> instance;
            instance_ = &instance;
        }

        static void recreate(void)
        {
            create();
            new(instance_) single_ptr<T>;
        }

        static void store(void)
        {
            std::atexit(reclame);
            destroyed_ = false;
        }

        static void reclame(void)
        {
            instance_->~single_ptr();
            instance_ = nullptr;
            destroyed_ = true;
        }


    public:
        static T * instance(void)
        {
            //if(!atomic_ptr_.load(std::memory_order_acquire))
            //{
            //    std::unique_lock<std::mutex> l(mutex_);
                if (!instance_)
                {
                    if (destroyed_)
                    {
                        recreate();
                    }
                    else
                    {
                        create();
                    }
                    store();
                }
            //    l.unlock();
            //}
            return instance_->ptr_;
        }

    }; // class single_ptr

} // namespace mem

//=============================================================================

template <class T>
mem::single_ptr<T> * mem::single_ptr<T>::instance_ = nullptr;

template <class T>
bool mem::single_ptr<T>::destroyed_ = false;

#endif // __MEM_SINGLE_PTR_H__
