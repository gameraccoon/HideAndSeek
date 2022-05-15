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
	Edit<SpriteParams>::Ptr FillEdit<SpriteParams>::Call(QLayout* layout, const QString& label, const SpriteParams& initialValue)
	{
		FillLabel(layout, label);

		using EditType = Edit<SpriteParams>;

		EditType::Ptr edit = std::make_shared<EditType>(initialValue);
		EditType::WeakPtr editWeakPtr = edit;

		{
			Edit<Vector2D>::Ptr editAnchor = FillEdit<Vector2D>::Call(layout, "anchor", initialValue.anchor);
			editAnchor->bindOnChange([editWeakPtr](const Vector2D& /*oldValue*/, const Vector2D& newValue, bool)
			{
				if (Edit<SpriteParams>::Ptr edit = editWeakPtr.lock())
				{
					SpriteParams animDescription = edit->getPreviousValue();
					animDescription.anchor = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editAnchor);
		}

		{
			Edit<Vector2D>::Ptr editSize = FillEdit<Vector2D>::Call(layout, "size", initialValue.size);
			editSize->bindOnChange([editWeakPtr](const Vector2D& /*oldValue*/, const Vector2D& newValue, bool)
			{
				if (Edit<SpriteParams>::Ptr edit = editWeakPtr.lock())
				{
					SpriteParams animDescription = edit->getPreviousValue();
					animDescription.size = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editSize);
		}

		return edit;
	}
}
