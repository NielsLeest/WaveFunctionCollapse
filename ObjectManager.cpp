#include "ObjectManager.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <string>
#include "TrackMoveComponent.h"

ObjectManager::ObjectManager(std::shared_ptr <std::list<std::shared_ptr<GameObject>>> objects)
{
	this->objectList = objects;
	//initEnvironment(fileName);
}
ObjectManager::~ObjectManager() {

}

#include "ExtraString.h"

glm::vec3 stringVectorToVec3(std::vector<std::string> strings) {
	return glm::vec3(atoi(strings[0].c_str()), atoi(strings[1].c_str()), atoi(strings[2].c_str()));
}

std::shared_ptr<GameObject> ObjectManager::addEnvironmentObject(const std::string& fileName, glm::vec3 position, glm::vec3 rotation) {
	std::shared_ptr<GameObject> object = std::make_shared<GameObject>();
	object->addComponent(getModel(fileName));
	object->position = position;
	object->rotation = rotation;
	objectList->push_back(object);
	return object;
}

std::shared_ptr<GameObject> ObjectManager::addTrackPath(const std::string& fileName, const std::vector<glm::vec3>& positions, glm::vec3 rotation, bool isLoop, bool isPlayerControlled)
{
	std::shared_ptr<GameObject> object = std::make_shared<GameObject>();
	object->addComponent(getModel(fileName));
	object->addComponent(std::make_shared<TrackMoveComponent>(TrackMoveComponent(positions, isLoop, 1.0f)));

	object->position = positions[0];
	object->rotation = rotation;
	objectList->push_back(object);
	return object;
}
