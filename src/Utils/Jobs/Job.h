#pragma once

#include <memory>

namespace Jobs
{
	class BaseJob
	{
	public:
		using UniquePtr = std::unique_ptr<BaseJob>;
		using JobGroupId = unsigned long;

	public:
		virtual ~BaseJob() = default;

		// called in worker thread
		virtual void process() = 0;
		// called in the requester thread when the work is done
		virtual void finalize() = 0;

		void setJobGroupId(JobGroupId jobGroupId);
		[[nodiscard]] JobGroupId getJobGroupId() const;

	private:
		JobGroupId mJobGroupId{};
	};
}
