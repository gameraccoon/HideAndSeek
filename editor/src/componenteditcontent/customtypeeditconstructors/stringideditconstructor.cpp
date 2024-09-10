#include <string>

#include <QCheckBox>
#include <QHBoxLayout>

#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

namespace TypesEditConstructor
{
	template<>
	Edit<StringId>::Ptr FillEdit<StringId>::Call(QLayout* layout, const QString& label, const StringId& initialValue)
	{
		FillLabel(layout, label);

		QHBoxLayout* innerLayout = HS_NEW QHBoxLayout;

		Edit<StringId>::Ptr edit = std::make_shared<Edit<StringId>>(initialValue);
		Edit<StringId>::WeakPtr editWeakPtr = edit;

		const Edit<std::string>::Ptr editX = FillEdit<std::string>::Call(innerLayout, "id", ID_TO_STR(initialValue));
		editX->bindOnChange([editWeakPtr](std::string /*oldValue*/, std::string newValue, bool) {
			if (const Edit<StringId>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(STR_TO_ID(newValue));
			}
		});
		edit->addChild(editX);

		innerLayout->addStretch();
		QWidget* container = HS_NEW QWidget();
		container->setLayout(innerLayout);
		layout->addWidget(container);
		return edit;
	}
} // namespace TypesEditConstructor
