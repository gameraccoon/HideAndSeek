#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

namespace TypesEditConstructor
{
	template<>
	Edit<SimpleBorder>::Ptr FillEdit<SimpleBorder>::Call(QLayout* layout, const QString& label, const SimpleBorder& initialValue)
	{
		FillLabel(layout, label);

		Edit<SimpleBorder>::Ptr edit = std::make_shared<Edit<SimpleBorder>>(initialValue);
		Edit<SimpleBorder>::WeakPtr editWeakPtr = edit;

		const Edit<Vector2D>::Ptr editA = FillEdit<Vector2D>::Call(layout, "a", initialValue.a);
		editA->bindOnChange([editWeakPtr](Vector2D /*oldValue*/, const Vector2D newValue, bool) {
			if (const Edit<SimpleBorder>::Ptr edit = editWeakPtr.lock())
			{
				SimpleBorder border = edit->getPreviousValue();
				border.a = newValue;
				edit->transmitValueChange(border);
			}
		});
		edit->addChild(editA);

		const Edit<Vector2D>::Ptr editB = FillEdit<Vector2D>::Call(layout, "b", initialValue.b);
		editB->bindOnChange([editWeakPtr](Vector2D /*oldValue*/, const Vector2D newValue, bool) {
			if (const Edit<SimpleBorder>::Ptr edit = editWeakPtr.lock())
			{
				SimpleBorder border = edit->getPreviousValue();
				border.b = newValue;
				edit->transmitValueChange(border);
			}
		});
		edit->addChild(editB);

		return edit;
	}
} // namespace TypesEditConstructor
