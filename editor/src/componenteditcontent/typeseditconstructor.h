#ifndef TYPESEDITCONSTRUCTOR_H
#define TYPESEDITCONSTRUCTOR_H

#include <vector>
#include <map>
#include <memory>

#include "EngineCommon/Types/String/ResourcePath.h"

#include "EngineData/TypeUtils/EnumUtils.h"

#include <QObject>
#include <QLayout>
#include <QString>
#include <QPushButton>

#include <QComboBox>

#include "typeeditconstructorhelpers.h"

namespace TypesEditConstructor
{
	// helpers
	void FillLabel(QLayout* layout, const QString& label);

	template<typename T, class Enable = void>
	struct FillEdit {
		static typename Edit<T>::Ptr Call(QLayout* layout, const QString& label, const T& initialValue);
	};

	template<>
	Edit<float>::Ptr FillEdit<float>::Call(QLayout* layout, const QString& label, const float& initialValue);

	template<>
	Edit<int>::Ptr FillEdit<int>::Call(QLayout* layout, const QString& label, const int& initialValue);

	template<>
	Edit<unsigned int>::Ptr FillEdit<unsigned int>::Call(QLayout* layout, const QString& label, const unsigned int& initialValue);

	template<>
	Edit<unsigned long>::Ptr FillEdit<unsigned long>::Call(QLayout* layout, const QString& label, const unsigned long& initialValue);

	template<>
	Edit<bool>::Ptr FillEdit<bool>::Call(QLayout* layout, const QString& label, const bool& initialValue);

	template<>
	Edit<std::string>::Ptr FillEdit<std::string>::Call(QLayout* layout, const QString& label, const std::string& initialValue);

	template<>
	Edit<RelativeResourcePath>::Ptr FillEdit<RelativeResourcePath>::Call(QLayout* layout, const QString& label, const RelativeResourcePath& initialValue);

	// partial specilization for enums
	template<typename T>
	struct FillEdit<T, typename std::enable_if<std::is_enum<T>::value>::type> {
		static typename Edit<T>::Ptr Call(QLayout* layout, const QString& label, const T& initialValue)
		{
			FillLabel(layout, label);

			QComboBox* stringList = HS_NEW QComboBox();
			for (StringId value : get_all_enum_value_names<T>())
			{
				stringList->addItem(QString::fromStdString(ID_TO_STR(value)));
			}
			stringList->setCurrentText(QString::fromStdString(ID_TO_STR(enum_to_string(initialValue))));

			typename Edit<T>::Ptr edit = std::make_shared<Edit<T>>(initialValue);
			typename Edit<T>::WeakPtr editWeakPtr = edit;

			QObject::connect(stringList, &QComboBox::currentTextChanged, edit->getOwner(), [editWeakPtr](const QString& newValue)
			{
				if (typename Edit<T>::Ptr edit = editWeakPtr.lock())
				{
					if (std::optional<T> value = string_to_enum<T>(STR_TO_ID(newValue.toStdString())))
					{
						edit->transmitValueChange(*value);
					}
				}
			});

			layout->addWidget(stringList);
			return edit;
		}
	};

	template <typename T>
	using is_vector = std::is_same<T, std::vector<typename T::value_type, typename T::allocator_type>>;

