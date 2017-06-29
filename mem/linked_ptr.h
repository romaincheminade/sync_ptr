
#ifndef __MEM_LINKED_PTR_H__
#define __MEM_LINKED_PTR_H__


#include <atomic>
#include <cassert>
#include <cstdint>
#include <functional>
#include <utility>

#ifndef __MEM_ALLOCATION_POLICY_H__
#include "mem/allocation_policy.h"
#endif

#ifndef __MEM_SYNC_PTR_H__
#include "mem/sync_ptr.h"
#endif


namespace mem
{

    template <
        class TPtr,
        template <class T> class TDeleter = sync_ptr_deleter>
    class linked_ptr final
    {

    private:
        using linked_ptr_t  = linked_ptr<TPtr, TDeleter>;
        using sync_ptr_t    = sync_ptr<TPtr, TDeleter>;
        using body_t        = typename sync_ptr_t::body;

    private:
        body_t *		body_;


    public:
        linked_ptr(void) noexcept
            : body_{ new body_t }
        {}

        linked_ptr(linked_ptr_t && p_rhs) noexcept
            : body_{ p_rhs.body_ }
        {
            p_rhs.body_ = nullptr;
        }

        linked_ptr_t & operator=(linked_ptr_t && p_rhs) noexcept
        {
            body_ = p_rhs.body_;
            p_rhs.body_ = nullptr;
            return *this;
        }

        linked_ptr(linked_ptr_t const & p_rhs) noexcept = delete;
        linked_ptr(sync_ptr_t const & p_rhs) noexcept
            : body_{ p_rhs.body_ }
        {
            body_->ref();
        }

        linked_ptr_t & operator=(linked_ptr_t const & p_rhs) & noexcept = delete;
        linked_ptr_t & operator=(sync_ptr_t const & p_rhs) & noexcept
        {
            auto * tmp = p_rhs.body_;
            if (tmp != body_)
            {
                body_->unref();
                body_ = tmp;
                body_->ref();
            }
            return *this;
        }

        ~linked_ptr(void) noexcept
        {
            if (body_)
            {
                body_->unref();
            }
        }

        void swap(linked_ptr_t & p_rhs) noexcept
        {
            std::swap(body_, p_rhs.body_);
        }

    public:
        TPtr * get(void) const noexcept
        {
            return body_->get();
        }

        TPtr * operator->(void) const noexcept
        {
            return get();
        }

        TPtr & operator*(void) const noexcept
        {
            return *get();
        }

    public:
        bool valid(void) const noexcept
        {
            return (get() != nullptr);
        }

        operator bool(void) const noexcept
        {
            return valid();
        }

        std::int32_t count(void) const noexcept
        {
            return body_->ref_count();
        }

        bool orphan(void) const noexcept
        {
            return (body_->ref_count() == 1);
        }

    }; // class linked_ptr

} // namespace mem


namespace std
{
    
    template <
        class TPtr,
        template <class T> class TDeleter>
    void swap(
        mem::linked_ptr<TPtr, TDeleter> & p_lhs,
        mem::linked_ptr<TPtr, TDeleter> & p_rhs)
        noexcept
    {
        p_lhs.swap(p_rhs);
    }

} // namespace std

#endif // __MEM_LINKED_PTR_H__