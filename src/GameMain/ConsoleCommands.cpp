#include "EngineCommon/precomp.h"

#include "GameMain/ConsoleCommands.h"

#include <algorithm>
#include <iostream>

#include "GameUtils/Application/ArgumentsParser.h"

namespace ConsoleCommands
{
	struct CommandInfo
	{
		std::string command;
		std::string syntax;
		std::string description;
	};

	bool TryExecuteQuickConsoleCommands(const ArgumentsParser& arguments)
	{
		const std::vector<CommandInfo> commands {
			{ "world", "world file", "start the game from the given world" },
			{ "gameData", "gameData path", "start the game with loading the given game data" },
			{ "disable-input", "disable-input", "disable keyboard/mouse/controller input" },
			{ "time-limit", "time-limit n", "quit the game after the given amount of frames" },
			{ "no-render", "no-render", "disable rendering/sound/input and creting a window" },
			{ "record-input", "record-input file", "record input into a file" },
			{ "replay-input", "replay-input file", "replay previously recorded input from a file" },
			{ "continue-after-input-end", "continue-after-input-end", "When replaying input, don't quit after recording is finished" },
			{ "randseed", "randseed n", "seed the gameplay random with the given seed" },
			{ "threads-count", "threads-count n", "set amount of extra worker threads to n" },
			{ "help", "help", "show this help" },
		};

		// check that the given commands are valid
		for (const std::string& argument : arguments.getAllArguments())
		{
			if (std::ranges::find_if(commands, [&argument](const CommandInfo& commandInfo)
			{
				return commandInfo.command == argument;
			}) == commands.end())
			{
				std::cout << "Command --" << argument << " not found, use --help to see the list of available commands" << std::endl;
				return true;
			}
		}

		if (arguments.hasArgument("help"))
		{
			std::cout << "Options:";

			const size_t longestCommand = std::ranges::max_element(commands,
				[](const CommandInfo& a, const CommandInfo& b){
					return a.syntax.size() < b.syntax.size();
				}
			)->syntax.size();

			for (const CommandInfo& commandInfo : commands)
			{
				std::cout << "\n " << arguments.getArgumentSwitch() << commandInfo.syntax;
				std::cout << std::string(longestCommand - commandInfo.syntax.size() + 1, ' ');
				std::cout << commandInfo.description;
			}
			std::cout << std::endl;
			return true;
		}

		return false;
	}
} // namespace ConsoleCommands
