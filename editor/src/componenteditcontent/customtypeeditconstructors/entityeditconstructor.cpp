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
	Edit<Entity>::Ptr FillEdit<Entity>::Call(QLayout* layout, const QString& label, const Entity& initialValue)
	{
		FillLabel(layout, label);

		QHBoxLayout *innerLayout = HS_NEW QHBoxLayout;

		Edit<Entity>::Ptr edit = std::make_shared<Edit<Entity>>(initialValue);
		Edit<Entity>::WeakPtr editWeakPtr = edit;

		Edit<Entity::EntityId>::Ptr editAngle = FillEdit<Entity::EntityId>::Call(innerLayout, "id", initialValue.getId());
		editAngle->bindOnChange([editWeakPtr](float /*oldValue*/, Entity::EntityId newValue, bool)
		{
			if (Edit<Entity>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(Entity(newValue));
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
