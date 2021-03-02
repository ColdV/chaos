#pragma once

#include "stdafx.h"
#include "AOIEntity.h"
#include <math.h>
#include <unordered_set>
#include <set>
#include <functional>


//场景尺寸
struct AOISenceSize
{
    uint32 length;
    uint32 width;
    uint32 high;
};


//3D立方体坐标范围
struct AOISenceRange
{
    float xleft;        
    float xright;

    float ytop;
    float ybottom;

    float ztop;
    float zbottom;
};


//AOI格子
struct AOIGrid
{
    std::unordered_set<AOIEntity*> entities;
    uint32 id;     //格子ID
};


//class PathAlgorithm;
class AOIManager : public NonCopyable
{
public:
	enum AOIAction
	{
		AOI_ENTER = 1,	//进入AOI
		AOI_LEAVE,		//离开AOI
		AOI_MOVE,		//在AOI中移动
	};

    typedef std::vector<std::vector<std::vector<AOIGrid>>>   GridMatrix;
	typedef std::function<void(const AOILocation & srcPos, const AOILocation & desPos, std::vector<AOILocation> & locations)> FindPathAlgorithm;

	AOIManager(const AOISenceRange& senceRange, const AOISenceSize& senceBlock, const FindPathAlgorithm& fpAlgorithm = NULL);
    ~AOIManager();

    //进入AOI
    void Enter(AOIEntity* pEntity, const AOILocation& pos);

    //离开AOI
    void Leave(AOIEntity* pEntity);

    //在AOI中移动
    bool Move(AOIEntity* pEntity, const AOILocation& targetPos);

    //寻找周围的格子
    void FindAroundGrids(const AOILocation& pos, std::set<const AOIGrid*>& grids);

private:
    //通知周围的格子
    void NotifyAroundGrids(const AOIEntity& entity, const AOILocation& targetPos, int action);

	template<typename T>
	void NotifyGrids(const T& grids, const AOIEntity& entity, const AOILocation& targetPos, int action);

    //根据坐标获取当前格子
    AOIGrid& GetGridByPos(const AOILocation& pos);

	//根据格子下标获取格子
	AOIGrid& GetGridByIdx(int xIdx, int yIdx, int zIdx);

	//根据坐标获取当前格子下标
	//@param xIdx:当前格子所在的x下标
	//@param yIdx:当前格子所在的y下标
	//@param zIdx:当前格子所在的x下标
	void GetGridIdxByPos(const AOILocation& pos, int& xIdx, int& yIdx, int& zIdx);

	//计算两个坐标之间的所有格子(直线距离跨越的格子)
	//正确的路径应由寻路算法计算得出
	void CalculateGridsBetweenLocation(const AOILocation& srcPos, const AOILocation& desPos, std::vector<AOILocation>& locations);
    
    bool CheckPos(const AOILocation& pos);

private:
    AOISenceRange m_senceRange;				//AOI管理的坐标范围

    AOISenceSize m_senceSize;				//AOI管理的地图尺寸

    AOISenceSize m_gridSize;				//AOI每一个格子的尺寸

    GridMatrix m_grids;						//所有格子

	FindPathAlgorithm m_fpAlgorithm;		//寻路算法
    //std::unique_ptr<PathAlgorithm> m_pathAlgorithm;           
};