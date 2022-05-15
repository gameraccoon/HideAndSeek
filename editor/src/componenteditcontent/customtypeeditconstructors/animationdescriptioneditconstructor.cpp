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
	Edit<AnimationClipDescription>::Ptr FillEdit<AnimationClipDescription>::Call(QLayout* layout, const QString& label, const AnimationClipDescription& initialValue)
	{
		FillLabel(layout, label);

		using EditType = Edit<AnimationClipDescription>;

		EditType::Ptr edit = std::make_shared<EditType>(initialValue);
		EditType::WeakPtr editWeakPtr = edit;
		{
			Edit<ResourcePath>::Ptr editPath = FillEdit<ResourcePath>::Call(layout, "path", initialValue.path);
			editPath->bindOnChange([editWeakPtr](const ResourcePath& /*oldValue*/, const ResourcePath& newValue, bool)
			{
				if (EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipDescription animDescription = edit->getPreviousValue();
					animDescription.path = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editPath);
		}

		{
			Edit<AnimationClipParams>::Ptr editIsLooped = FillEdit<AnimationClipParams>::Call(layout, "clip parameters", initialValue.params);
			editIsLooped->bindOnChange([editWeakPtr](AnimationClipParams /*oldValue*/, AnimationClipParams newValue, bool)
			{
				if (EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipDescription animDescription = edit->getPreviousValue();
					animDescription.params = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editIsLooped);
		}

		{
			Edit<Vector2D>::Ptr editAnchor = FillEdit<Vector2D>::Call(layout, "anchor", initialValue.spriteParams.anchor);
			editAnchor->bindOnChange([editWeakPtr](const Vector2D& /*oldValue*/, const Vector2D& newValue, bool)
			{
				if (EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipDescription animDescription = edit->getPreviousValue();
					animDescription.spriteParams.anchor = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editAnchor);
		}

		{
			Edit<Vector2D>::Ptr editSize = FillEdit<Vector2D>::Call(layout, "size", initialValue.spriteParams.size);
			editSize->bindOnChange([editWeakPtr](const Vector2D& /*oldValue*/, const Vector2D& newValue, bool)
			{
				if (EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipDescription animDescription = edit->getPreviousValue();
					animDescription.spriteParams.size = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editSize);
		}

		return edit;
	}
}
