#include "Base/precomp.h"

#include <gtest/gtest.h>

#include "UnitTests/TestAssertHelper.h"

using ::testing::EmptyTestEventListener;
using ::testing::InitGoogleTest;
using ::testing::Test;
using ::testing::TestCase;
using ::testing::TestEventListeners;
using ::testing::TestInfo;
using ::testing::TestPartResult;
using ::testing::UnitTest;
using ::testing::Environment;

class SGTestingEnvironment : public Environment
{
public:

	void SetUp() override;
	void TearDown() override;
};

class TestInfoLogger : public EmptyTestEventListener
{
	// Called before a test starts
	void OnTestStart(const TestInfo& test_info) override;
	// Called after a failed assertion or a SUCCEED() invocation
	void OnTestPartResult(const TestPartResult& test_part_result) override;
	// Called after a test ends
	void OnTestEnd(const TestInfo& test_info) override;
};

void SGTestingEnvironment::SetUp()
{
	EnableFailOnAssert();
}

void SGTestingEnvironment::TearDown()
{
}

// Called before a test starts.
void TestInfoLogger::OnTestStart(const TestInfo& /*test_info*/)
{
//	LogInfo("======= Test %s.%s starting.", test_info.test_case_name(), test_info.name());
}

// Called after a failed assertion or a SUCCEED() invocation.
void TestInfoLogger::OnTestPartResult(const TestPartResult& test_part_result)
{
	if (test_part_result.failed()) {
		LogError("======= %s in %s:%d\n%s", (test_part_result.failed() ? "Failure" : "Success"),
			test_part_result.file_name(), test_part_result.line_number(), test_part_result.summary());
	}
}

// Called after a test ends.
void TestInfoLogger::OnTestEnd(const TestInfo& /*test_info*/)
{
//	LogInfo("======= Test %s.%s ending.", test_info.test_case_name(), test_info.name());
	EnableFailOnAssert();
}

int main(int argc, char* argv[])
{
	InitGoogleTest(&argc, argv);

	AddGlobalTestEnvironment(HS_NEW SGTestingEnvironment());

	TestEventListeners& listeners = UnitTest::GetInstance()->listeners();
	listeners.Append(HS_NEW TestInfoLogger());

	int retVal = RUN_ALL_TESTS();

	return retVal;
}
