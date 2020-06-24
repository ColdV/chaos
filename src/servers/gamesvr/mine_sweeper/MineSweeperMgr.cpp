#include "MineSweeperMgr.h"
#include <stdlib.h>

MineSweeperMgr::MineSweeperMgr()
{
	m_max_point.x = 0;
	m_max_point.y = 0;
	m_max_mine = 0;
	m_open_point = 0;
	m_start_time = 0;
	m_end_time = 0;

	m_mine_sweeper_map.clear();
	m_mine_points.clear();
}

MineSweeperMgr::~MineSweeperMgr()
{

}




/*
将二维数组的坐标转化为一维数组 再从数组中随机出N个雷
*/
int MineSweeperMgr::CreateMap(int nMaxX, int nMaxY, int nMaxMine /*= 0*/)
{
	m_max_point.x = nMaxX - 1;
	m_max_point.y = nMaxY - 1;

	if (0 == nMaxMine)
	{
		nMaxMine = nMaxX * nMaxY / 10;
	}

	Point MinePoint;
	static Point* MinePointAry = new Point[nMaxX * nMaxY];
	//static Point MinePointAry[100];
	int nCurIdx = 0;

	PointInfo pointInfo;
	//将所有坐标点放入数组中
	for (int x = 0; x < nMaxX; ++x)
	{
		for (int y = 0; y < nMaxY; ++y)
		{
			/*
			MinePointAry[x * nMaxY + nMaxY].x = x;
			MinePointAry[x * nMaxY + nMaxY].y = y;
			*/

			MinePointAry[nCurIdx].x = x;
			MinePointAry[nCurIdx].y = y;

			if (MinePointAry[nCurIdx].x < 0)
			{
				printf("error!\n");
			}

			m_mine_sweeper_map.insert(std::make_pair(MinePointAry[nCurIdx], pointInfo));

			++nCurIdx;
		}
	}


	
	//随机出所有雷的位置
	for (int i = 0; i < nMaxMine; ++i)
	{

		int nMaxIdx = nMaxX * nMaxY - i;
		int nMineIdx = rand() % (nMaxIdx);

		//存储地雷坐标
		//m_mine_sweeper_map.insert(std::make_pair(MinePointAry[nMineIdx], LANDMINE));
		m_mine_sweeper_map[MinePointAry[nMineIdx]].Content = LANDMINE;
		m_mine_points.insert(MinePointAry[nMineIdx]);
		

		//去掉已选中过的坐标
		memcpy(&MinePointAry[nMineIdx], &MinePointAry[nMaxIdx], sizeof(MinePointAry[0]));
		//MinePointAry[nMineIdx] = MinePointAry[nMaxIdx];
	}
	

	//计算出所有格子中的数字
	CalcNumber();

	m_max_mine = nMaxMine;
	m_start_time = time(NULL);

	//输出地图
	//PrintMap(true);
	PrintMap();

	return 0;
}


int MineSweeperMgr::CreateMap(int nMapLevel)
{
	const MapLevelCnf* pMapCnf = GetMapConfig(nMapLevel);

	if (!pMapCnf)
		return -1;

	return CreateMap(pMapCnf->MaxPoint.x,
		pMapCnf->MaxPoint.y,
		pMapCnf->MaxMineCount);
}

//计算出所有格子中的数字
void MineSweeperMgr::CalcNumber()
{
	for (std::set<Point>::iterator it = m_mine_points.begin(); it != m_mine_points.end(); ++it)
	{
		//获取地雷周围的格子
		std::vector<Point> vPoint;
		GetPointAroundMine(*it, vPoint);

		for (auto iter = vPoint.begin(); iter != vPoint.end(); ++iter)
		{
			std::map<Point, PointInfo>::iterator itPoint = m_mine_sweeper_map.find(*iter);
			if (itPoint != m_mine_sweeper_map.end() && itPoint->second.Content != LANDMINE)
			{
				++itPoint->second.Content;
			}
		}
	}
}




//输出地图
void MineSweeperMgr::PrintMap(bool IsShow /*=false*/)
{
	//表头
	for (int i = 0; i <= m_max_point.y; ++i)
	{
		printf("\t%d", i);
	}

	//printf("\n");

	//表内容
	for (auto it = m_mine_sweeper_map.begin(); it != m_mine_sweeper_map.end(); ++it)
	{
		if (0 == it->first.x % (m_max_point.x + 1))
		{
			printf("\n%d", it->first.y);
		}

		if (IsShow)
		{
			printf("\t(%d)", it->second.Content);
		}

		else
		{
			switch (it->second.Status)
			{
			default:
			case MSS_DEFAULT:
				printf("\t(*)");
				break;

			case MSS_OPEN:
				printf("\t(%d)", it->second.Content);
				break;

			case MSS_FLAG:
				printf("\t(√)");
				break;

			}
		}
		//printf("\t(%d)", it->second.Content);
	}

	printf("\n\n");
}


