#pragma once

#include <functional>
#include <memory>

#include <QObject>

namespace TypesEditConstructor
{
	class BaseEdit
	{
	public:
		using Ptr = std::shared_ptr<BaseEdit>;
	};

	template<typename T>
	class Edit : public BaseEdit
	{
	public:
		using Ptr = std::shared_ptr<Edit>;
		using WeakPtr = std::weak_ptr<Edit>;
		using OnChangeFn = std::function<void(const T&, const T&, bool)>;

	public:
		explicit Edit(T initialValue)
			: mPrevValue(std::forward<T>(initialValue))
		{
		}

		~Edit()
		{
			delete mOwner;
		}

		void bindOnChange(OnChangeFn onChange)
		{
			mOnChange = onChange;
		}

		void transmitValueChange(T newValue, bool forceUpdateLayout = false)
		{
			if (newValue != mPrevValue)
			{
				if (mOnChange)
				{
					mOnChange(mPrevValue, std::forward<T>(newValue), forceUpdateLayout);
				}
				mPrevValue = newValue;
			}
		}

		void addChild(BaseEdit::Ptr&& child)
		{
			mChildObjects.emplace_back(std::move(child));
		}

		QObject* getOwner() { return mOwner; }
		T getPreviousValue() const { return mPrevValue; }

		Edit(const Edit&) = delete;
		Edit& operator=(const Edit&) = delete;

	private:
		T mPrevValue;
		OnChangeFn mOnChange;
		QObject* mOwner = HS_NEW QObject();
		std::vector<BaseEdit::Ptr> mChildObjects;
	};
} // namespace TypesEditConstructor
