#ifndef ABSTRACTEDITFACTORY_H
#define ABSTRACTEDITFACTORY_H

#include <memory>

#include <QObject>

#include "GameData/EcsDefinitions.h"

#include "src/editorcommands/editorcommandsstack.h"

class QWidget;
class QLayout;
struct ComponentSourceReference;

class EditData : public QObject
{
public:
	virtual ~EditData() = default;
	virtual void fillContent(QLayout* layout, const ComponentSourceReference& sourceReference, const void* component, EditorCommandsStack& commandStack, World* world) = 0;
};

class AbstractEditFactory
{
public:
	virtual ~AbstractEditFactory() = default;
	virtual std::shared_ptr<EditData> getEditData() = 0;
};

#endif // ABSTRACTEDITFACTORY_H
