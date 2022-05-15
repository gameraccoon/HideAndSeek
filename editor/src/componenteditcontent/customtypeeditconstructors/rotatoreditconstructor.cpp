#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

#include <string>

#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QCheckBox>
#include <QHBoxLayout>

namespace TypesEditConstructor
{
	static const float RAD_TO_DEG = 180.0f / PI;
	static const float DEG_TO_RAD = 1.0f / RAD_TO_DEG;

	template<>
	Edit<Rotator>::Ptr FillEdit<Rotator>::Call(QLayout* layout, const QString& label, const Rotator& initialValue)
	{
		FillLabel(layout, label);

		QHBoxLayout *innerLayout = HS_NEW QHBoxLayout;

		Edit<Rotator>::Ptr edit = std::make_shared<Edit<Rotator>>(initialValue);
		Edit<Rotator>::WeakPtr editWeakPtr = edit;

		Edit<float>::Ptr editAngle = FillEdit<float>::Call(innerLayout, "angle", initialValue.getValue() * RAD_TO_DEG);
		editAngle->bindOnChange([editWeakPtr](float /*oldValue*/, float newValue, bool)
		{
			if (Edit<Rotator>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(Rotator(newValue * DEG_TO_RAD));
			}
		});
		edit->addChild(editAngle);

		innerLayout->addStretch();
		QWidget* container = HS_NEW QWidget();
		container->setLayout(innerLayout);
		layout->addWidget(container);
		return edit;
	}
}
