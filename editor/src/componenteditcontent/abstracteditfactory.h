#pragma once

#include <memory>

#include <QObject>

#include "src/editorcommands/editorcommandsstack.h"

class QWidget;
class QLayout;
struct ComponentSourceReference;

class EditData : public QObject
{
public:
	~EditData() override = default;
	virtual void fillContent(QLayout* layout, const ComponentSourceReference& sourceReference, const void* component, EditorCommandsStack& commandStack, CommandExecutionContext& context) = 0;
};

class AbstractEditFactory
{
public:
	virtual ~AbstractEditFactory() = default;
	virtual std::shared_ptr<EditData> getEditData() = 0;
};
