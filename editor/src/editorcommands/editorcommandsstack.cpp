#include "editorcommandsstack.h"


void EditorCommandsStack::undo(CommandExecutionContext context)
{
	if (haveSomethingToUndo())
	{
		mCommands[static_cast<size_t>(mCurrentHeadIndex)]->undoCommand(context);
		EditorCommand::EffectBitset effects = mCommands[static_cast<size_t>(mCurrentHeadIndex)]->getEffects();
		--mCurrentHeadIndex;

		mLastExecutedCommandIdx = mCurrentHeadIndex;
		mIsLastExecutedUndo = true;

		if (mChangeHandler)
		{
			mChangeHandler(effects, false);
		}
	}
}

void EditorCommandsStack::redo(CommandExecutionContext context)
{
	if (haveSomethingToRedo())
	{
		++mCurrentHeadIndex;
		mCommands[static_cast<size_t>(mCurrentHeadIndex)]->doCommand(context);
		EditorCommand::EffectBitset effects = mCommands[static_cast<size_t>(mCurrentHeadIndex)]->getEffects();

		mLastExecutedCommandIdx = mCurrentHeadIndex;
		mIsLastExecutedUndo = false;

		if (mChangeHandler)
		{
			mChangeHandler(effects, false);
		}
	}
}

bool EditorCommandsStack::haveSomethingToUndo() const
{
	return mCurrentHeadIndex >= 0;
}

bool EditorCommandsStack::haveSomethingToRedo() const
{
	return mCurrentHeadIndex < static_cast<int>(mCommands.size()) - 1;
}

void EditorCommandsStack::clear()
{
	mCommands.clear();
	mCurrentHeadIndex = -1;
}

void EditorCommandsStack::bindFunctionToCommandChange(OnChangeFn handler)
{
	mChangeHandler = handler;
}

std::weak_ptr<const EditorCommand> EditorCommandsStack::getLastExecutedCommand() const
{
	if (mLastExecutedCommandIdx >= 0 && mLastExecutedCommandIdx < static_cast<int>(mCommands.size()))
	{
		return mCommands[mLastExecutedCommandIdx];
	}
	else
	{
		return std::weak_ptr<const EditorCommand>();
	}
}

bool EditorCommandsStack::isLastExecutedUndo() const
{
	return mIsLastExecutedUndo;
}

void EditorCommandsStack::clearOldCommands()
{
	mCommands.erase(mCommands.begin(), mCommands.begin() + mClearLag);
	mCurrentHeadIndex -= mClearLag;

	if (mCurrentHeadIndex < -1)
	{
		mCurrentHeadIndex = -1;
	}

	if (mChangeHandler)
	{
		mChangeHandler(EditorCommand::EffectBitset(EditorCommand::EffectType::CommandStack), false);
	}
}
