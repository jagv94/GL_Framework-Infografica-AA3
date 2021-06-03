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

class Object
{
private:
	const char* modelPath;
	const char* texturePath;
	bool flip = false;
	Shader ourShader;

	// Read our .obj file
	std::vector< glm::vec3 > vertices;
	std::vector< glm::vec2 > uvs;
	std::vector< glm::vec3 > normals;
	std::vector< glm::vec3 > objectIdx;
	unsigned int texture;
	unsigned int fboTex;

	GLuint objectVao;
	GLuint objectVbo[3]; //objectVbo[4]; //Numero de componentes que tenemos (vertices, normales, texturas (UVs), indices)
	GLuint objectShaders[2];
	GLuint objectProgram;
	glm::mat4 objMat = glm::mat4(1.f);
	glm::vec3 randomizedVec;

public:
	float color[4]{ 1.f, 1.f, 1.f, 1.f }; //Modificador del color del objeto
	float pos[3]{ 0.f, 0.f, 0.f }; //Posicion del objeto
	float rotation = 0.f; //Rotacion del objeto
	float axisRotation[3]{ 0.f, 1.f, 0.f }; //Eje de rotacion del objeto
	float scale[3]{ 1.f, 1.f, 1.f }; //Escala del objeto
	
	Object();
	Object(const char* _modelPath, unsigned int _texture, Shader _shader);

	void draw(float _pos[], float _rotation, float _axisRotation[], float _scale[], float _color[], float _ambientColor[], float _ambientIntensity, float _difuseIntensity,
		float _difuseColor[], float _lightDirection[], float _pointPos[], float _specularColor[], float _specularIntensity,
		int _specularDesity, int _lightSelection, glm::mat4 _modelView, glm::mat4 _MVP);
	void draw(float _pos[], float _rotation, float _axisRotation[], float _scale[], unsigned int _framebuffer, glm::mat4 _modelView, glm::mat4 _MVP);
	void update(const glm::mat4& transform);
	void cleanup();
};