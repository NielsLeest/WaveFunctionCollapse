#include "ModelComponent.h"
#include <fstream>
#include <iostream>
#include <vector>
#include <algorithm>

#include "tigl.h"
#include "texture.h"

#include <unordered_map>

static std::unordered_map<std::string, std::shared_ptr<ModelComponent>> models;

using tigl::Vertex;

#include "ExtraString.h"
ModelComponent::ModelComponent(const std::string& fileName) {
	init(fileName, 1);
}
ModelComponent::ModelComponent(const std::string& fileName, const float scale)
{
	init(fileName, scale);
}
ModelComponent::ModelComponent(const std::string& fileName, const float scale, glm::vec4 color)
{
	init(fileName, scale);
	materials[0]->ambientColor = color;
}


ModelComponent::~ModelComponent()
{
}

void ModelComponent::init(const std::string& fileName, const float scale) {
	std::cout << "Loading " << fileName << std::endl;
	std::string dirName = fileName;
	if (dirName.rfind("/") != std::string::npos)
		dirName = dirName.substr(0, dirName.rfind("/"));
	if (dirName.rfind("\\") != std::string::npos)
		dirName = dirName.substr(0, dirName.rfind("\\"));
	if (fileName == dirName)
		dirName = "";


	std::ifstream pFile(fileName.c_str());

	if (!pFile.is_open())
	{
		std::cout << "Could not open file " << fileName << std::endl;
		return;
	}


	ObjGroup* currentGroup = new ObjGroup();
	currentGroup->materialIndex = -1;


	while (!pFile.eof())
	{
		std::string line;
		std::getline(pFile, line);
		line = cleanLine(line);
		if (line == "" || line[0] == '#') //skip empty or commented line
			continue;

		std::vector<std::string> params = split(line, " ");
		params[0] = toLower(params[0]);

		if (params[0] == "v")
			vertices.push_back(glm::vec3((float)atof(params[1].c_str()) * scale, (float)atof(params[2].c_str()) * scale, (float)atof(params[3].c_str()) * scale));
		else if (params[0] == "vn")
			normals.push_back(glm::vec3((float)atof(params[1].c_str()), (float)atof(params[2].c_str()), (float)atof(params[3].c_str())));
		else if (params[0] == "vt")
			texcoords.push_back(glm::vec2((float)atof(params[1].c_str()), (float)atof(params[2].c_str())));
		else if (params[0] == "f")
		{
			for (size_t ii = 4; ii <= params.size(); ii++)
			{
				Face face;

				for (size_t i = ii - 3; i < ii; i++)	//magische forlus om van quads triangles te maken ;)
				{
					Vertex vertex;
					std::vector<std::string> indices = split(params[i == (ii - 3) ? 1 : i], "/");
					if (indices.size() >= 1)	//er is een positie
						vertex.position = atoi(indices[0].c_str()) - 1;
					if (indices.size() == 2)		//alleen texture
						vertex.texcoord = atoi(indices[1].c_str()) - 1;
					if (indices.size() == 3)		//v/t/n of v//n
					{
						if (indices[1] != "")
							vertex.texcoord = atoi(indices[1].c_str()) - 1;
						vertex.normal = atoi(indices[2].c_str()) - 1;
					}
					face.vertices.push_back(vertex);
				}
				currentGroup->faces.push_back(face);
			}
		}
		else if (params[0] == "s")
		{//smoothing groups
		}
		else if (params[0] == "mtllib")
		{
			loadMaterialFile(dirName + "/" + params[1], dirName);
		}
		else if (params[0] == "usemtl")
		{
			if (currentGroup->faces.size() != 0)
				groups.push_back(currentGroup);
			currentGroup = new ObjGroup();
			currentGroup->materialIndex = -1;

			for (size_t i = 0; i < materials.size(); i++)
			{
				MaterialInfo* info = materials[i];
				if (info->name == params[1])
				{
					currentGroup->materialIndex = i;
					break;
				}
			}
			if (currentGroup->materialIndex == -1)
				std::cout << "Could not find material name " << params[1] << std::endl;
		}
	}
	groups.push_back(currentGroup);
	initModel();
}



