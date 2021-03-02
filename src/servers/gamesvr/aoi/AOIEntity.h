#pragma once

#include <memory.h>

struct AOILocation
{
    float x;
    float y;
    float z;
};


//AOI范围
struct AOIRange
{
    float length;
    float width;
    float high;
};


/*AOI对象基类*/
class AOIEntity
{
public:
    AOIEntity(const AOILocation& point, const AOIRange& range);
    virtual ~AOIEntity();

    const AOILocation& GetLocation() const { return m_location; }

    void SetLocation(const AOILocation& pos) { memcpy(&m_location, &pos, sizeof(AOILocation)); }

    virtual void AOIEnter(const AOIEntity& target, const AOILocation& pos) = 0;

    virtual void AOILeave(const AOIEntity& target) = 0;

    virtual void AOIMove(const AOIEntity& target, const AOILocation& targetPos) = 0;

private:
    AOILocation m_location;       //当前坐标

    AOIRange m_range;       //AOI范围
};


AOIEntity::AOIEntity(const AOILocation& point, const AOIRange& range)
{
    memcpy(&m_location, &point, sizeof(AOILocation));
    memcpy(&m_range, &range, sizeof(AOIRange));
}


AOIEntity::~AOIEntity()
{
}