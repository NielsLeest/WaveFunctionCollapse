#pragma once
#include "DrawComponent.h"
#include "tigl.h"

#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <vector>
#include <list>

class Texture;

using tigl::Vertex;

#include <vector>
class ModelComponent : public DrawComponent
{
	std::vector<Vertex> verts;
private:
	class Vertex
	{
	public:
		int position;
		int normal;
		int texcoord;
	};

	class Face
	{
	public:
		std::list<Vertex> vertices;
	};
	class MaterialInfo
	{
	public:
		MaterialInfo();
		std::string name;
		Texture* texture;
		glm::vec4 diffuseColor = glm::vec4(0.5f);
		glm::vec4 ambientColor = glm::vec4(0.0f);
		glm::vec4 specularColor = glm::vec4(0.5f);
	};

	class ObjGroup
	{
	public:
		std::string name;
		int materialIndex;
		std::list<Face> faces;
	};


	std::vector<glm::vec3>	vertices;
	std::vector<glm::vec3>	normals;
	std::vector<glm::vec2>	texcoords;
	std::vector<ObjGroup*> groups;
	std::vector<MaterialInfo*> materials;

	void loadMaterialFile(const std::string& fileName, const std::string& dirName);
	void initModel();
	void init(const std::string& fileName, const float scale);
public:
	ModelComponent(const std::string& fileName);
	ModelComponent(const std::string& fileName, float scale);
	ModelComponent(const std::string& fileName, const float scale, glm::vec4 color);
	~ModelComponent();

	virtual void draw() override;
};

std::shared_ptr<ModelComponent> getModel(const std::string& fileName);