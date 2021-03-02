#include "AOIManager.h"
#include <algorithm>
//#include "PathAlgorithm.h"



AOIManager::AOIManager(const AOISenceRange& senceRange, const AOISenceSize& senceBlock, const FindPathAlgorithm& fpAlgorithm/* = NULL*/):
	m_fpAlgorithm(fpAlgorithm)
{
	memcpy(&m_senceRange, &senceRange, sizeof(AOISenceRange));
	memcpy(&m_gridSize, &senceBlock, sizeof(AOISenceSize));

	m_senceSize.length = (uint32)ceil(m_senceRange.xright - m_senceRange.xleft);       //长
	m_senceSize.width = (uint32)ceil(m_senceRange.ytop - m_senceRange.ybottom);        //宽
	m_senceSize.high = (uint32)ceil(m_senceRange.ztop - m_senceRange.zbottom);         //高

	int xMaxPiece = m_senceSize.length / m_gridSize.length +
		(m_senceSize.length % m_gridSize.length > 0 ? 1 : 0);     //x轴上最大分片
	xMaxPiece = xMaxPiece <= 0 ? 1 : xMaxPiece;

	int yMaxPiece = m_senceSize.width / m_gridSize.width +
		(m_senceSize.width % m_gridSize.width > 0 ? 1 : 0);       //y轴上最大分片
	yMaxPiece = yMaxPiece <= 0 ? 1 : yMaxPiece;

	int zMaxPiece = m_senceSize.high / m_gridSize.high +
		(m_senceSize.high % m_gridSize.high > 0 ? 1 : 0);         //z轴上最大分片
	zMaxPiece = zMaxPiece <= 0 ? 1 : zMaxPiece;

	//预分配地图中的所有格子
	std::vector<AOIGrid> zGrids;
	zGrids.assign(zMaxPiece, {});

	std::vector<std::vector<AOIGrid>> yGrids;
	yGrids.assign(yMaxPiece, zGrids);

	m_grids.assign(xMaxPiece, yGrids);

	if (!m_fpAlgorithm)
		m_fpAlgorithm = std::bind(&AOIManager::CalculateGridsBetweenLocation, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3);
}


AOIManager::~AOIManager()
{
}


void AOIManager::Enter(AOIEntity* pEntity, const AOILocation& pos)
{
	if (!pEntity)
		return;

	if (!CheckPos(pos))
		return;

	pEntity->SetLocation(pos);

	AOIGrid& grid = GetGridByPos(pos);
	grid.entities.insert(pEntity);

	NotifyAroundGrids(*pEntity, pos, AOI_ENTER);

}


void AOIManager::Leave(AOIEntity* pEntity)
{
	if (!pEntity)
		return;

	if (!CheckPos(pEntity->GetLocation()))
		return;

	AOIGrid& grid = GetGridByPos(pEntity->GetLocation());
	grid.entities.erase(pEntity);

	NotifyAroundGrids(*pEntity, pEntity->GetLocation(), AOI_LEAVE);
}


bool AOIManager::Move(AOIEntity* pEntity, const AOILocation& targetPos)
{
	if (!pEntity || !CheckPos(targetPos))
		return false;

	std::vector<AOILocation> locations;
	//m_pathAlgorithm->FindPath(locations, targetPos);			//这里不写成抽象算法,不然返回的坐标有可能跨越N个格子
	//CalculateGridsBetweenLocation(pEntity->GetLocation(), targetPos, locations);
	m_fpAlgorithm(pEntity->GetLocation(), targetPos, locations);

	for (auto pos : locations)
	{
		if (!CheckPos(pos))
			return false;
			
		//这里计算交集的话要考虑两个坐标之间跨格子问题,暂时假设每次跨格子都是相邻的格子
		//之后用考虑是否用算法保证每次移动最多只跨相邻格子 或者这里用其他算法计算两点之间
		//的直线距离跨了多少个格子...

		AOIGrid& oldGrid = GetGridByPos(pEntity->GetLocation());
		AOIGrid& newGrid = GetGridByPos(pos);

		if (&oldGrid == &newGrid)
			NotifyAroundGrids(*pEntity, pos, AOI_MOVE);
		
		else
		{
			std::set<const AOIGrid*> oldSet;
			std::set<const AOIGrid*> newSet;
			FindAroundGrids(pEntity->GetLocation(), oldSet);
			FindAroundGrids(pos, newSet);

			//需要通知离开AOI的格子
			std::vector<const AOIGrid*> leaveSet(oldSet.size());
			auto lastIt1 = std::set_difference(oldSet.begin(), oldSet.end(), newSet.begin(), newSet.end(), leaveSet.begin());
			leaveSet.resize(lastIt1 - leaveSet.begin());

			//需要通知进入AOI的格子
			std::vector<const AOIGrid*> enterSet(newSet.size());
			auto lastIt2 = std::set_difference(newSet.begin(), newSet.end(), oldSet.begin(), oldSet.end(), enterSet.begin());
			enterSet.resize(lastIt2 - enterSet.begin());

			//需要通知移动AOI的格子
			std::vector<const AOIGrid*> moveSet(oldSet.size() + newSet.size());
			auto lastIt3 = std::set_union(oldSet.begin(), oldSet.end(), newSet.begin(), newSet.end(), moveSet.begin());
			moveSet.resize(lastIt3 - moveSet.begin());

			oldGrid.entities.erase(pEntity);
			newGrid.entities.insert(pEntity);

			NotifyGrids(leaveSet, *pEntity, pEntity->GetLocation(), AOI_LEAVE);
			NotifyGrids(enterSet, *pEntity, pos, AOI_ENTER);
			NotifyGrids(moveSet, *pEntity, pos, AOI_MOVE);
		}

		pEntity->SetLocation(pos);
	}

	return true;
}


