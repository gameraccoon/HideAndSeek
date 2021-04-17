#include "Base/precomp.h"

#include "Utils/Jobs/Job.h"

namespace Jobs
{
	void BaseJob::setJobGroupId(JobGroupId jobGroupId)
	{
		mJobGroupId = jobGroupId;
	}

	BaseJob::JobGroupId Jobs::BaseJob::getJobGroupId() const
	{
		return mJobGroupId;
	}
}
