#pragma once

#include "AOIManager.h"

class PathAlgorithm
{
public:
	PathAlgorithm(const AOISenceRange& range) {}
    virtual ~PathAlgorithm() {}

    virtual void FindPath(std::vector<AOILocation>& locations, const AOILocation& targetPos) = 0;

private:
	AOISenceRange m_senceRange;

	AOISenceSize m_senceSize;
};