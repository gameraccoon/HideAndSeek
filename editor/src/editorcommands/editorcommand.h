#pragma once

#include "../editorutils/editoridgenerator.h"

#include "EngineCommon/Types/ComplexTypes/EnumBitset.h"

class MainWindow;
class World;

struct CommandExecutionContext
{
	World* world;
	std::reference_wrapper<EditorIdGenerator> editorIdGenerator;

	EditorIdGenerator& getEditorIdGenerator() { return editorIdGenerator.get(); }
};

class EditorCommand
{
public:
	enum class EffectType
	{
		Entities,
		EntityLocations,
		Components,
		ComponentAttributes,
		CommandStack,
		SkipLayoutUpdate
	};

	using EffectBitset = EnumBitset<EffectType>;

public:
	EditorCommand(EffectBitset effects)
		: mEffects(effects)
	{}
	virtual ~EditorCommand() = default;

	virtual void doCommand(CommandExecutionContext& context) = 0;
	virtual void undoCommand(CommandExecutionContext& context) = 0;
	EffectBitset getEffects() const { return mEffects; }

private:
	EffectBitset mEffects;
};
