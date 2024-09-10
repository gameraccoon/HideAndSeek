#include <QCheckBox>

#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

namespace TypesEditConstructor
{
	template<>
	Edit<SpriteDescription>::Ptr FillEdit<SpriteDescription>::Call(QLayout* layout, const QString& label, const SpriteDescription& initialValue)
	{
		FillLabel(layout, label);

		Edit<SpriteDescription>::Ptr edit = std::make_shared<Edit<SpriteDescription>>(initialValue);
		Edit<SpriteDescription>::WeakPtr editWeakPtr = edit;
		{
			const Edit<RelativeResourcePath>::Ptr editPath = FillEdit<RelativeResourcePath>::Call(layout, "path", initialValue.path);
			editPath->bindOnChange([editWeakPtr](const RelativeResourcePath& /*oldValue*/, const RelativeResourcePath& newValue, bool) {
				if (const Edit<SpriteDescription>::Ptr edit = editWeakPtr.lock())
				{
					SpriteDescription animDescription = edit->getPreviousValue();
					animDescription.path = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editPath);
		}
		{
			const Edit<SpriteParams>::Ptr editAnchor = FillEdit<SpriteParams>::Call(layout, "anchor", initialValue.params);
			editAnchor->bindOnChange([editWeakPtr](const SpriteParams& /*oldValue*/, const SpriteParams& newValue, bool) {
				if (const Edit<SpriteDescription>::Ptr edit = editWeakPtr.lock())
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
} // namespace TypesEditConstructor
