#ifndef COMPONENTCONTENTFABRIC_H
#define COMPONENTCONTENTFABRIC_H

#include <map>
#include <memory>

#include <QLayout>

#include "abstracteditfactory.h"
#include "src/editorcommands/editorcommandsstack.h"

struct ComponentSourceReference;

class ComponentContentFactory
{
public:
	void registerComponents();
	void replaceEditContent(QLayout* layout, const ComponentSourceReference& sourceReference, const BaseComponent* component, EditorCommandsStack& commandStack, World* world);
	void removeEditContent(QLayout* layout);

private:
	QWidget* mContentWidget = nullptr;
	std::shared_ptr<EditData> mCurrentEdit;
	std::map<StringID, std::unique_ptr<AbstractEditFactory>> mFactories;
};

#endif // COMPONENTCONTENTFABRIC_H