void ModelComponent::initModel()
{
	for (auto group : groups)
	{
		tigl::shader->enableColor(true);
		if (group->materialIndex >= 0) {
			if (materials[group->materialIndex]->texture != NULL) {
				tigl::shader->enableTexture(true);
				materials[group->materialIndex]->texture->bind();
			}
		}
		bool hascolor = true;
		glm::vec4 color = materials[0]->diffuseColor;
		//tigl::shader->setColorMult(color);
		for (const auto& face : group->faces) {
			for (const auto& vertex : face.vertices) {
				if (vertex.position < 0) throw "no position found";
				bool hasnormal = vertex.normal >= 0;
				bool hastexcoord = vertex.texcoord >= 0;
				char vertexCode = (hascolor << 2) | (hasnormal << 1) | hastexcoord;

				switch (vertexCode) {
				case 0:
					verts.push_back(tigl::Vertex::P(vertices[vertex.position]));
					break;
				case 1:
					verts.push_back(tigl::Vertex::PT(vertices[vertex.position], texcoords[vertex.texcoord]));
					break;
				case 2:
					verts.push_back(tigl::Vertex::PN(vertices[vertex.position], normals[vertex.normal]));
					break;
				case 3:
					verts.push_back(tigl::Vertex::PTN(vertices[vertex.position], texcoords[vertex.texcoord], normals[vertex.normal]));
					break;
				case 4:
					verts.push_back(tigl::Vertex::PC(vertices[vertex.position], color));
					break;
				case 5:
					verts.push_back(tigl::Vertex::PTC(vertices[vertex.position], texcoords[vertex.texcoord], color));
					break;
				case 6:
					verts.push_back(tigl::Vertex::PCN(vertices[vertex.position], color, normals[vertex.normal]));
					break;
				case 7:
					verts.push_back(tigl::Vertex::PCTN(vertices[vertex.position], color, texcoords[vertex.texcoord], normals[vertex.normal]));
					break;
				}

			}
		}
		tigl::shader->enableColorMult(false);
	}

	//foreach group in groups
	//  set material texture, if available
	//  set material color, if available
	//  foreach face in group
	//    foreach vertex in face
	//      emit vertex
}
void ModelComponent::loadMaterialFile(const std::string& fileName, const std::string& dirName)
{
	std::cout << "Loading " << fileName << std::endl;
	std::ifstream pFile(fileName.c_str());
	if (!pFile.is_open())
	{
		std::cout << "Could not open file " << fileName << std::endl;
		return;
	}

	MaterialInfo* currentMaterial = NULL;

	while (!pFile.eof())
	{
		std::string line;
		std::getline(pFile, line);
		line = cleanLine(line);
		if (line == "" || line[0] == '#')
			continue;

		std::vector<std::string> params = split(line, " ");
		params[0] = toLower(params[0]);

		if (params[0] == "newmtl")
		{
			if (currentMaterial != NULL)
			{
				materials.push_back(currentMaterial);
			}
			currentMaterial = new MaterialInfo();
			currentMaterial->name = params[1];
		}
		else if (params[0] == "map_kd")
		{
			std::string tex = params[1];
			if (tex.find("/"))
				tex = tex.substr(tex.rfind("/") + 1);
			if (tex.find("\\"))
				tex = tex.substr(tex.rfind("\\") + 1);
			//TODO
			currentMaterial->texture = getTexture(dirName + "/" + tex).get();
		}
		else if (params[0] == "kd")
		{//TODO, diffuse color
			currentMaterial->diffuseColor = glm::vec4(std::stof(params[1]), std::stof(params[2]), std::stof(params[3]), 1.0f);
		}
		else if (params[0] == "ka")
		{//TODO, ambient color
			currentMaterial->ambientColor = glm::vec4(std::stof(params[1]), std::stof(params[2]), std::stof(params[3]), 1.0f);
		}
		else if (params[0] == "ks")
		{//TODO, specular color
			currentMaterial->specularColor = glm::vec4(std::stof(params[1]), std::stof(params[2]), std::stof(params[3]), 1.0f);
		}
		else if (
			params[0] == "illum" ||
			params[0] == "map_bump" ||
			params[0] == "map_ke" ||
			params[0] == "map_ka" ||
			params[0] == "map_d" ||
			params[0] == "d" ||
			params[0] == "ke" ||
			params[0] == "ns" ||
			params[0] == "ni" ||
			params[0] == "td" ||
			params[0] == "tf" ||
			params[0] == "tr" ||
			false)
		{
			//these values are usually not used for rendering at this time, so ignore them
		}
		else
			std::cout << "Didn't parse " << params[0] << " in material file" << std::endl;
	}
	if (currentMaterial != NULL)
		materials.push_back(currentMaterial);

}

void ModelComponent::draw()
{
	for (auto group : groups) {
		if (group->materialIndex >= 0) {
			tigl::shader->setLightAmbient(0, glm::vec3(materials[group->materialIndex]->ambientColor.r, materials[group->materialIndex]->ambientColor.g, materials[group->materialIndex]->ambientColor.b));
			tigl::shader->setLightDiffuse(0, glm::vec3(materials[group->materialIndex]->diffuseColor.r, materials[group->materialIndex]->diffuseColor.g, materials[group->materialIndex]->diffuseColor.b));
			tigl::shader->setLightSpecular(0, glm::vec3(materials[group->materialIndex]->specularColor.r, materials[group->materialIndex]->specularColor.g, materials[group->materialIndex]->specularColor.b));
			tigl::shader->setShinyness(5.0f);
			if (materials[group->materialIndex]->texture != NULL) {
				tigl::shader->enableTexture(true);
				materials[group->materialIndex]->texture->bind();
			}
			else {
				tigl::shader->enableTexture(false);
			}
		}
	}
	tigl::drawVertices(GL_TRIANGLES, verts);
	tigl::shader->enableTexture(false);
}

ModelComponent::MaterialInfo::MaterialInfo()
{
	texture = NULL;
}

std::shared_ptr<ModelComponent> getModel(const std::string& fileName)
{
	std::unordered_map<std::string, std::shared_ptr<ModelComponent>>::const_iterator lookup = models.find(fileName);
	if (lookup != models.end())
		return lookup->second;

	std::shared_ptr<ModelComponent> model = std::make_shared<ModelComponent>(ModelComponent(fileName));
	models.insert({ { fileName, model } });

	return model;
}
