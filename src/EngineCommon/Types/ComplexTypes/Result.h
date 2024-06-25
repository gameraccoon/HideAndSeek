#pragma once

#include <variant>

template<typename T, typename ErrT>
class Result
{
public:
	template<typename U>
	static Result Ok(U&& value) { return Result(std::forward<U>(value)); }
	template<typename U>
	static Result Err(U&& error) { return Result(Error{std::forward<U>(error)}); }

	Result(const Result&) = default;
	Result(Result&&) = default;
	Result& operator=(const Result&) = default;
	Result& operator=(Result&&) = default;
	~Result() = default;

	bool operator==(const Result& rhs) const = default;
	bool operator!=(const Result& rhs) const = default;

	[[nodiscard]] bool hasValue() const
	{
		return std::holds_alternative<T>(mData);
	}

	[[nodiscard]] bool hasError() const
	{
		return std::holds_alternative<Error>(mData);
	}

	[[nodiscard]] const T& getValue() const
	{
		AssertFatal(hasValue(), "Trying to get value from Result that has an error set");
		return std::get<T>(mData);
	}

	T consumeValue() {
		AssertFatal(hasValue(), "Trying to get value from Result that has an error set");
		return std::move(std::get<T>(std::move(mData)));
	}

	[[nodiscard]] const T& getValueOr(const T& defaultValue) const
	{
		if (hasValue())
		{
			return std::get<T>(mData);
		}
		else
		{
			return defaultValue;
		}
	}

	[[nodiscard]] const ErrT& getError() const
	{
		AssertFatal(hasError(), "Trying to get error from Result that has a value set");
		return std::get<Error>(mData).error;
	}

private:
	struct Error
	{
		ErrT error;

		bool operator==(const Error& rhs) const = default;
		bool operator!=(const Error& rhs) const = default;
	};

private:
	template<typename U>
	explicit Result(U&& value) : mData(std::forward<U>(value)) {}
	template<typename U>
	explicit Result(Error&& error) : mData(std::move(error)) {}

	std::variant<T, Error> mData;
};
