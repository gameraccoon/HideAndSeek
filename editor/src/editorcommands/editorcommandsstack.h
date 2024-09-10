#pragma once

#include <functional>

#include "editorcommand.h"
#include <raccoon-ecs/delegates.h>

class EditorCommandsStack
{
public:
	using OnChangeFn = std::function<void(EditorCommand::EffectBitset, bool)>;

public:
	template<typename T, typename... Args>
	void executeNewCommand(CommandExecutionContext context, Args&&... args)
	{
		// clear old redo commands
		if (haveSomethingToRedo())
		{
			mCommands.erase(mCommands.begin() + (mCurrentHeadIndex + 1), mCommands.end());
		}

		// add and activate
		mCommands.emplace_back(std::make_shared<T>(std::forward<Args>(args)...));
		mCommands.back()->doCommand(context);
		EditorCommand::EffectBitset effects = mCommands.back()->getEffects();
		++mCurrentHeadIndex;

		// clear old commands if exceed limits
		if (mCurrentHeadIndex > mMaxHistorySize + mClearLag)
		{
			clearOldCommands();
		}

		mLastExecutedCommandIdx = mCurrentHeadIndex;
		mIsLastExecutedUndo = false;

		if (mChangeHandler)
		{
			mChangeHandler(effects, true);
		}
	}

	void undo(CommandExecutionContext context);
	void redo(CommandExecutionContext context);
	bool haveSomethingToUndo() const;
	bool haveSomethingToRedo() const;
	void clear();
	void bindFunctionToCommandChange(OnChangeFn handler);

	std::weak_ptr<const EditorCommand> getLastExecutedCommand() const;
	bool isLastExecutedUndo() const;

private:
	void clearOldCommands();

private:
	std::vector<std::shared_ptr<EditorCommand>> mCommands;
	int mCurrentHeadIndex = -1;
	static constexpr int mMaxHistorySize = 10000;
	// how ofter commands will be cleaned after reaching the limit
	static constexpr int mClearLag = 100;
	OnChangeFn mChangeHandler;

	int mLastExecutedCommandIdx = -1;
	bool mIsLastExecutedUndo = false;

	RaccoonEcs::Delegates::Handle mOnCommandEffectHandle;
};
