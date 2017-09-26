#include "texture.hpp"

#include <vector>
#include <iostream>
#include "lodepng.h"

void Texture::loadCubemap(const std::string & path)
{
	const std::string faces[6] = {
		"right",	//GL_TEXTURE_CUBE_MAP_POSITIVE_X
		"left",		//GL_TEXTURE_CUBE_MAP_NEGATIVE_X	
		"top",		//GL_TEXTURE_CUBE_MAP_POSITIVE_Y	
		"bottom",	//GL_TEXTURE_CUBE_MAP_NEGATIVE_Y
		"back",		//GL_TEXTURE_CUBE_MAP_POSITIVE_Z	
		"front",	//GL_TEXTURE_CUBE_MAP_NEGATIVE_Z	
	};

	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_CUBE_MAP, id);

	std::vector<unsigned char> image;
	unsigned width, height;
	for (int i = 0; i < 6; i++)
	{
		unsigned error = lodepng::decode(image, width, height, path + faces[i] + ".png");
		if (error != 0)
		{
			std::cout << (std::to_string(error) + ": " + lodepng_error_text(error));
			system("pause");
			exit(EXIT_FAILURE);
		}
		glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
		image.clear();
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
}

void Texture::load2D(const std::string & path)
{
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	std::vector<unsigned char> image;
	unsigned width, height;
	unsigned error = lodepng::decode(image, width, height, path);
	if (error != 0)
	{
		std::cout << (std::to_string(error) + ": " + lodepng_error_text(error));
		system("pause");
		exit(EXIT_FAILURE);
	}
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, &image[0]);
	image.clear();

}

void Texture::load(const std::string & path, TextureType _type)
{
	type = _type;
	switch (type)
	{
	case TEXTURE_CUBEMAP:
		loadCubemap(path);
		break;
	case TEXTURE_2D:
		load2D(path);
		break;
	}
	glBindTexture(GL_TEXTURE_2D, 0);
}

void Texture::bind(size_t slot)
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(type, id);
}


