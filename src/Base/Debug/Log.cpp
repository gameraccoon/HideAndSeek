#include "Base/precomp.h"

#include <chrono>
#include <iomanip>
#include <filesystem>
#include <iostream>

Log::Log()
{
	const std::filesystem::path LOGS_DIR{"./logs"};
	const std::filesystem::path LOG_FILE = LOGS_DIR / "log.txt";

	namespace fs = std::filesystem;
	if (!fs::is_directory(LOGS_DIR) || !fs::exists(LOGS_DIR))
	{
		fs::create_directory(LOGS_DIR);
	}

	mLogFileStream = std::ofstream(LOG_FILE);
	writeLog("Log file created");
}

Log::~Log()
{
	writeLog("End of log");
	mLogFileStream.close();
}

Log& Log::Instance()
{
	static Log log;
	return log;
}

void Log::writeError(const std::string& text)
{
	writeLine(std::string(" Error: ").append(text));
	mLogFileStream << std::flush;
}

void Log::writeWarning(const std::string& text)
{
	writeLine(std::string(" Warning: ").append(text));
	mLogFileStream << std::flush;
}

void Log::writeLog(const std::string& text)
{
	writeLine(std::string(" Log: ").append(text));
	mLogFileStream << std::flush;
}

void Log::writeInit(const std::string& text)
{
	writeLine(std::string(" Init: ").append(text));
	mLogFileStream << std::flush;
}

void Log::writeLine(const std::string& text)
{
	if (mLogFileStream.is_open())
	{
		auto now = std::chrono::system_clock::now();
		auto in_time_t = std::chrono::system_clock::to_time_t(now);

		mLogFileStream << std::put_time(std::localtime(&in_time_t), "%Y-%m-%d %X");
		mLogFileStream << text << "\n";
	}

	std::clog << text << "\n";
}
