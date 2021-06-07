#pragma once
#include "GL_framework.h"
#include <../shader_s.h>
#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>
#include <cstdio>
#include <cassert>
#include <vector>
#include "../TextureManager.h"

class CubeMap
{
	const char* modelPath;
	bool flip = false;
	Shader ourShader;

	// Read our .obj file
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	std::vector< glm::vec3 > objectIdx;
	unsigned int texture;

	GLuint objectVao;
	GLuint objectVbo[3]; //objectVbo[4]; //Numero de componentes que tenemos (vertices, normales, texturas (UVs), indices)
	GLuint objectShaders[2];
	GLuint objectProgram;
	glm::mat4 objMat = glm::mat4(1.f);
	glm::vec3 randomizedVec;


public:
	CubeMap(const char* _modelPath, unsigned int _texture, Shader _shader);

	unsigned int Start(std::vector<std::string> faces);

	

	void Draw(glm::mat4 cameraView, glm::mat4 projection);

};

