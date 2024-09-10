#include <QCheckBox>

#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

namespace TypesEditConstructor
{
	template<>
	Edit<Hull>::Ptr FillEdit<Hull>::Call(QLayout* layout, const QString& label, const Hull& initialValue)
	{
		FillLabel(layout, label);

		Edit<Hull>::Ptr edit = std::make_shared<Edit<Hull>>(initialValue);
		Edit<Hull>::WeakPtr editWeakPtr = edit;

		const Edit<HullType>::Ptr editType = FillEdit<HullType>::Call(layout, "type", initialValue.type);
		editType->bindOnChange([editWeakPtr](HullType /*oldValue*/, const HullType newValue, bool) {
			if (const Edit<Hull>::Ptr edit = editWeakPtr.lock())
			{
				Hull hull = edit->getPreviousValue();
				hull.type = newValue;
				edit->transmitValueChange(hull);
			}
		});
		edit->addChild(editType);

		const Edit<float>::Ptr editRadius = FillEdit<float>::Call(layout, "radius", initialValue.getRadius());
		editRadius->bindOnChange([editWeakPtr](float /*oldValue*/, const float newValue, bool) {
			if (const Edit<Hull>::Ptr edit = editWeakPtr.lock())
			{
				Hull hull = edit->getPreviousValue();
				hull.setRadius(newValue);
				edit->transmitValueChange(hull);
			}
		});
		edit->addChild(editRadius);

		const Edit<std::vector<Vector2D>>::Ptr editPoints = FillEdit<std::vector<Vector2D>>::Call(layout, "points", initialValue.points);
		editPoints->bindOnChange([editWeakPtr](const std::vector<Vector2D>& /*oldValue*/, const std::vector<Vector2D>& newValue, const bool needLayoutUpdate) {
			if (const Edit<Hull>::Ptr edit = editWeakPtr.lock())
			{
				Hull hull = edit->getPreviousValue();
				hull.points = newValue;
				hull.generateBorders();
				edit->transmitValueChange(hull, needLayoutUpdate);
			}
		});
		edit->addChild(editPoints);

		return edit;
	}
} // namespace TypesEditConstructor
