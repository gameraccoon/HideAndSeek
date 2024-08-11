#pragma once

#include <optional>

// The wrapper calls the callable object (lambda, function, std::function) only when first evaluated and caches the result
// 2nd and later evaluations just return this cahced result without evaluating the callable object
//
// Limitations:
// * only callables with copyable non-vold return type (makes sense, right?)
// * only callables without arguments
// * the callable should be rvalue ref
// * only single-thread usage
//
// check out the tests to see how it can be used
template<typename CallableType, typename ReturnType = std::invoke_result_t<CallableType>>
class LazyEvaluated
{
public:
	explicit LazyEvaluated(CallableType lambda)
		: mLambda(std::move(lambda))
	{}

	ReturnType operator()()
	{
		if (!mCachedResult.has_value())
		{
			mCachedResult = mLambda();
		}
		return *mCachedResult;
	}

private:
	CallableType mLambda;
	std::optional<ReturnType> mCachedResult;
};
