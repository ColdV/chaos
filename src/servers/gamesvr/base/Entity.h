#pragma once



/*所有游戏实体的基类*/
class Entity
{
public:
	Entity() {}
	virtual ~Entity() = 0;
};

Entity::~Entity()
{}