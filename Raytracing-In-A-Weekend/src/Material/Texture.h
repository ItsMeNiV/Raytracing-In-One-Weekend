#pragma once
#include "stb_image.h"

class Texture
{
public:
	Texture(const char* path)
		: textureData(nullptr), textureWidth(-1), textureHeight(-1)
	{
		int nrChannels;
		float* data = stbi_loadf(path, &textureWidth, &textureHeight, &nrChannels, 0);
		if (data)
		{
			unsigned int dataSize = textureWidth * textureHeight;
			textureData = new glm::vec3[dataSize];
			unsigned int j = 0;
			for (unsigned int i = 0; i < dataSize; i++)
			{
				textureData[i].x = data[j++];
				textureData[i].y = data[j++];
				textureData[i].z = data[j++];
			}
		}
		else
		{
			std::cout << "Failed to load texture" << std::endl;
		}
		stbi_image_free(data);
	}

	~Texture()
	{
		if(textureData)
			delete[] textureData;
	}

	glm::vec3& At(float uCoord, float vCoord)
	{
		int texelX = (uCoord * textureWidth) - 0.5f;
		int texelY = ((1-vCoord) * textureHeight) - 0.5f;

		return textureData[texelY * textureHeight + texelX];
	}

private:
	glm::vec3* textureData;
	int textureWidth, textureHeight;
};