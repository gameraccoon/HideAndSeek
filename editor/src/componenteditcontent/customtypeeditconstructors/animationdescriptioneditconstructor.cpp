#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

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
			const Edit<RelativeResourcePath>::Ptr editPath = FillEdit<RelativeResourcePath>::Call(layout, "path", initialValue.path);
			editPath->bindOnChange([editWeakPtr](const RelativeResourcePath& /*oldValue*/, const RelativeResourcePath& newValue, bool) {
				if (const EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipDescription animDescription = edit->getPreviousValue();
					animDescription.path = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editPath);
		}

		{
			const Edit<AnimationClipParams>::Ptr editIsLooped = FillEdit<AnimationClipParams>::Call(layout, "clip parameters", initialValue.params);
			editIsLooped->bindOnChange([editWeakPtr](AnimationClipParams /*oldValue*/, const AnimationClipParams newValue, bool) {
				if (const EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipDescription animDescription = edit->getPreviousValue();
					animDescription.params = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editIsLooped);
		}

		{
			const Edit<Vector2D>::Ptr editAnchor = FillEdit<Vector2D>::Call(layout, "anchor", initialValue.spriteParams.anchor);
			editAnchor->bindOnChange([editWeakPtr](const Vector2D& /*oldValue*/, const Vector2D& newValue, bool) {
				if (const EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipDescription animDescription = edit->getPreviousValue();
					animDescription.spriteParams.anchor = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editAnchor);
		}

		{
			const Edit<Vector2D>::Ptr editSize = FillEdit<Vector2D>::Call(layout, "size", initialValue.spriteParams.size);
			editSize->bindOnChange([editWeakPtr](const Vector2D& /*oldValue*/, const Vector2D& newValue, bool) {
				if (const EditType::Ptr edit = editWeakPtr.lock())
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
} // namespace TypesEditConstructor
