#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

namespace TypesEditConstructor
{
	template<>
	Edit<AnimationClipParams>::Ptr FillEdit<AnimationClipParams>::Call(QLayout* layout, const QString& label, const AnimationClipParams& initialValue)
	{
		FillLabel(layout, label);

		using EditType = Edit<AnimationClipParams>;

		EditType::Ptr edit = std::make_shared<EditType>(initialValue);
		EditType::WeakPtr editWeakPtr = edit;

		{
			const Edit<bool>::Ptr editIsLooped = FillEdit<bool>::Call(layout, "is looped", initialValue.isLooped);
			editIsLooped->bindOnChange([editWeakPtr](bool /*oldValue*/, const bool newValue, bool) {
				if (const EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipParams animDescription = edit->getPreviousValue();
					animDescription.isLooped = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editIsLooped);
		}

		{
			const Edit<float>::Ptr editSpeed = FillEdit<float>::Call(layout, "speed", initialValue.speed);
			editSpeed->bindOnChange([editWeakPtr](float /*oldValue*/, const float newValue, bool) {
				if (const EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipParams animDescription = edit->getPreviousValue();
					animDescription.speed = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editSpeed);
		}

		return edit;
	}
} // namespace TypesEditConstructor
