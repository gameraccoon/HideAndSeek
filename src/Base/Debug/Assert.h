#pragma once

// to be able to change behavior for tests
inline std::function<void()> gGlobalAssertHandler = [](){};
inline std::function<void()> gGlobalFatalAssertHandler = [](){ std::terminate(); };

inline bool gGlobalAllowAssertLogs = true;
template<typename... Args>
void LogAssertHelper(const char* condition, const char* file, size_t line, const std::string& message, Args... args) noexcept
{
	if (gGlobalAllowAssertLogs)
	{
		Log::Instance().writeError(FormatString(std::string("Assertion failed '%s' %s:%d with message: '").append(message).append("'"), condition, file, line, std::forward<Args>(args)...));
	}
}

#ifdef DEBUG_CHECKS
	#define ReportError(...) \
		do \
		{ \
			LogAssertHelper("false", __FILE__, __LINE__, ##__VA_ARGS__); \
			gGlobalAssertHandler(); \
		} while(0)
#else
	#define ReportError(...) do { } while(0)
#endif

#ifdef DEBUG_CHECKS
	#define ReportFatalError(...) \
		do \
		{ \
			LogAssertHelper("false", __FILE__, __LINE__, ##__VA_ARGS__); \
			gGlobalFatalAssertHandler(); \
		} while(0)
#else
	#define ReportFatalError(...) do { } while(0)
#endif

#ifdef DEBUG_CHECKS
	#define Assert(cond, ...) \
	do \
	{ \
		if ALMOST_NEVER(static_cast<bool>(cond) == false) \
		{ \
			LogAssertHelper(STR(cond), __FILE__, __LINE__, ##__VA_ARGS__); \
			gGlobalAssertHandler(); \
		} \
	} while(0)
#else
	#define Assert(...) do { } while(0)
#endif

#ifdef DEBUG_CHECKS
	#define AssertFatal(cond, ...) \
	do { \
		if ALMOST_NEVER(static_cast<bool>(cond) == false) \
		{ \
			LogAssertHelper(STR(cond), __FILE__, __LINE__, ##__VA_ARGS__); \
			gGlobalFatalAssertHandler(); \
		} \
	} while(0)
#else
	#define AssertFatal(...) do { } while(0)
#endif

#define AssertRelease(cond, ...) \
	do { \
	if ALMOST_NEVER(static_cast<bool>(cond) == false) \
	{ \
		LogAssertHelper("false", __FILE__, __LINE__, ##__VA_ARGS__); \
		gGlobalAssertHandler(); \
	} \
} while(0)
