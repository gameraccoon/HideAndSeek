#include <QHBoxLayout>

#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

namespace TypesEditConstructor
{
	template<>
	Edit<RaccoonEcs::Entity>::Ptr FillEdit<RaccoonEcs::Entity>::Call(QLayout* layout, const QString& label, const RaccoonEcs::Entity& initialValue)
	{
		FillLabel(layout, label);

		QHBoxLayout* innerLayout = HS_NEW QHBoxLayout;

		Edit<RaccoonEcs::Entity>::Ptr edit = std::make_shared<Edit<RaccoonEcs::Entity>>(initialValue);
		Edit<RaccoonEcs::Entity>::WeakPtr editWeakPtr = edit;

		const Edit<RaccoonEcs::Entity::RawId>::Ptr editRawId = FillEdit<RaccoonEcs::Entity::RawId>::Call(innerLayout, "rawId", initialValue.getRawId());
		editRawId->bindOnChange([editWeakPtr](RaccoonEcs::Entity::RawId /*oldValue*/, const RaccoonEcs::Entity::RawId newValue, bool) {
			if (const Edit<RaccoonEcs::Entity>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(RaccoonEcs::Entity(newValue, edit->getPreviousValue().getVersion()));
			}
		});
		edit->addChild(editRawId);

		const Edit<RaccoonEcs::Entity::Version>::Ptr editVersion = FillEdit<RaccoonEcs::Entity::Version>::Call(innerLayout, "version", initialValue.getVersion());
		editVersion->bindOnChange([editWeakPtr](RaccoonEcs::Entity::Version /*oldValue*/, const RaccoonEcs::Entity::Version newValue, bool) {
			if (const Edit<RaccoonEcs::Entity>::Ptr edit = editWeakPtr.lock())
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
} // namespace TypesEditConstructor
