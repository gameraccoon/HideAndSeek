#include "Base/precomp.h"

#include <chrono>
#include <iomanip>
#include <filesystem>
#include <iostream>

Log::Log()
{
	const std::string LOG_FILE = std::string("./logs/").append("log.txt");

	namespace fs = std::filesystem;
	if (!fs::is_directory("./logs") || !fs::exists("./logs"))
	{
		fs::create_directory("logs");
	}

	mLogFileStream = std::ofstream(LOG_FILE, std::ios_base::trunc);
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
}

void Log::writeInit(const std::string& text)
{
	writeLine(std::string(" Init: ").append(text));
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
