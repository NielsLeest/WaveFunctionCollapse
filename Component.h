#pragma once
#include "GameObject.h"
//class GameObject;

class Component
{
protected:
	GameObject* object;
public:
	Component();
	~Component();

	virtual void update(float elapsedTime) {};

	inline void setObject(GameObject* object) { this->object = object; }
};

