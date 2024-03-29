#include <../shader_s.h>

//Utilidad de comprobación de errores de linkado o compilación del shader
void Shader::checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024];
	if (type != "PROGRAM")
	{
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
	else
	{
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
		}
	}
}

//Constructor del shader
Shader::Shader() {}
Shader::Shader(const char* vertexPath, const char* fragmentPath, const char* geometryPath)
{
	//Leemos los shaders desde su archivo
	std::string vertexCode;
	std::string fragmentCode;
	std::string geometryCode;
	std::ifstream vShaderFile;
	std::ifstream fShaderFile;
	std::ifstream gShaderFile;
	// ensure ifstream objects can throw exceptions:
	vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	gShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
	try
	{
		// open files
		if (Shaders::shaderLocation.find(vertexPath) == Shaders::shaderLocation.end())
		{
			vShaderFile.open(vertexPath);
			std::stringstream vShaderStream;
			// read file's buffer content into stream
			vShaderStream << vShaderFile.rdbuf();
			// close file handler
			vShaderFile.close();
			// convert stream into string
			vertexCode = vShaderStream.str();
			Shaders::shaderLocation[vertexPath] = vertexCode;
		}
		else vertexCode = Shaders::shaderLocation.at(vertexPath);

		if (Shaders::shaderLocation.find(fragmentPath) == Shaders::shaderLocation.end())
		{
			fShaderFile.open(fragmentPath);
			std::stringstream fShaderStream;
			// read file's buffer content into stream
			fShaderStream << fShaderFile.rdbuf();
			// close file handler
			fShaderFile.close();
			// convert stream into string
			fragmentCode = fShaderStream.str();
			Shaders::shaderLocation[fragmentPath] = fragmentCode;
		}
		else fragmentCode = Shaders::shaderLocation.at(fragmentPath);
		
		// if geometry shader path is present, also load a geometry shader
		if (geometryPath != nullptr)
		{
			if (Shaders::shaderLocation.find(geometryPath) == Shaders::shaderLocation.end())
			{
				gShaderFile.open(geometryPath);
				std::stringstream gShaderStream;
				gShaderStream << gShaderFile.rdbuf();
				gShaderFile.close();
				geometryCode = gShaderStream.str();
				Shaders::shaderLocation[geometryPath] = geometryCode;
			}
			else geometryCode = Shaders::shaderLocation.at(geometryPath);
		}
	}
	catch (std::ifstream::failure& e)
	{
		std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
	}
	const char* vShaderCode = vertexCode.c_str();
	const char* fShaderCode = fragmentCode.c_str();

	//Compilamos los shaders
	GLuint vertex, fragment;
	vertex = compileShader(vShaderCode, GL_VERTEX_SHADER, "VERTEX"); //Vertex shader
	fragment = compileShader(fShaderCode, GL_FRAGMENT_SHADER, "FRAGMENT"); //Fragment shader

	//Si disponemos de un geometry shader, lo compilamos
	unsigned int geometry;
	if (geometryPath != nullptr)
	{
		const char* gShaderCode = geometryCode.c_str();
		geometry = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometry, 1, &gShaderCode, NULL);
		glCompileShader(geometry);
		checkCompileErrors(geometry, "GEOMETRY");
	}

	//Preparamos el programa del shader
	ID = glCreateProgram();
	glAttachShader(ID, vertex);
	glAttachShader(ID, fragment);
	if (geometryPath != nullptr)
		glAttachShader(ID, geometry);
	glLinkProgram(ID);
	checkCompileErrors(ID, "PROGRAM");

	//Eliminamos los shaders. Ya están linkados al programa y deja de ser necesario que estén en memoria
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	if (geometryPath != nullptr)
		glDeleteShader(geometry);
}

//Activar shader
void Shader::use() { glUseProgram(ID); }

//Funciones uniform
void Shader::setBool(const std::string& name, bool value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value); }
void Shader::setInt(const std::string& name, int value) const { glUniform1i(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setFloat(const std::string& name, float value) const { glUniform1f(glGetUniformLocation(ID, name.c_str()), value); }
void Shader::setVec2(const std::string& name, const glm::vec2& value) const { glUniform2fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec2(const std::string& name, float x, float y) const { glUniform2f(glGetUniformLocation(ID, name.c_str()), x, y); }
void Shader::setVec3(const std::string& name, const glm::vec3& value) const { glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec3(const std::string& name, float x, float y, float z) const { glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z); }
void Shader::setVec4(const std::string& name, const glm::vec4& value) const { glUniform4fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]); }
void Shader::setVec4(const std::string& name, float x, float y, float z, float w) { glUniform4f(glGetUniformLocation(ID, name.c_str()), x, y, z, w); }
void Shader::setMat2(const std::string& name, const glm::mat2& mat) const { glUniformMatrix2fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
void Shader::setMat3(const std::string& name, const glm::mat3& mat) const { glUniformMatrix3fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }
void Shader::setMat4(const std::string& name, const glm::mat4& mat) const { glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]); }

unsigned int Shader::GetID()
{
	return ID;
}
