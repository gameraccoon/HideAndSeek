#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

#include <string>

#include <QLabel>
#include <QLineEdit>
#include <QDoubleValidator>
#include <QCheckBox>
#include <QHBoxLayout>
#include <QPushButton>

namespace TypesEditConstructor
{
	template<>
	Edit<Hull>::Ptr FillEdit<Hull>::Call(QLayout* layout, const QString& label, const Hull& initialValue)
	{
		FillLabel(layout, label);

		Edit<Hull>::Ptr edit = std::make_shared<Edit<Hull>>(initialValue);
		Edit<Hull>::WeakPtr editWeakPtr = edit;

		Edit<HullType>::Ptr editType = FillEdit<HullType>::Call(layout, "type", initialValue.type);
		editType->bindOnChange([editWeakPtr](HullType /*oldValue*/, HullType newValue, bool)
		{
			if (Edit<Hull>::Ptr edit = editWeakPtr.lock())
			{
				Hull hull = edit->getPreviousValue();
				hull.type = newValue;
				edit->transmitValueChange(hull);
			}
		});
		edit->addChild(editType);

		Edit<float>::Ptr editRadius = FillEdit<float>::Call(layout, "radius", initialValue.getRadius());
		editRadius->bindOnChange([editWeakPtr](float /*oldValue*/, float newValue, bool)
		{
			if (Edit<Hull>::Ptr edit = editWeakPtr.lock())
			{
				Hull hull = edit->getPreviousValue();
				hull.setRadius(newValue);
				edit->transmitValueChange(hull);
			}
		});
		edit->addChild(editRadius);

		Edit<std::vector<Vector2D>>::Ptr editPoints = FillEdit<std::vector<Vector2D>>::Call(layout, "points", initialValue.points);
		editPoints->bindOnChange([editWeakPtr](const std::vector<Vector2D>& /*oldValue*/, const std::vector<Vector2D>& newValue, bool needLayoutUpdate)
		{
			if (Edit<Hull>::Ptr edit = editWeakPtr.lock())
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
}
