#include "typeseditconstructor.h"

#include <limits>
#include <string>

#include <QCheckBox>
#include <QDoubleValidator>
#include <QIntValidator>
#include <QLabel>
#include <QLineEdit>

namespace TypesEditConstructor
{
	void FillLabel(QLayout* layout, const QString& label)
	{
		QLabel* editLabel = HS_NEW QLabel();
		editLabel->setText(label + ":");
		layout->addWidget(editLabel);
	}

	template<>
	Edit<float>::Ptr FillEdit<float>::Call(QLayout* layout, const QString& label, const float& initialValue)
	{
		FillLabel(layout, label);

		QLineEdit* floatEdit = HS_NEW QLineEdit();

		const auto dv = HS_NEW QDoubleValidator(std::numeric_limits<double>::lowest(), std::numeric_limits<double>::max(), 10);
		dv->setNotation(QDoubleValidator::StandardNotation);
		floatEdit->setValidator(dv);

		floatEdit->setText(QString::number(static_cast<double>(initialValue)));

		Edit<float>::Ptr edit = std::make_shared<Edit<float>>(initialValue);
		Edit<float>::WeakPtr editWeakPtr = edit;

		QObject::connect(floatEdit, &QLineEdit::textChanged, edit->getOwner(), [editWeakPtr](const QString& newValueStr) {
			if (const Edit<float>::Ptr edit = editWeakPtr.lock())
			{
				bool ok;
				const float newValue = newValueStr.toFloat(&ok);
				if (ok)
				{
					edit->transmitValueChange(newValue);
				}
			}
		});

		layout->addWidget(floatEdit);
		return edit;
	}

	template<>
	Edit<int>::Ptr FillEdit<int>::Call(QLayout* layout, const QString& label, const int& initialValue)
	{
		FillLabel(layout, label);

		QLineEdit* intEdit = HS_NEW QLineEdit();

		const auto iv = HS_NEW QIntValidator(std::numeric_limits<int>::lowest(), std::numeric_limits<int>::max());
		intEdit->setValidator(iv);

		intEdit->setText(QString::number(initialValue));

		Edit<int>::Ptr edit = std::make_shared<Edit<int>>(initialValue);
		Edit<int>::WeakPtr editWeakPtr = edit;

		QObject::connect(intEdit, &QLineEdit::textChanged, edit->getOwner(), [editWeakPtr](const QString& newValueStr) {
			if (const Edit<int>::Ptr edit = editWeakPtr.lock())
			{
				bool ok;
				const int newValue = newValueStr.toInt(&ok);
				if (ok)
				{
					edit->transmitValueChange(newValue);
				}
			}
		});

		layout->addWidget(intEdit);
		return edit;
	}

	template<>
	Edit<unsigned int>::Ptr FillEdit<unsigned int>::Call(QLayout* layout, const QString& label, const unsigned int& initialValue)
	{
		FillLabel(layout, label);

		QLineEdit* intEdit = HS_NEW QLineEdit();

		intEdit->setText(QString::number(initialValue));

		Edit<unsigned int>::Ptr edit = std::make_shared<Edit<unsigned int>>(initialValue);
		Edit<unsigned int>::WeakPtr editWeakPtr = edit;

		QObject::connect(intEdit, &QLineEdit::textChanged, edit->getOwner(), [editWeakPtr](const QString& newValueStr) {
			if (const Edit<unsigned int>::Ptr edit = editWeakPtr.lock())
			{
				bool ok;
				const unsigned int newValue = newValueStr.toUInt(&ok);
				if (ok)
				{
					edit->transmitValueChange(newValue);
				}
			}
		});

		layout->addWidget(intEdit);
		return edit;
	}

	template<>
	Edit<unsigned long>::Ptr FillEdit<unsigned long>::Call(QLayout* layout, const QString& label, const unsigned long& initialValue)
	{
		FillLabel(layout, label);

		QLineEdit* longEdit = HS_NEW QLineEdit();

		longEdit->setText(QString::number(initialValue));

		Edit<unsigned long>::Ptr edit = std::make_shared<Edit<unsigned long>>(initialValue);
		Edit<unsigned long>::WeakPtr editWeakPtr = edit;

		QObject::connect(longEdit, &QLineEdit::textChanged, edit->getOwner(), [editWeakPtr](const QString& newValueStr) {
			if (const Edit<unsigned long>::Ptr edit = editWeakPtr.lock())
			{
				bool ok;
				const unsigned long newValue = newValueStr.toUInt(&ok);
				if (ok)
				{
					edit->transmitValueChange(newValue);
				}
			}
		});

		layout->addWidget(longEdit);
		return edit;
	}

	template<>
	Edit<bool>::Ptr FillEdit<bool>::Call(QLayout* layout, const QString& label, const bool& initialValue)
	{
		FillLabel(layout, label);

		QCheckBox* checkbox = HS_NEW QCheckBox();
		checkbox->setChecked(initialValue);

		Edit<bool>::Ptr edit = std::make_shared<Edit<bool>>(initialValue);
		Edit<bool>::WeakPtr editWeakPtr = edit;

		QObject::connect(checkbox, &QCheckBox::stateChanged, edit->getOwner(), [editWeakPtr](int newValue) {
			if (const Edit<bool>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(static_cast<bool>(newValue));
			}
		});

		layout->addWidget(checkbox);
		return edit;
	}

	template<>
	Edit<std::string>::Ptr FillEdit<std::string>::Call(QLayout* layout, const QString& label, const std::string& initialValue)
	{
		FillLabel(layout, label);

		QLineEdit* stringEdit = HS_NEW QLineEdit();
		stringEdit->setText(QString::fromStdString(initialValue));

		Edit<std::string>::Ptr edit = std::make_shared<Edit<std::string>>(initialValue);
		Edit<std::string>::WeakPtr editWeakPtr = edit;

		QObject::connect(stringEdit, &QLineEdit::textChanged, edit->getOwner(), [editWeakPtr](const QString& newValue) {
			if (const Edit<std::string>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(newValue.toStdString());
			}
		});

		layout->addWidget(stringEdit);
		return edit;
	}

	template<>
	Edit<RelativeResourcePath>::Ptr FillEdit<RelativeResourcePath>::Call(QLayout* layout, const QString& label, const RelativeResourcePath& initialValue)
	{
		FillLabel(layout, label);

		QLineEdit* stringEdit = HS_NEW QLineEdit();
		stringEdit->setText(QString::fromStdString(initialValue.getRelativePathStr()));

		Edit<RelativeResourcePath>::Ptr edit = std::make_shared<Edit<RelativeResourcePath>>(initialValue);
		Edit<RelativeResourcePath>::WeakPtr editWeakPtr = edit;

		QObject::connect(stringEdit, &QLineEdit::textChanged, edit->getOwner(), [editWeakPtr](const QString& newValue) {
			if (const Edit<RelativeResourcePath>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(static_cast<RelativeResourcePath>(newValue.toStdString()));
			}
		});

		layout->addWidget(stringEdit);
		return edit;
	}
} // namespace TypesEditConstructor