//点击格子
/*
@nAction:执行动作 1.打开格子, 2.标记格子
*/
int MineSweeperMgr::OnClickPoint(const Point& point, int nAction)
{
	MineSweepMap::iterator it = m_mine_sweeper_map.find(point);

	if (it == m_mine_sweeper_map.end())
	{
		printf("invalid point:%d,%d\n", point.x, point.y);
		return 0;
	}

	//非默认状态不可进行操作
	if (MSS_DEFAULT != it->second.Status)
	{
		printf("current status don't action:%d! cur status:%d\n", nAction, it->second.Status);
		return 0;
	}

	//标记格子
	if (PA_FLAG == nAction)
	{
		it->second.Status = PA_FLAG;
		return 0;
	}

	//点中雷游戏结束
	if (LANDMINE == it->second.Content)
	{
		printf("game over!\n");
		m_end_time = time(NULL);
		return -2;
	}

	//打开格子
	OpenPoint(point);

	//如果当前非雷坐标都已打开,游戏胜利
	if (m_open_point >= (m_max_point.x + 1) * (m_max_point.y + 1) - m_max_mine)
	{
		printf("game win!\n");
		m_end_time = time(NULL);
		return 1;
	}

	PrintMap(false);
	printf("cur open point:%d\n", m_open_point);

	return 0;
}

//打开坐标
int MineSweeperMgr::OpenPoint(const Point& point)
{
	MineSweepMap::iterator it = m_mine_sweeper_map.find(point);

	if (it == m_mine_sweeper_map.end())
	{
		printf("invalid point:%d,%d\n", point.x, point.y);
		return -1;
	}

	//踩雷
	if (LANDMINE == it->second.Content)
		return 0;

	//非默认状态不可进行操作
	if (MSS_DEFAULT != it->second.Status)
		return 0;

	++m_open_point;
	it->second.Status = MSS_OPEN;

	//如果是0,继续搜寻周围的格子,直到周围不再有为0的格子
	if (0 == it->second.Content)
	{
		//++m_open_point;

		//获取地雷周围的格子
		std::vector<Point> vPoint;
		GetPointAroundMine(point, vPoint);

		for (auto iter = vPoint.begin(); iter != vPoint.end(); ++iter)
		{
			OpenPoint(*iter);
		}
	}

	return 0;
}


//获取地雷周围的格子
void MineSweeperMgr::GetPointAroundMine(const Point& rePoint, std::vector<Point>& vPoint)
{
	Point desPoint;

	//正上方的格子
	if (rePoint.y + 1 <= m_max_point.y)
	{
		desPoint.x = rePoint.x;
		desPoint.y = rePoint.y + 1;
		vPoint.push_back(desPoint);
	}

	//右上角的格子
	if (rePoint.x + 1 <= m_max_point.x && rePoint.y + 1 <= m_max_point.y)
	{
		desPoint.x = rePoint.x + 1;
		desPoint.y = rePoint.y + 1;
		vPoint.push_back(desPoint);
	}

	//右边的格子
	if (rePoint.x + 1 <= m_max_point.x)
	{
		desPoint.x = rePoint.x + 1;
		desPoint.y = rePoint.y;
		vPoint.push_back(desPoint);
	}

	//右下角的格子
	if (rePoint.x + 1 <= m_max_point.x && rePoint.y - 1 >= 0)
	{
		desPoint.x = rePoint.x + 1;
		desPoint.y = rePoint.y - 1;
		vPoint.push_back(desPoint);
	}

	//下方的格子
	if (rePoint.y - 1 >= 0)
	{
		desPoint.x = rePoint.x;
		desPoint.y = rePoint.y - 1;
		vPoint.push_back(desPoint);
	}

	//左下角的格子
	if (rePoint.x - 1 >= 0 && rePoint.y - 1 >= 0)
	{
		desPoint.x = rePoint.x - 1;
		desPoint.y = rePoint.y - 1;
		vPoint.push_back(desPoint);
	}

	//左边的格子
	if (rePoint.x - 1 >= 0)
	{
		desPoint.x = rePoint.x - 1;
		desPoint.y = rePoint.y;
		vPoint.push_back(desPoint);
	}

	//左上角的格子
	if (rePoint.x - 1 >= 0 && rePoint.y + 1 <= m_max_point.y)
	{
		desPoint.x = rePoint.x - 1;
		desPoint.y = rePoint.y + 1;
		vPoint.push_back(desPoint);
	}
}

