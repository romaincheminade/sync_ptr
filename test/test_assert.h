
#pragma once
#ifndef __CC_TEST_ASSERT_H__
#define __CC_TEST_ASSERT_H__

#include <cstdlib>


namespace cc
{

	template<class TExpression>
	void test_assert(TExpression const & p_expression)
	{
		if (!p_expression)
		{
			std::abort();
		}
	}

} // namespace cc

#endif // __CC_TEST_ASSERT_H__
