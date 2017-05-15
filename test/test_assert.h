
#pragma once
#ifndef __TEST_ASSERT_H__
#define __TEST_ASSERT_H__

#include <cstdlib>


namespace 
{

	template<class TExpression>
	void test_assert(TExpression const & p_expression)
	{
		if (!p_expression)
		{
			std::abort();
		}
	}

} // namespace 

#endif // __TEST_ASSERT_H__
