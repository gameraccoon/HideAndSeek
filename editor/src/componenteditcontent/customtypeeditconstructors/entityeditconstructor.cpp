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

		Edit<RaccoonEcs::Entity::RawId>::Ptr editRawId = FillEdit<RaccoonEcs::Entity::RawId>::Call(innerLayout, "rawId", initialValue.getRawId());
		editRawId->bindOnChange([editWeakPtr](RaccoonEcs::Entity::RawId /*oldValue*/, RaccoonEcs::Entity::RawId newValue, bool)
		{
			if (Edit<RaccoonEcs::Entity>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(RaccoonEcs::Entity(newValue, edit->getPreviousValue().getVersion()));
			}
		});
		edit->addChild(editRawId);

		Edit<RaccoonEcs::Entity::Version>::Ptr editVersion = FillEdit<RaccoonEcs::Entity::Version>::Call(innerLayout, "version", initialValue.getVersion());
		editVersion->bindOnChange([editWeakPtr](RaccoonEcs::Entity::Version /*oldValue*/, RaccoonEcs::Entity::Version newValue, bool)
		{
			if (Edit<RaccoonEcs::Entity>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(RaccoonEcs::Entity(edit->getPreviousValue().getRawId(), newValue));
			}
		});
		edit->addChild(editVersion);

		innerLayout->addStretch();
		QWidget* container = HS_NEW QWidget();
		container->setLayout(innerLayout);
		layout->addWidget(container);
		return edit;
	}
}
