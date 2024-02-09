#include "componentcontentfactory.h"

#include <QPushButton>
#include <QSpacerItem>

#include "componentregistration.h"

#include "src/editorutils/componentreference.h"

void ComponentContentFactory::registerComponents()
{
	ComponentRegistration::RegisterToEditFactory(mFactories);
}

void ComponentContentFactory::replaceEditContent(QLayout* layout, const ComponentSourceReference& sourceReference, TypedComponent componentData, EditorCommandsStack& commandStack, CommandExecutionContext& context)
{
	auto it = mFactories.find(componentData.typeId);

	QWidget* newContent = nullptr;
	if (it != mFactories.end())
	{
		mCurrentEdit = it->second->getEditData();
		newContent = HS_NEW QWidget();
		QVBoxLayout* innerLayout = HS_NEW QVBoxLayout();
		mCurrentEdit->fillContent(innerLayout, sourceReference, componentData.component, commandStack, context);
		innerLayout->addStretch();
		newContent->setLayout(innerLayout);
	}
	else
	{
		mCurrentEdit = nullptr;
		ReportError("ComponentEditFactory not registered for component type '%s'", componentData.typeId);
	}

	if (newContent != nullptr && mContentWidget == nullptr)
	{
		layout->addWidget(newContent);
		newContent->show();
	}
	else if (newContent != nullptr && mContentWidget != nullptr)
	{
		mContentWidget->hide();
		layout->replaceWidget(mContentWidget, newContent);
		newContent->show();
		delete mContentWidget;
	}
	else if (mContentWidget != nullptr)
	{
		mContentWidget->hide();
		layout->removeWidget(mContentWidget);
		delete mContentWidget;
	}

	mContentWidget = newContent;
}

void ComponentContentFactory::removeEditContent(QLayout* layout)
{
	if (mContentWidget)
	{
		mContentWidget->hide();
		layout->removeWidget(mContentWidget);
		delete mContentWidget;
		mContentWidget = nullptr;
	}
}

bool ComponentContentFactory::isComponentEditable(StringId componentType) const
{
	return mFactories.find(componentType) != mFactories.end();
}
