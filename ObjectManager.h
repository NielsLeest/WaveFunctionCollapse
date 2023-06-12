#pragma once
//#include "Game.h"
#include "Components.h"
#include <memory>

class EnvironmentObject {
public:
	glm::vec3 postion;
	glm::vec3 rotation;
};
class ObjectManager {
private:
	std::shared_ptr<std::list<std::shared_ptr<GameObject>>> objectList;
	std::vector<EnvironmentObject> environmentObjects;
public:
	ObjectManager(std::shared_ptr <std::list<std::shared_ptr<GameObject>>> objects);
	~ObjectManager();
	std::shared_ptr<GameObject> addEnvironmentObject(const std::string& fileName, glm::vec3 position, glm::vec3 rotation);
	std::shared_ptr<GameObject> addTrackPath(const std::string& fileName, const std::vector<glm::vec3>& positions, glm::vec3 rotation, bool isLoop, bool isPlayerControlled);
};
