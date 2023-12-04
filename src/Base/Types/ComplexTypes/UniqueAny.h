#pragma once

#include <memory>

/**
 * @brief Class that implements non-copyable version of std::any
 *
 * It has pretty limited implementation, add new functionalily when needed
 */
class UniqueAny
{
public:
	UniqueAny() noexcept = default;
	~UniqueAny() = default;
	UniqueAny(UniqueAny&) = delete;
	UniqueAny(UniqueAny&& other) noexcept
		: mData(std::move(other.mData))
	{
	}

	UniqueAny& operator=(UniqueAny&) = delete;
	UniqueAny& operator=(UniqueAny&& other) noexcept {
		mData = std::move(other.mData);
		return *this;
	}

	template <typename T, typename... Args>
	static UniqueAny Create(Args&&... args)
	{
		UniqueAny newAny;
		newAny.mData = std::make_unique<TypeStoredData<std::decay_t<T>>>(std::forward<Args>(args)...);
		return newAny;
	}

	template <typename T>
	T* cast() noexcept
	{
		if (mData && dynamic_cast<TypeStoredData<T>*>(mData.get()) != nullptr)
		{
			return &static_cast<TypeStoredData<T>*>(mData.get())->data;
		}
		return nullptr;
	}

	template <typename T>
	const T* cast() const noexcept
	{
		if (mData && dynamic_cast<TypeStoredData<T>*>(mData.get()) != nullptr)
		{
			return &static_cast<TypeStoredData<T>*>(mData.get())->data;
		}
		return nullptr;
	}

private:
	struct AbstractStoredData
	{
		virtual ~AbstractStoredData() = default;
		virtual const std::type_info& getType() const noexcept = 0;
	};

	template <typename T>
	struct TypeStoredData final : public AbstractStoredData
	{

		template <typename... Args>
		TypeStoredData(Args&&... args)
			: data(std::forward<Args>(args)...)
		{}

		const std::type_info& getType() const noexcept override
		{
			return typeid(T);
		}

		T data;
	};

	std::unique_ptr<AbstractStoredData> mData;
};
