#include <QHBoxLayout>
#include <QLineEdit>

#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

namespace TypesEditConstructor
{
	template<>
	Edit<Vector2D>::Ptr FillEdit<Vector2D>::Call(QLayout* layout, const QString& label, const Vector2D& initialValue)
	{
		FillLabel(layout, label);

		QHBoxLayout* innerLayout = HS_NEW QHBoxLayout;

		Edit<Vector2D>::Ptr edit = std::make_shared<Edit<Vector2D>>(initialValue);
		Edit<Vector2D>::WeakPtr editWeakPtr = edit;

		const Edit<float>::Ptr editX = FillEdit<float>::Call(innerLayout, "x", initialValue.x);
		editX->bindOnChange([editWeakPtr](float /*oldValue*/, const float newValue, bool) {
			if (const Edit<Vector2D>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(Vector2D(newValue, edit->getPreviousValue().y));
			}
		});
		edit->addChild(editX);

		const Edit<float>::Ptr editY = FillEdit<float>::Call(innerLayout, "y", initialValue.y);
		editY->bindOnChange([editWeakPtr](float /*oldValue*/, const float newValue, bool) {
			if (const Edit<Vector2D>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(Vector2D(edit->getPreviousValue().x, newValue));
			}
		});
		edit->addChild(editY);

		innerLayout->addStretch();
		QWidget* container = HS_NEW QWidget();
		container->setLayout(innerLayout);
		layout->addWidget(container);
		return edit;
	}
} // namespace TypesEditConstructor
