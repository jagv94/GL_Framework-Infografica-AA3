#pragma once
#include <GL/glew.h>
#include <vector>
#include <string>
#include <iostream>
#include "stb_image.h"

class CubeMap
{
	//unsigned int textureID = 0;


public:
	/*CubeMap(){}
	CubeMap(std::vector<std::string> faces){}*/

	unsigned int Start(std::vector<std::string> faces);

	//unsigned int ReturnIDTexture() { return textureID; }

};

