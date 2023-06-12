#pragma once

#include "Component.h"
#include <vector>


class TrackMoveComponent : public Component
{
private:
	bool isCyclic;
	bool goingForwards;
	bool moving;
	int currentIndex;
	float moveSpeed;
	float lerp;
	std::vector<glm::vec3> trackCoordinates;
public:
	void toggleMove();

	void update(float elapsedTime);

	TrackMoveComponent(const std::vector<glm::vec3>& track, bool cyclic, float speed);
	~TrackMoveComponent();
};