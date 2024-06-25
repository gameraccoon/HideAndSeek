#pragma once

#ifdef ENABLE_SCOPED_PROFILER

#include <chrono>
#include <list>

class ScopedProfilerThreadData
{
public:
	ScopedProfilerThreadData(size_t eventsCount = 10000)
		: mRecords(eventsCount)
	{
	}

	struct ScopeRecord
	{
		std::chrono::time_point<std::chrono::system_clock> begin;
		std::chrono::time_point<std::chrono::system_clock> end;
		const char* scopeName = nullptr;
	};

	using Records = std::list<ScopeRecord>;

	void addRecord(
		std::chrono::time_point<std::chrono::system_clock>&& begin
		, std::chrono::time_point<std::chrono::system_clock>&& end
		, const char* scopeName
	)
	{
		mRecords.splice(mRecords.end(), mRecords, mRecords.begin());
		ScopeRecord& newRecord = mRecords.back();
		newRecord.begin = std::move(begin);
		newRecord.end = std::move(end);
		newRecord.scopeName = scopeName;
	}

	const Records& getAllRecords() const { return mRecords; }

private:
	Records mRecords;
};

thread_local inline ScopedProfilerThreadData gtlScopedProfilerData;

class ScopedProfiler
{
public:
	ScopedProfiler(const char* scopeName)
		: mScopeName(scopeName)
		, mStart(std::chrono::system_clock::now())
	{
	}

	~ScopedProfiler()
	{
		gtlScopedProfilerData.addRecord(
			std::move(mStart)
			, std::chrono::system_clock::now()
			, mScopeName
		);
	}

private:
	const char* mScopeName;
	std::chrono::time_point<std::chrono::system_clock> mStart;
};

#define SCOPED_PROFILER_NAME(A,B) A##B
#define SCOPED_PROFILER_IMPL(scopeName, namePostfix) ScopedProfiler SCOPED_PROFILER_NAME(cadg_inst_, namePostfix){(scopeName)}
// macro generates a unique instance name for us
#define SCOPED_PROFILER(scopeName) SCOPED_PROFILER_IMPL((scopeName), __COUNTER__)
#else
#define SCOPED_PROFILER(scopeName)
#endif
