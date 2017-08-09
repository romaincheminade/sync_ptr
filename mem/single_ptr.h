
#ifndef __MEM_SINGLE_PTR_H__
#define __MEM_SINGLE_PTR_H__


#include <atomic>
#include <cassert>
#include <cstdint>
#include <cstdlib>
#include <memory>
#include <mutex>
#include <utility>


namespace mem
{

    template <class T>
    class single_ptr final
    {

    private:
        class body final
        {

        private:
            static body *           instance_;
            static bool             destroyed_;
            std::unique_ptr<T>      ptr_;

        private:
            body(void) noexcept
                : ptr_{ std::make_unique<T>() }
            {}

        public:
            ~body(void) noexcept
            {
                instance_ = nullptr;
                destroyed_ = true;
            }

            body(body && p_rhs) noexcept = delete;
            body(body const & p_rhs) noexcept = delete;
            body & operator=(body && p_rhs) noexcept = delete;
            body & operator=(body const & p_rhs) & noexcept = delete;

        private:
            static void create(void)
            {
                static body instance;
                instance_ = &instance;
            }

            static void recreate(void)
            {
                create();
                new (instance_) T;
                std::atexit(reclame);
            }

            static void reclame(void)
            {
                instance_->~body();
                destroyed_ = true;
            }

        public:
            static T * instance(void)
            {
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
                }
                return instance_->ptr_.get();
            }

        }; // class body


    public:
        single_ptr(void) noexcept = default;
        ~single_ptr(void) noexcept = default;

        single_ptr(single_ptr && p_rhs) noexcept = delete;
        single_ptr(single_ptr const & p_rhs) noexcept = delete;
        single_ptr & operator=(single_ptr && p_rhs) noexcept = delete;
        single_ptr & operator=(single_ptr const & p_rhs) & noexcept = delete;


    public:
        T * get(void) const noexcept
        {
            return body::instance();
        }

        T * operator->(void) const noexcept
        {
            return get();
        }

        T & operator*(void) const noexcept
        {
            return *get();
        }

    public:
        constexpr bool valid(void) const noexcept
        {
            return true;
        }

        constexpr operator bool(void) const noexcept
        {
            return true;
        }


    }; // class single_ptr

} // namespace mem

//=============================================================================

template <class T>
typename mem::single_ptr<T>::body * mem::single_ptr<T>::body::instance_ = nullptr;

template <class T>
bool mem::single_ptr<T>::body::destroyed_ = false;

#endif // __MEM_SINGLE_PTR_H__
