#ifndef ABSTRACTEDITFACTORY_H
#define ABSTRACTEDITFACTORY_H

#include <memory>

#include <QObject>

#include "ECS/Entity.h"
#include "ECS/Component.h"

#include "src/editorcommands/editorcommandsstack.h"

class QWidget;
class QLayout;
class ComponentSourceReference;

class EditData : public QObject
{
public:
	virtual ~EditData() = default;
	virtual void fillContent(QLayout* layout, const ComponentSourceReference& sourceReference, const BaseComponent* component, EditorCommandsStack& commandStack, World* world) = 0;
};

class AbstractEditFactory
{
public:
	virtual ~AbstractEditFactory() = default;
	virtual std::shared_ptr<EditData> getEditData() = 0;
};

#endif // ABSTRACTEDITFACTORY_H
