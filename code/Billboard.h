#include <GL\glew.h>
#include <glm\gtc\type_ptr.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <cstdio>
#include <cassert>

#include <imgui\imgui.h>
#include <imgui\imgui_impl_sdl_gl3.h>

#include "GL_framework.h"

#include <vector>
#include <../shader_s.h>

class Billboard
{
private:
	const char* texturePath;
	bool flip = false;
	Shader ourShader;

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	std::vector< glm::vec3 > objectIdx;
	int x, y, n;
	unsigned char* data;
	unsigned int texture;

	GLuint objectVao;
	GLuint objectVbo[3]; //objectVbo[4]; //Numero de componentes que tenemos (vertices, normales, texturas (UVs), indices)
	GLuint objectShaders[2];
	GLuint objectProgram;
	glm::mat4 objMat = glm::mat4(1.f);

public:
	float color[4]{ 1.f, 1.f, 1.f, 1.f }; //Modificador del color del objeto
	float pos[3]{ 0.f, 0.f, 0.f }; //Posicion del billboard

	Billboard();
	Billboard(const char* _texturePath, bool _flip, Shader _shader);
	void update(const glm::mat4& transform);
	void draw(float _pos[], glm::mat4 _modelView, glm::mat4 _MVP);
	void cleanup();
};