
#ifndef __MEM_STACK_PTR_H__
#define __MEM_STACK_PTR_H__


#include <cassert>
#include <cstdint>
#include <memory>
#include <utility>


namespace mem
{

    template <class T>
    class stack_ptr final
    {

    private:
        T       object_;

    public:
        template<class ...TArgs>
        stack_ptr(TArgs&&... p_args) noexcept(noexcept(T(std::forward<TArgs>(p_args)...)))
            : object_{ std::forward<TArgs>(p_args)... }
        {}

        ~stack_ptr(void) noexcept = default;

        stack_ptr(stack_ptr && p_rhs) noexcept = default;
        stack_ptr(stack_ptr const & p_rhs) = default;
        stack_ptr & operator=(stack_ptr && p_rhs) noexcept = default;
        stack_ptr & operator=(stack_ptr const & p_rhs) & = default;


    public:
        template<class ...TArgs>
        void reset(TArgs&&... p_args) noexcept(noexcept(T(std::forward<TArgs>(p_args)...)))
        {
            object_.~T();
            auto ptr = &object_;
            new (ptr) T(std::forward<TArgs>(p_args)...);
        }


    public:
        T & get(void) noexcept
        {
            return object_;
        }

        T * operator->(void) noexcept
        {
            return &object_;
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

    }; // class stack_ptr

} // namespace mem

//=============================================================================


#endif // __MEM_STACK_PTR_H__
