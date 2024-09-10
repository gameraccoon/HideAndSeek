#include <QCheckBox>
#include <QHBoxLayout>

#include "src/componenteditcontent/customtypeeditconstructors/customtypeeditconstructors.h"

namespace TypesEditConstructor
{
	template<>
	Edit<SpatialEntity>::Ptr FillEdit<SpatialEntity>::Call(QLayout* layout, const QString& label, const SpatialEntity& initialValue)
	{
		FillLabel(layout, label);

		QHBoxLayout* innerLayout = HS_NEW QHBoxLayout;

		Edit<SpatialEntity>::Ptr edit = std::make_shared<Edit<SpatialEntity>>(initialValue);
		Edit<SpatialEntity>::WeakPtr editWeakPtr = edit;

		const Edit<Entity>::Ptr editX = FillEdit<Entity>::Call(innerLayout, "entity", initialValue.entity.getEntity());
		editX->bindOnChange([editWeakPtr](Entity /*oldValue*/, const Entity newValue, bool) {
			if (const Edit<SpatialEntity>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(SpatialEntity(newValue, edit->getPreviousValue().cell));
			}
		});
		edit->addChild(editX);

		const Edit<CellPos>::Ptr editY = FillEdit<CellPos>::Call(innerLayout, "cell", initialValue.cell);
		editY->bindOnChange([editWeakPtr](CellPos /*oldValue*/, const CellPos newValue, bool) {
			if (const Edit<SpatialEntity>::Ptr edit = editWeakPtr.lock())
			{
				edit->transmitValueChange(SpatialEntity(edit->getPreviousValue().entity.getEntity(), newValue));
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
