#pragma once

#include <map>
#include <memory>

#include "abstracteditfactory.h"
#include "src/editorcommands/editorcommandsstack.h"

#include "GameData/EcsDefinitions.h"

struct ComponentSourceReference;
class QLayout;

class ComponentContentFactory
{
public:
	void registerComponents();
	void replaceEditContent(QLayout* layout, const ComponentSourceReference& sourceReference, TypedComponent componentData, EditorCommandsStack& commandStack, CommandExecutionContext& context);
	void removeEditContent(QLayout* layout);
	[[nodiscard]]
	bool isComponentEditable(StringId componentType) const;

private:
	QWidget* mContentWidget = nullptr;
	std::shared_ptr<EditData> mCurrentEdit;
	std::map<StringId, std::unique_ptr<AbstractEditFactory>> mFactories;
};
