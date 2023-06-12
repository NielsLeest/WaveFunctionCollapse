#pragma once

#include <gl/glew.h>
#include <string>
#include <memory>

class Texture
{
	GLuint id;
public:
	int width;
	int height;
	Texture(const std::string& fileName);
	void bind();
};

std::shared_ptr<Texture> getTexture(const std::string& fileName);