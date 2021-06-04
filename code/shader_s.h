#ifndef SHADER_H
#define SHADER_H

//#include <glad/glad.h>
#include <GL/glew.h>
#include <glm/glm.hpp>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

extern GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name);

class Shader
{
private:
	GLuint vao;
	GLuint vbo;

	//Utilidad de comprobación de errores de linkado o compilación del shader
	void checkCompileErrors(GLuint shader, std::string type);

public:
	unsigned int ID = 0;
	float* ambientColor = new float[4];
	float ambientIntensity;
	float difuseIntensity;
	float* difuseColor = new float[4];
	float* lightDirection = new float[4];
	float* pointPos = new float[4];
	float* specularColor = new float[4];
	float specularIntensity;
	int specularDensity;
	int lightSelection;

	//Constructor
	Shader();
	Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath = nullptr);

	//Activar shader
	void use();

	//Funciones uniform
	void setBool(const std::string& name, bool value) const;
	void setInt(const std::string& name, int value) const;
	void setFloat(const std::string& name, float value) const;
	void setVec2(const std::string& name, const glm::vec2& value) const;
	void setVec2(const std::string& name, float x, float y) const;
	void setVec3(const std::string& name, const glm::vec3& value) const;
	void setVec3(const std::string& name, float x, float y, float z) const;
	void setVec4(const std::string& name, const glm::vec4& value) const;
	void setVec4(const std::string& name, float x, float y, float z, float w);
	void setMat2(const std::string& name, const glm::mat2& mat) const;
	void setMat3(const std::string& name, const glm::mat3& mat) const;
	void setMat4(const std::string& name, const glm::mat4& mat) const;

	unsigned int GetID();
};
#endif