	// partial specilization for vectors
	template<typename T>
	struct FillEdit<T, typename std::enable_if<is_vector<T>::value>::type> {
		static typename Edit<T>::Ptr Call(QLayout* layout, const QString& label, const T& initialValue)
		{
			using item_type = typename T::value_type;

			typename Edit<T>::Ptr edit = std::make_shared<Edit<T>>(initialValue);
			typename Edit<T>::WeakPtr editWeakPtr = edit;

			FillLabel(layout, label);

			int index = 0;
			for (const item_type& item : initialValue)
			{
				typename Edit<item_type>::Ptr editItem = FillEdit<item_type>::Call(layout, QString("[%1]").arg(index), item);
				editItem->bindOnChange([editWeakPtr, index](const item_type& /*oldValue*/, const item_type& newValue, bool)
				{
					if (typename Edit<T>::Ptr edit = editWeakPtr.lock())
					{
						T container = edit->getPreviousValue();
						container[static_cast<size_t>(index)] = newValue;
						edit->transmitValueChange(container);
					}
				});
				edit->addChild(editItem);

				QPushButton* removeButton = HS_NEW QPushButton();
				removeButton->setText("x");
				Edit<bool>::Ptr removeItem = std::make_shared<Edit<bool>>(false);
				QObject::connect(removeButton, &QPushButton::pressed, edit->getOwner(), [editWeakPtr, index]()
				{
					if (typename Edit<T>::Ptr edit = editWeakPtr.lock())
					{
						T container = edit->getPreviousValue();
						container.erase(container.begin() + index);
						edit->transmitValueChange(container, true);
					}
				});
				edit->addChild(removeItem);
				layout->addWidget(removeButton);
				index++;
			}

			QPushButton* addButton = HS_NEW QPushButton();
			addButton->setText("add item");
			Edit<bool>::Ptr addItem = std::make_shared<Edit<bool>>(false);
			QObject::connect(addButton, &QPushButton::pressed, edit->getOwner(), [editWeakPtr]()
			{
				if (typename Edit<T>::Ptr edit = editWeakPtr.lock())
				{
					T container = edit->getPreviousValue();
					container.push_back(item_type());
					edit->transmitValueChange(container, true);
				}
			});
			edit->addChild(addItem);
			layout->addWidget(addButton);

			return edit;
		}
	};

	template <typename T>
	using is_map = std::is_same<T, std::map<typename T::key_type, typename T::mapped_type, typename T::key_compare>>;

	// partial specilization for maps
	template<typename T>
	struct FillEdit<T, typename std::enable_if<is_map<T>::value>::type> {
		static typename Edit<T>::Ptr Call(QLayout* layout, const QString& label, const T& initialValue)
		{
			using key_type = typename T::key_type;
			using value_type = typename T::mapped_type;

			typename Edit<T>::Ptr edit = std::make_shared<Edit<T>>(initialValue);
			typename Edit<T>::WeakPtr editWeakPtr = edit;

			FillLabel(layout, label);

			for (const auto& pair : initialValue)
			{
				typename Edit<key_type>::Ptr editKey = FillEdit<key_type>::Call(layout, "key", pair.first);
				editKey->bindOnChange([editWeakPtr, value = pair.second](const key_type& oldKey, const key_type& newKey, bool)
				{
					if (typename Edit<T>::Ptr edit = editWeakPtr.lock())
					{
						T container = edit->getPreviousValue();
						if (container.find(oldKey) != container.end())
						{
							container[newKey] = value;
							container.erase(oldKey);
							edit->transmitValueChange(container);
						}
					}
				});
				edit->addChild(editKey);

				typename Edit<value_type>::Ptr editValue = FillEdit<value_type>::Call(layout, "value", pair.second);
				editValue->bindOnChange([editWeakPtr, key = pair.first](const value_type& /*oldValue*/, const value_type& newValue, bool)
				{
					if (typename Edit<T>::Ptr edit = editWeakPtr.lock())
					{
						T container = edit->getPreviousValue();
						container[key] = newValue;
						edit->transmitValueChange(container);
					}
				});
				edit->addChild(editValue);

				QPushButton* removeButton = HS_NEW QPushButton();
				removeButton->setText("x");
				Edit<bool>::Ptr removeItem = std::make_shared<Edit<bool>>(false);
				QObject::connect(removeButton, &QPushButton::pressed, edit->getOwner(), [editWeakPtr, key = pair.first]()
				{
					if (typename Edit<T>::Ptr edit = editWeakPtr.lock())
					{
						T container = edit->getPreviousValue();
						container.erase(key);
						edit->transmitValueChange(container, true);
					}
				});
				edit->addChild(removeItem);
				layout->addWidget(removeButton);
			}

			QPushButton* addButton = new QPushButton();
			addButton->setText("add item");
			Edit<bool>::Ptr addItem = std::make_shared<Edit<bool>>(false);
			QObject::connect(addButton, &QPushButton::pressed, edit->getOwner(), [editWeakPtr]()
			{
				if (typename Edit<T>::Ptr edit = editWeakPtr.lock())
				{
					T container = edit->getPreviousValue();
					container.emplace();
					edit->transmitValueChange(container, true);
				}
			});
			edit->addChild(addItem);
			layout->addWidget(addButton);

			return edit;
		}
	};
}

#endif // TYPESEDITCONSTRUCTOR_H
