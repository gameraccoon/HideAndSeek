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
	Edit<RaccoonEcs::Entity>::Ptr FillEdit<RaccoonEcs::Entity>::Call(QLayout* layout, const QString& label, const RaccoonEcs::Entity& initialValue)
	{
		FillLabel(layout, label);

		QHBoxLayout *innerLayout = HS_NEW QHBoxLayout;

		Edit<RaccoonEcs::Entity>::Ptr edit = std::make_shared<Edit<RaccoonEcs::Entity>>(initialValue);
		Edit<RaccoonEcs::Entity>::WeakPtr editWeakPtr = edit;

		Edit<RaccoonEcs::Entity::EntityId>::Ptr editAngle = FillEdit<RaccoonEcs::Entity::EntityId>::Call(innerLayout, "id", initialValue.getId());
		editAngle->bindOnChange([editWeakPtr](float /*oldValue*/, RaccoonEcs::Entity::EntityId newValue, bool)
		{
			if (Edit<RaccoonEcs::Entity>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(RaccoonEcs::Entity(newValue));
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
