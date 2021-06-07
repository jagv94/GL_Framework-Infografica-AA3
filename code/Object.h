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

namespace RenderVars {
	extern glm::mat4 _projection;
	extern glm::mat4 _modelView;
	extern glm::mat4 _MVP;
	extern glm::mat4 _inv_modelview;
	extern glm::vec4 _cameraPoint;
}
namespace RV = RenderVars;

namespace ShaderVariables {
	extern float ambientColor[4]; //Color de la luz ambiente
	extern float ambientIntensity; //Intensidad de la luz ambiente
	extern float difuseIntensity;  //Intensidad de la luz difusa
	extern float difuseColor[4]; //Color de la luz difusa
	extern float lightDirection[4]; //Dirección de la luz direccional
	extern float pointPos[4]; //Posición de la PointLight
	extern float specularColor[4]; //Color de la luz especular
	extern float specularIntensity; //Intensidad de la luz especular
	extern int specularDensity; //Densidad de la luz especular
}
namespace SV = ShaderVariables;

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
	float scale[3]{ 1.f, 1.f, 1.f }; //Escala del objeto
	
	Object();
	Object(const char* _modelPath, unsigned int _texture, Shader _shader);

	void draw();
	void draw(unsigned int _framebuffer);
	void update(const glm::mat4& transform);
	void cleanup();
};