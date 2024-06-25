#include "EngineCommon/precomp.h"

#include "EngineCommon/Debug/MemoryLeakDetection.h"

#ifdef DETECT_MEMORY_LEAKS

LeakDetectorClass::~LeakDetectorClass()
{
	printLeaks();

	freeAllocatedList();
	FreeLeaksList(mMemoryLeaksFirst);
}

void LeakDetectorClass::addAllocated(void* p, size_t size, const char* file, int line) noexcept
{
	std::lock_guard<std::recursive_mutex> g(mMutex);

	void* nextItemAddr = malloc(sizeof(AllocatedPtr));
	AllocatedPtr* nextItem = new (nextItemAddr) AllocatedPtr(p, size, file, line, mAllocatedList);
	mAllocatedList = nextItem;
}

void LeakDetectorClass::removeAllocated(void* p) noexcept
{
	std::lock_guard<std::recursive_mutex> g(mMutex);

	if (p == nullptr)
	{
		return;
	}

	if (mAllocatedList == nullptr)
	{
		return;
	}

	// first item needs a special treatment, since allocatedList points to it
	if (mAllocatedList->pointer == p)
	{
		AllocatedPtr* item = mAllocatedList;
		mAllocatedList = mAllocatedList->next;
		free(item);
		return;
	}

	AllocatedPtr* item = mAllocatedList;

	while (item->next != nullptr)
	{
		AllocatedPtr* next = item->next;
		if (next->pointer == p)
		{
			item->next = next->next;
			free(next);
			return;
		}
		item = next;
	}
}

void LeakDetectorClass::addLeakInfo(void* pointer, size_t size, const char* file, int line)
{
	void* nextItemAddr = malloc(sizeof(MemoryLeak));
	MemoryLeak* nextItem = new (nextItemAddr) MemoryLeak(pointer, size, file, line);

	if (mMemoryLeaksFirst == nullptr)
	{
		mMemoryLeaksFirst = nextItem;
	}

	if (mMemoryLeaksFirst != nullptr)
	{
		mMemoryLeaksFirst->next = nextItem;
	}
	mMemoryLeaksFirst = nextItem;
}

template<typename... Args>
static void writeOstreamLine(std::ofstream& ostream, Args... args)
{
	if (ostream.is_open())
	{
		((ostream << args), ...);
		ostream << "\n";
	}

	((std::clog << args), ...);
	std::clog << "\n";
}

void LeakDetectorClass::printLeaks() noexcept
{
	MemoryLeak* localLeaksList;
	{
		std::lock_guard<std::recursive_mutex> g(mMutex);

		AllocatedPtr* item = mAllocatedList;
		while (item != nullptr)
		{
			addLeakInfo(item->pointer, item->size, item->file, item->line);
			item = item->next;
		}

		// "move" the list to this function to ignore all the allocations happened in this function
		localLeaksList = mMemoryLeaksFirst;
		mMemoryLeaksFirst = nullptr;
		mMemoryLeaksFirst = nullptr;

		freeAllocatedList();
		mAllocatedList = nullptr;
	}

	{
		namespace fs = std::filesystem;
		if (!fs::is_directory("./logs") || !fs::exists("./logs"))
		{
			fs::create_directory("logs");
		}

		std::ofstream logFileStream("./logs/leaks_report.txt", std::ios_base::trunc);
		logFileStream << std::showbase;

		writeOstreamLine(logFileStream, "Memory leak report started");

		if (localLeaksList == nullptr)
		{
			writeOstreamLine(logFileStream, "No memory leaks detected");
		}

		MemoryLeak* item = localLeaksList;
		size_t leakedBytes = 0;
		while (item != nullptr)
		{
			writeOstreamLine(logFileStream, "Error: Leaked ", item->size, " bytes (", std::hex, item->pointer, std::dec,
				") allocated in ", item->file, "(", item->line, ")");
			leakedBytes += item->size;
			item = item->next;
		}

		if (leakedBytes > 0)
		{
			writeOstreamLine(logFileStream, "Total leaked bytes: ", leakedBytes);
		}

		writeOstreamLine(logFileStream, "Memory leak report finished");

		FreeLeaksList(localLeaksList);
	}
}

void LeakDetectorClass::freeAllocatedList()
{
	AllocatedPtr* item = mAllocatedList;
	while (item != nullptr)
	{
		AllocatedPtr* next = item->next;
		item->~AllocatedPtr();
		free(item);
		item = next;
	}
}

void LeakDetectorClass::FreeLeaksList(LeakDetectorClass::MemoryLeak* listBegin)
{
	MemoryLeak* item = listBegin;
	while (item != nullptr)
	{
		MemoryLeak* next = item->next;
		item->~MemoryLeak();
		free(item);
		item = next;
	}
}

void* operator new(size_t size, const char* file, int line) noexcept
{
	void* p = malloc(size);
	LeakDetectorClass::Get().addAllocated(p, size, file, line);
	return p;
}

void* operator new[](size_t size, const char* file, int line) noexcept
{
	void* p = malloc(size);
	LeakDetectorClass::Get().addAllocated(p, size, file, line);
	return p;
}

void* operator new(size_t size)
{
	return operator new(size, "<Unknown>", 0);
}

void* operator new[](size_t size)
{
	return operator new[](size, "<Unknown>", 0);
}

void* operator new(size_t size, const std::nothrow_t& /*tag*/) noexcept
{
	return operator new(size, "<Unknown>", 0);
}

void* operator new[](size_t size, const std::nothrow_t& /*tag*/) noexcept
{
	return operator new[](size, "<Unknown>", 0);
}

void operator delete(void* p) noexcept
{
	LeakDetectorClass::Get().removeAllocated(p);
	free(p);
}

void operator delete(void* p, size_t) noexcept
{
	LeakDetectorClass::Get().removeAllocated(p);
	free(p);
}

void operator delete[](void* p) noexcept
{
	LeakDetectorClass::Get().removeAllocated(p);
	free(p);
}

void operator delete[](void* p, size_t) noexcept
{
	LeakDetectorClass::Get().removeAllocated(p);
	free(p);
}

#endif // DETECT_MEMORY_LEAKS
