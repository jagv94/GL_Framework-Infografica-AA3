#pragma once
#include <GL/glew.h>
#include <vector>
#include "stb_image.h"
#include <map>

namespace Textures {
	extern std::map<const char*, unsigned char*> imgLocation;
}

class TextureManager
{
private:
	int x = 0, y = 0, n = 0;
	unsigned char* data = nullptr;
	const char* texturePath = nullptr;
	unsigned int img = 0;

public:
	TextureManager();
	TextureManager(const char* _texturePath, bool _fliped);
	const char* GetTexturePath();
	unsigned int GetImg();
};