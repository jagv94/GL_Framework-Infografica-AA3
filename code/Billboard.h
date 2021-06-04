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

namespace RenderVars {
	extern glm::mat4 _projection;
	extern glm::mat4 _modelView;
	extern glm::mat4 _MVP;
	extern glm::mat4 _inv_modelview;
	extern glm::vec4 _cameraPoint;
}
namespace RV = RenderVars;

class Billboard
{
private:
	Shader ourShader;

	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	std::vector< glm::vec3 > objectIdx;
	unsigned int texture = 0;

	GLuint objectVao = 0;
	GLuint objectVbo[3] = { 0, 0, 0 }; //objectVbo[4]; //Numero de componentes que tenemos (vertices, normales, texturas (UVs), indices)
	GLuint objectShaders[2] = { 0, 0 };
	GLuint objectProgram = 0;
	glm::mat4 objMat = glm::mat4(1.f);

public:
	float color[4]{ 1.f, 1.f, 1.f, 1.f }; //Modificador del color del objeto
	float pos[3]{ 0.f, 0.f, 0.f }; //Posicion del billboard

	Billboard();
	Billboard(unsigned int _texture, Shader _shader);
	void update(const glm::mat4& transform);
	void draw();
	void cleanup();
};