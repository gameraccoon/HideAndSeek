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
	Edit<SpriteDescription>::Ptr FillEdit<SpriteDescription>::Call(QLayout* layout, const QString& label, const SpriteDescription& initialValue)
	{
		FillLabel(layout, label);

		Edit<SpriteDescription>::Ptr edit = std::make_shared<Edit<SpriteDescription>>(initialValue);
		Edit<SpriteDescription>::WeakPtr editWeakPtr = edit;
		{
			Edit<ResourcePath>::Ptr editPath = FillEdit<ResourcePath>::Call(layout, "path", initialValue.path);
			editPath->bindOnChange([editWeakPtr](const ResourcePath& /*oldValue*/, const ResourcePath& newValue, bool)
			{
				if (Edit<SpriteDescription>::Ptr edit = editWeakPtr.lock())
				{
					SpriteDescription animDescription = edit->getPreviousValue();
					animDescription.path = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editPath);
		}
		{
			Edit<SpriteParams>::Ptr editAnchor = FillEdit<SpriteParams>::Call(layout, "anchor", initialValue.params);
			editAnchor->bindOnChange([editWeakPtr](const SpriteParams& /*oldValue*/, const SpriteParams& newValue, bool)
			{
				if (Edit<SpriteDescription>::Ptr edit = editWeakPtr.lock())
				{
					SpriteDescription animDescription = edit->getPreviousValue();
					animDescription.params = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editAnchor);
		}

		return edit;
	}
}
