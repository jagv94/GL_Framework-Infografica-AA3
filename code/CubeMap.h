#pragma once
#include <GL/glew.h>
#include <vector>
#include <string>
#include <iostream>
#include "stb_image.h"

class CubeMap
{
	unsigned int textureID;


public:
	CubeMap(std::vector<std::string> faces){}

	unsigned int ReturnIDTexture() { return textureID; }

};

