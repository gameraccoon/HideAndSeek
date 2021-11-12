#pragma once

#ifdef RACCOON_ECS_PROFILE_SYSTEMS

#include <chrono>
#include <vector>

class ScopedProfilerThreadData
{
public:
	struct ScopeRecord
	{
		std::chrono::time_point<std::chrono::system_clock> start;
		std::chrono::time_point<std::chrono::system_clock> end;
		const char* scopeName;
		int stackDepth;
	};

	using Records = std::vector<ScopeRecord>;

	void addRecord(
		std::chrono::time_point<std::chrono::system_clock>&& start
		, std::chrono::time_point<std::chrono::system_clock>&& end
		, const char* scopeName
		, int stackDepth)
	{
		mRecords.emplace_back(std::move(start), std::move(end), scopeName, stackDepth);
	}

	const Records& getAllRecords() const { return mRecords; }
	Records consumeAllRecords() { return std::move(mRecords); }

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
		++tlScopedProfilerStackDepth;
	}

	~ScopedProfiler()
	{
		gtlScopedProfilerData.addRecord(
			std::move(mStart),
			std::chrono::system_clock::now(),
			mScopeName,
			tlScopedProfilerStackDepth
		);

		--tlScopedProfilerStackDepth;
	}

private:
	const char* mScopeName;
	std::chrono::time_point<std::chrono::system_clock> mStart;

	static thread_local inline int tlScopedProfilerStackDepth = 0;
};

#define SCOPED_PROFILER_NAME(A,B) A##B
#define SCOPED_PROFILER_IMPL(scopeName, namePostfix) ScopedProfiler SCOPED_PROFILER_NAME(cadg_inst_, namePostfix){(scopeName)}
// macro generates a unique instance name for us
#define SCOPED_PROFILER(scopeName) SCOPED_PROFILER_IMPL((scopeName), __COUNTER__)
#else
#define SCOPED_PROFILER
#endif
