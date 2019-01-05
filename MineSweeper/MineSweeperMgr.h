#pragma once

#include<map>
#include<vector>
#include<set>
#include <time.h>

struct Point
{
	Point()
	{
		x = 0;
		y = 0;
	}

	Point(int nX, int nY) :x(nX), y(nY) 
	{
	}

	bool operator < (const Point& point) const
	{
		if (y < point.y)
			return true;

		if (y == point.y && x < point.x)
			return true;

		return false;
	}

	bool operator == (const Point& point) const
	{
		if (x == point.x && y == point.y)
			return true;

		return false;
	}

	int x;
	int y;
};


struct MapLevelCnf
{
	int Level;
	int MaxMineCount;
	Point MaxPoint;
};

static const MapLevelCnf MapConfig[] =
{
	{1, 10, {10, 10}},
	{2,	40, {20, 20}},
	{3, 90, {30, 30}},
};


inline const MapLevelCnf* GetMapConfig(int nMapLevel)
{
	if (0 >= nMapLevel || sizeof(MapConfig) / sizeof(MapConfig[0]) < nMapLevel)
	{
		printf("don't fount map config by map level:%d\n", nMapLevel);
		return NULL;
	}

	return &MapConfig[nMapLevel - 1];
}


#define	LANDMINE	-1		//雷


//地图格子状态
enum MineSweeperStatus
{
	MSS_DEFAULT		= 0,	//默认没打开状态
	MSS_OPEN		= 1,	//打开状态
	MSS_FLAG		= 2,	//标记状态
};


//格子数据
struct PointInfo
{
	PointInfo()
	{
		Status	= MSS_DEFAULT;
		Content = 0;
	}

	int		Status;		//状态
	int		Content;	//格子内容
};


//格子动作
enum PointAction
{
	PA_OPEN		= 1,	//打开格子
	PA_FLAG,			//标记格子
};

//扫雷管理器
class MineSweeperMgr
{
public:

	//扫雷地图 坐标索引
	typedef	std::map<Point, PointInfo> MineSweepMap;

	MineSweeperMgr();
	virtual ~MineSweeperMgr();

	//创建地图
	int CreateMap(int nMaxX, int nMaxY, int nMaxMine = 0);

	int CreateMap(int nMapLevel);

	//输出地图
	void PrintMap(bool IsShow = false);

	//点击格子
	int OnClickPoint(const Point& point, int nAction);

private:

	//计算出所有格子中的数字
	void CalcNumber();

	//获取地雷周围的格子
	void GetPointAroundMine(const Point& rePoint, std::vector<Point>& vPoint);

	//打开坐标
	int OpenPoint(const Point& point);

private:
	Point					m_max_point;		//最大坐标
	int						m_max_mine;			//地图中最大雷数
	MineSweepMap			m_mine_sweeper_map;	//地图中所有坐标
	std::set<Point>			m_mine_points;		//所有雷的坐标
	int						m_open_point;		//当前已打开的坐标数量
	int						m_start_time;		//开始时间
	int						m_end_time;			//结束时间
};