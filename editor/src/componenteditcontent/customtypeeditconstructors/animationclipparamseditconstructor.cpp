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
	Edit<AnimationClipParams>::Ptr FillEdit<AnimationClipParams>::Call(QLayout* layout, const QString& label, const AnimationClipParams& initialValue)
	{
		FillLabel(layout, label);

		using EditType = Edit<AnimationClipParams>;

		EditType::Ptr edit = std::make_shared<EditType>(initialValue);
		EditType::WeakPtr editWeakPtr = edit;

		{
			Edit<bool>::Ptr editIsLooped = FillEdit<bool>::Call(layout, "is looped", initialValue.isLooped);
			editIsLooped->bindOnChange([editWeakPtr](bool /*oldValue*/, bool newValue, bool)
			{
				if (EditType::Ptr edit = editWeakPtr.lock())
				{
					AnimationClipParams animDescription = edit->getPreviousValue();
					animDescription.isLooped = newValue;
					edit->transmitValueChange(animDescription);
				}
			});
			edit->addChild(editIsLooped);
		}

		{
			Edit<float>::Ptr editSpeed = FillEdit<float>::Call(layout, "speed", initialValue.speed);
			editSpeed->bindOnChange([editWeakPtr](float /*oldValue*/, float newValue, bool)
			{
				if (EditType::Ptr edit = editWeakPtr.lock())
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
}
