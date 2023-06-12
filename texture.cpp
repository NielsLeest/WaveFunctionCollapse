#include "Texture.h"


#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include <iostream>

#include <unordered_map>

static std::unordered_map<std::string, std::shared_ptr<Texture>> textures;

Texture::Texture(const std::string& fileName)
{
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	stbi_set_flip_vertically_on_load(true);
	int width, height, bpp;
	unsigned char* imgData = stbi_load(fileName.c_str(), &width, &height, &bpp, 4);
	this->width = width;
	this->height = height;

	glTexImage2D(GL_TEXTURE_2D,
		0, //level
		GL_RGBA, //internal format
		width, //width
		height, //height
		0, //border
		GL_RGBA, //data format
		GL_UNSIGNED_BYTE, //data type
		imgData);
	stbi_image_free(imgData);//data

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glBindTexture(GL_TEXTURE_2D, 0);

	std::cout << "Made a texture of size " << width << "x" << height << " on id " << id << " from " << fileName << std::endl;
}

void Texture::bind()
{
	//std::cout << "Bound texture " << id << " " << width << "x" << height << std::endl;
	glBindTexture(GL_TEXTURE_2D, id);
}

std::shared_ptr<Texture> getTexture(const std::string& fileName)
{
	std::unordered_map<std::string, std::shared_ptr<Texture>>::const_iterator lookup = textures.find(fileName);
	if (lookup != textures.end())
		return lookup->second;

	std::shared_ptr<Texture> texture = std::make_shared<Texture>(Texture(fileName));
	textures.insert({ { fileName, texture } });

	return texture;
}
