#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include "tigl.h"
#include <glm/gtc/matrix_transform.hpp>
#include "ObjectManager.h"
#include "GameObject.h"
#include "Components.h"
#include <iostream>
#include <algorithm>
#include "WaveFunctionCollapse.h";
using tigl::Vertex;

#pragma comment(lib, "glfw3.lib")
#pragma comment(lib, "glew32s.lib")
#pragma comment(lib, "opengl32.lib")

GLFWwindow* window;
std::shared_ptr < std::list<std::shared_ptr<GameObject>>> objects;
glm::vec3 camPosition = glm::vec3(1.0f);
glm::vec3 camLookat = glm::vec3(0.0f);
float rotation = 0;

void init();
void update();
void draw();

int main(void)
{
	if (!glfwInit())
		throw "Could not initialize glwf";
	window = glfwCreateWindow(1400, 800, "Wave Function Collapse", NULL, NULL);
	if (!window)
	{
		glfwTerminate();
		throw "Could not initialize glwf";
	}
	glfwMakeContextCurrent(window);

	tigl::init();

	init();

	while (!glfwWindowShouldClose(window))
	{
		update();
		draw();
		glfwSwapBuffers(window);
		glfwPollEvents();
	}

	glfwTerminate();


	return 0;
}


void init()
{
	objects = std::make_shared<std::list<std::shared_ptr<GameObject>>>();
	ObjectManager objectManager = ObjectManager(objects);

	std::shared_ptr<std::vector<std::vector<std::string>>> strings = std::make_shared<std::vector<std::vector<std::string>>>(std::vector<std::vector<std::string>>());

	//This should be on a thread
	std::shared_ptr<WaveFunctionCollapse> wfc = runWaveFunctionCollapse(initialiseWFC(15, 5, "models/wfc_data.dat", strings));
	if (wfc == nullptr)
	{
		std::cout << "Unable to collapse" << std::endl;
		return;
	}
	else
	{
		std::cout << "Collapsed" << std::endl;
		wfc->renderKnowns(objectManager, strings);
	}

	camLookat = glm::vec3(0, wfc->height, 0);
	camPosition = glm::vec3(0, wfc->height, wfc->sideLength * 2);

	glfwSetKeyCallback(window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
		{
			if (key == GLFW_KEY_ESCAPE)
			glfwSetWindowShouldClose(window, true);
		});

	//moving tiles the other way

}

double lastFrameTime = 0;
float rotationSpeed = 2;
float moveSpeed = 16;
bool spaceHeld = false;
void update()
{
	double frameTime = glfwGetTime();
	float deltaTime = frameTime - lastFrameTime;
	lastFrameTime = frameTime;

	for (std::shared_ptr<GameObject> object : *objects)
	{
		object->update(deltaTime);
	}

	if (glfwGetKey(window, GLFW_KEY_D)) {
		glm::mat4 camMatrix = glm::mat4(1.0f);
		camMatrix = glm::rotate(camMatrix, -rotationSpeed * deltaTime, glm::vec3(0, 1, 0));
		camPosition = glm::vec3(camMatrix * glm::vec4(camPosition, 0.0));
	}
	if (glfwGetKey(window, GLFW_KEY_A)) {
		glm::mat4 camMatrix = glm::mat4(1.0f);
		camMatrix = glm::rotate(camMatrix, rotationSpeed * deltaTime, glm::vec3(0, 1, 0));
		camPosition = glm::vec3(camMatrix * glm::vec4(camPosition, 0.0));
	}
	if (glfwGetKey(window, GLFW_KEY_S)) {
		camPosition.y += moveSpeed * deltaTime;
	}
	if (glfwGetKey(window, GLFW_KEY_W)) {
		camPosition.y -= moveSpeed * deltaTime;
	}

	if (glfwGetKey(window, GLFW_KEY_SPACE))
	{
		if (!spaceHeld)
		{
			spaceHeld = true;
			for (auto& object : *objects)
			{
				std::shared_ptr<TrackMoveComponent> component = object->getComponent<TrackMoveComponent>();
				if (component != nullptr)
					component->toggleMove();
			}
		}
	}
	else
	{
		spaceHeld = false;
	}
}

void draw()
{

	float lerp = std::clamp(camPosition.y / 20.0f, 0.0f, 1.0f);
	float invLerp = (1 - lerp);
	glClearColor(0.875f * lerp + 0.125f * invLerp, 0.625f * lerp + 0.125f * invLerp, 0.375f * lerp + 0.25f * invLerp, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	int viewport[4];
	glGetIntegerv(GL_VIEWPORT, viewport);
	glm::mat4 projection = glm::perspective(glm::radians(75.0f), viewport[2] / (float)viewport[3], 0.01f, 500.0f);

	tigl::shader->setProjectionMatrix(projection);
	tigl::shader->setViewMatrix(glm::lookAt(glm::vec3(camPosition.x, camPosition.y, camPosition.z), camLookat, glm::vec3(0, 1, 0)));
	tigl::shader->setModelMatrix(glm::rotate(glm::mat4(1.0f), rotation, glm::vec3(0, 1, 0)));

	tigl::shader->enableColor(true);
	tigl::shader->enableLighting(true);
	tigl::shader->setLightCount(1);
	tigl::shader->setLightDirectional(0, true);
	tigl::shader->setLightPosition(0, glm::vec3(5.0f, 10.0f, 5.0f));

	tigl::shader->enableFog(true);
	tigl::shader->setFogColor(glm::vec3(0.125f, 0.125f, 0.125f));
	tigl::shader->setFogExp(0.05);

	glEnable(GL_DEPTH_TEST);


	glPointSize(10.0f);
	for (auto& object : *objects) {
		object->draw();
	}
}