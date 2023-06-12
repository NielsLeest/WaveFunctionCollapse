#include "TrackMoveComponent.h"
#include <algorithm>
#include <iostream>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

TrackMoveComponent::TrackMoveComponent(const std::vector<glm::vec3>& track, bool cyclic, float speed)
{
	trackCoordinates = track;

	isCyclic = cyclic;
	goingForwards = true;
	moving = true;
	currentIndex = 0;
	moveSpeed = speed;
	lerp = 0;
}

TrackMoveComponent::~TrackMoveComponent()
{

}

void TrackMoveComponent::toggleMove()
{
	moving = !moving;
}

void TrackMoveComponent::update(float elapsedTime)
{
	if (!moving)
	{
		return;
	}

	lerp += elapsedTime * moveSpeed;

	int trackLength = trackCoordinates.size();

	//std::cout << lerp << "; " << currentIndex << "/" << trackLength << std::endl;

	while (lerp > 1)
	{
		lerp--;

		if (goingForwards)
		{
			currentIndex++;
		}
		else
		{
			currentIndex--;
		}

		if (isCyclic)
		{
			currentIndex = (currentIndex % trackLength + trackLength) % trackLength;
		}
		else
		{
			if (currentIndex >= trackLength)
			{
				goingForwards = false;
				currentIndex = trackLength - 1;
				lerp = 1 - lerp;
			}
			else if (currentIndex < 0)
			{
				goingForwards = true;
				currentIndex = 0;
				lerp = 1 - lerp;
			}
		}
	}


	//no clue what's going on here anymore
	//rewrite this mess
	float floatPos = 1 - lerp;

	glm::vec3 coord1 = trackCoordinates[currentIndex];
	//just making sure we stay in bounds
	glm::vec3 coord2 = trackCoordinates[((currentIndex + (goingForwards ? -1 : 1)) % trackLength + trackLength) % trackLength];

	object->position = floatPos * coord2 + (1 - floatPos) * coord1;

	//object->position = coord1;
}