#pragma once

// to be able to change behavior for tests
inline std::function<void()> GlobalAssertHandler = [](){};
inline std::function<void()> GlobalFatalAssertHandler = [](){ std::terminate(); };

#ifdef DEBUG_CHECKS
	#define ReportError(...) \
		do \
		{ \
			LogAssertHelper("false", __FILE__, __LINE__, ##__VA_ARGS__); \
			GlobalAssertHandler(); \
		} while(0)
#else
	#define ReportError(...) do { } while(0)
#endif

#ifdef DEBUG_CHECKS
	#define ReportFatalError(...) \
		do \
		{ \
			LogAssertHelper("false", __FILE__, __LINE__, ##__VA_ARGS__); \
			GlobalFatalAssertHandler(); \
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
			GlobalAssertHandler(); \
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
			GlobalFatalAssertHandler(); \
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
		GlobalAssertHandler(); \
	} \
} while(0)
