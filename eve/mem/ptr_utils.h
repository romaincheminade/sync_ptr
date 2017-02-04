
#pragma once
#ifndef __EVE_MEMORY_PTR_UTILS_H__
#define __EVE_MEMORY_PTR_UTILS_H__

#include <type_traits>
#include <utility>

namespace eve
{
	namespace mem
	{

		/** 
		* \brief Default allocator used by smart pointer(s).
		* Calls "new" on template type.
		*/
		template<
            class TType>
		struct default_allocator
		{
			constexpr default_allocator(
				void)
                noexcept = default;

            template<
                class TType2,
                class = typename std::enable_if<std::is_convertible<TType2 *, TType *>::value, void>::type>
                default_allocator(default_allocator<TType2> const &)
                noexcept
            {}

			template<class ...TArg>
			TType * operator()(
				TArg && ...p_args)
				const
			{
				static_assert(
                    0 < sizeof(TType),
					"can't allocate an incomplete type");
				return new TType(std::forward<TArg>(p_args)...);
			}

		}; // struct default_allocator


		/**
		* \brief Default deleter used by smart pointer(s).
		* Calls "delete" on target template type pointer.
		*/
		template<
            class TType>
		struct default_deleter
		{	
			constexpr default_deleter(
				void) 
				noexcept = default;

            template<
                class TType2,
                class = typename std::enable_if<std::is_convertible<TType2 *, TType *>::value, void>::type>
                default_deleter(default_deleter<TType2> const &)
                noexcept
            {}

			void operator()(
				TType * p_ptr) 
				const noexcept
			{	
				static_assert(
                    0 < sizeof(TType),
					"can't delete an incomplete type");
				delete p_ptr;
			}

		}; // struct default_deleter

	} // namespace mem

} // namespace eve

#endif // __EVE_MEMORY_PTR_UTILS_H__
