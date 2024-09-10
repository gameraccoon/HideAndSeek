#pragma once

class EditorIdGenerator
{
public:
	size_t getNextId()
	{
		return ++mNextId;
	}

private:
	size_t mNextId = 0;
};
