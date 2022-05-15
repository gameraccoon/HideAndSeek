#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

#include <string>

#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QCheckBox>
#include <QHBoxLayout>

namespace TypesEditConstructor
{
	template<>
	Edit<CellPos>::Ptr FillEdit<CellPos>::Call(QLayout* layout, const QString& label, const CellPos& initialValue)
	{
		FillLabel(layout, label);

		QHBoxLayout *innerLayout = HS_NEW QHBoxLayout;

		Edit<CellPos>::Ptr edit = std::make_shared<Edit<CellPos>>(initialValue);
		Edit<CellPos>::WeakPtr editWeakPtr = edit;

		Edit<int>::Ptr editX = FillEdit<int>::Call(innerLayout, "x", initialValue.x);
		editX->bindOnChange([editWeakPtr](int /*oldValue*/, int newValue, bool)
		{
			if (Edit<CellPos>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(CellPos(newValue, edit->getPreviousValue().y));
			}
		});
		edit->addChild(editX);

		Edit<int>::Ptr editY = FillEdit<int>::Call(innerLayout, "y", initialValue.y);
		editY->bindOnChange([editWeakPtr](int /*oldValue*/, int newValue, bool)
		{
			if (Edit<CellPos>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(CellPos(edit->getPreviousValue().x, newValue));
			}
		});
		edit->addChild(editY);

		innerLayout->addStretch();
		QWidget* container = HS_NEW QWidget();
		container->setLayout(innerLayout);
		layout->addWidget(container);
		return edit;
	}
}
