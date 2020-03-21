#include "Base/precomp.h"

#include "GameData/AI/NavMesh.h"

#include <DetourNavMesh.h>

NavMesh::~NavMesh()
{
	if (mMesh)
	{
		delete mMesh;
	}
}

void NavMesh::setMesh(dtNavMesh *newMesh)
{
	if (mMesh)
	{
		delete mMesh;
	}
	mMesh = newMesh;
}

dtNavMesh* NavMesh::getMesh()
{
	return mMesh;
}