//寻找周围的格子
void AOIManager::FindAroundGrids(const AOILocation& pos, std::set<const AOIGrid*>& grids)
{
	grids.clear();

	if (!CheckPos(pos))
		return;

	int xIdx, yIdx, zIdx;
	GetGridIdxByPos(pos, xIdx, yIdx, zIdx);

	//立方体-这里应该获取周围包括自己共27个格子
	for (int x = xIdx - 1; x < xIdx + 1; ++x)
	{
		if (x < 0 || x >= m_grids.size())
			continue;

		for (int y = yIdx - 1; y < yIdx + 1; ++y)
		{
			if (y < 0 || y >= m_grids[x].size())
				continue;

			for (int z = zIdx - 1; z < zIdx + 1; ++z)
			{
				if (z < 0 || z > m_grids[x][y].size())
					continue;

				grids.insert(&m_grids[x][y][z]);
			}
		}
	}

}


void AOIManager::NotifyAroundGrids(const AOIEntity& entity, const AOILocation& targetPos, int action)
{
	std::set<const AOIGrid*> grids;
	FindAroundGrids(entity.GetLocation(), grids);

	NotifyGrids(grids, entity, targetPos, action);
}


template<typename T>
void AOIManager::NotifyGrids(const T& grids, const AOIEntity& target, const AOILocation& targetPos, int action)
{
	for (auto grid : grids)
	{
		for (auto pEntity : grid->entities)
		{
			assert(pEntity);

			switch (action)
			{
			case AOI_ENTER:
				pEntity->AOIEnter(target, targetPos);
				break;

			case AOI_LEAVE:
				pEntity->AOILeave(target);
				break;

			case AOI_MOVE:
				pEntity->AOIMove(target, targetPos);
				break;

			default:
				break;
			}
		}
	}
}


AOIGrid& AOIManager::GetGridByPos(const AOILocation& pos)
{
	assert(CheckPos(pos));

	int xIdx, yIdx, zIdx;
	GetGridIdxByPos(pos, xIdx, yIdx, zIdx);
	
	assert(xIdx >= 0 && yIdx >= 0 && zIdx >= 0);

	return m_grids[xIdx][yIdx][zIdx];
}


AOIGrid& AOIManager::GetGridByIdx(int xIdx, int yIdx, int zIdx)
{
	assert((xIdx >= 0 && xIdx < m_grids.size()) &&
		(yIdx > 0 && yIdx < m_grids[xIdx].size()) &&
		(zIdx > 0 && zIdx < m_grids[xIdx][yIdx].size()));

	return m_grids[xIdx][yIdx][zIdx];
}


void AOIManager::GetGridIdxByPos(const AOILocation& pos, int& xIdx, int& yIdx, int& zIdx)
{
	assert(CheckPos(pos));

	float length = pos.x - m_senceRange.xleft;
	xIdx = (uint32)ceil(length / m_gridSize.length);

	float width = pos.y - m_senceRange.ybottom;
	yIdx = (uint32)ceil(width / m_gridSize.width);

	float high = pos.z - m_senceRange.zbottom;
	zIdx = (uint32)ceil(high / m_gridSize.high);
}


//计算两个坐标之间的所有格子(直线距离跨越的格子)
void AOIManager::CalculateGridsBetweenLocation(const AOILocation& srcPos, const AOILocation& desPos, std::vector<AOILocation>& locations)
{
	assert(CheckPos(srcPos) && CheckPos(desPos));

	//根据两个坐标求出空间直线方程

	//求出直线在x,y,z上跨越多少个边(比如边为:x=1)

	//根据交点求直线方块边的交点

	//根据所有交点两两相判断求出所有经过的格子
}


bool AOIManager::CheckPos(const AOILocation& pos)
{
	if (pos.x < m_senceRange.xleft || pos.x > m_senceRange.xright
		|| pos.y < m_senceRange.ybottom || pos.y > m_senceRange.ytop
		|| pos.z < m_senceRange.zbottom || pos.z > m_senceRange.ztop)
		return false;

	return true;
}