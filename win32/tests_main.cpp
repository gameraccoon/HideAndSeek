#include <gtest/gtest.h>

#include "sdl/SDL.h"
#include "Log.h"
#include "Assert.h"

using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;
using ::testing::Environment;

namespace testing
{
	namespace internal
	{
		enum GTestColor
		{
			COLOR_DEFAULT,
			COLOR_RED,
			COLOR_GREEN,
			COLOR_YELLOW
		};

		extern void ColoredPrintf(GTestColor color, const char* fmt, ...);
	}
}

class SGTestingEnvironment : public Environment
{
public:

	virtual void SetUp() override;
	virtual void TearDown() override;
};

class TestInfoLogger : public EmptyTestEventListener
{
	// Called before a test starts.
	virtual void OnTestStart(const TestInfo& test_info);
	// Called after a failed assertion or a SUCCEED() invocation.
	virtual void OnTestPartResult(const TestPartResult& test_part_result);
	// Called after a test ends.
	virtual void OnTestEnd(const TestInfo& /*test_info*/);
};

void SGTestingEnvironment::SetUp()
{
}

void SGTestingEnvironment::TearDown()
{
}

// Called before a test starts.
void TestInfoLogger::OnTestStart(const TestInfo& test_info)
{
	LogInfo(std::string("======= Test ") + test_info.test_case_name() + "." + test_info.name() + " starting.");
}

// Called after a failed assertion or a SUCCEED() invocation.
void TestInfoLogger::OnTestPartResult(const TestPartResult& test_part_result)
{
	if (test_part_result.failed()) {
		LogError(std::string("=======").append(test_part_result.failed() ? "Failure" : "Success") + "in "
			+ test_part_result.file_name() + ":" + std::to_string(test_part_result.line_number()) + "\n" + test_part_result.summary());
	}
}

// Called after a test ends.
void TestInfoLogger::OnTestEnd(const TestInfo& /*test_info*/)
{
	//LogInfo(std::string("======= Test ") + test_info.test_case_name() + "." + test_info.name() + " ending.");
}

int main(int argc, char* argv[])
{
	InitGoogleTest(&argc, argv);

	AddGlobalTestEnvironment(new SGTestingEnvironment());

	TestEventListeners& listeners = UnitTest::GetInstance()->listeners();
	listeners.Append(new TestInfoLogger());

	int ret_val = RUN_ALL_TESTS();

	return ret_val;
}
