#pragma once

#ifdef DETECT_MEMORY_LEAKS

#include <mutex>
#include <filesystem>
#include <fstream>
#include <iostream>

class LeakDetectorClass
{
public:
	static LeakDetectorClass& Get()
	{
		static LeakDetectorClass instance;
		return instance;
	}

	~LeakDetectorClass();

	void addAllocated(void* p, size_t size, const char* file, int line) noexcept;
	void removeAllocated(void* p) noexcept;

private:
	// single-linked list node
	struct AllocatedPtr
	{
		AllocatedPtr(void* pointer, size_t size, const char* file, int line, AllocatedPtr* next)
			: pointer(pointer), next(next), size(size), file(file), line(line) {}

		void* pointer;
		AllocatedPtr* next;
		size_t size;
		const char* file;
		int line;
	};

	// linked list node
	struct MemoryLeak
	{
		MemoryLeak(void* pointer, size_t size, const char* file, int line)
			: pointer(pointer), size(size), file(file), line(line) {}

		MemoryLeak* next = nullptr;
		void* pointer;
		size_t size;
		const char* file;
		int line;
	};

	void addLeakInfo(void* pointer, size_t size, const char* file, int line);
	void printLeaks() noexcept;

	void freeAllocatedList();
	static void FreeLeaksList(MemoryLeak* listBegin);

	AllocatedPtr* mAllocatedList = nullptr;

	MemoryLeak* mMemoryLeaksFirst = nullptr;
	MemoryLeak* mEmoryLeaksLast = nullptr;

	std::recursive_mutex mMutex;
};

void* operator new(size_t size, const char* file, int line) noexcept;
void* operator new[](size_t size, const char* file, int line) noexcept;

void* operator new(size_t size);
void* operator new[](size_t size);

void operator delete(void* p) noexcept;
void operator delete(void* p, size_t /*size*/) noexcept;

void* operator new(size_t size, const std::nothrow_t& tag) noexcept;
void* operator new[](size_t size, const std::nothrow_t& tag) noexcept;

void operator delete[](void* p) noexcept;
void operator delete[](void* p, size_t /*size*/) noexcept;

// use it instead of 'new' to ease memory leak detection (HS stands for Hide&Seek)
#define HS_NEW new (__FILE__, __LINE__)

#ifdef REDEFINE_NEW
#define new HS_NEW
#endif

#else

// use it instead of 'new' to ease memory leak detection (HS stands for Hide&Seek)
#define HS_NEW new (std::nothrow)

#endif // DETECT_MEMORY_LEAKS
