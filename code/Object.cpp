#include "object.h"
#include "stb_image.h"

//Acceso externo a loadOBJ, que nos ayudará a cargar nuestro modelo 3D posteriormente.
extern bool loadOBJ(const char* path, std::vector < glm::vec3 >& out_vertices,
	std::vector < glm::vec2 >& out_uvs, std::vector < glm::vec3 >& out_normals);

extern void linkProgram(GLuint program);
extern glm::vec3 randomize(float _min, float _max);

Object::Object() {}
Object::Object(const char* _modelPath, const char* _texturePath, bool _flip, Shader _shader) : modelPath(_modelPath), texturePath(_texturePath), flip(_flip), ourShader(_shader) {
	//Cargamos el objeto
	bool res = loadOBJ(modelPath, vertices, uvs, normals);

	//Seteamos vertices, normales y UVs
	glGenVertexArrays(1, &objectVao); //Obtenemos ide del objeto
	glBindVertexArray(objectVao); //Bindeamos el objeto (empezamos a acceder a la información del objeto)
	glGenBuffers(3, objectVbo); //Cargamos los datos en GPU

	//Parametrizamos el modelo
	glBindBuffer(GL_ARRAY_BUFFER, objectVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, objectVbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, objectVbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	if (texturePath != nullptr) {
		glGenTextures(1, &texture); //Cargamos en gráfica y accedemos a la variable que contendrá la textura
		glBindTexture(GL_TEXTURE_2D, texture); //Bindeamos la texztura (empezamos a acceder a la información de la textura)

		stbi_set_flip_vertically_on_load(flip); //Giramos la imagen si la vemos al reves
		data = stbi_load(texturePath, &x, &y, &n, 4); //Cargamos la textura
		if (data)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, x, y, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);
		}
		else
		{
			printf("Failed to load texture\n");
		}

		//Filtros de la textura
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		//Liberamos memoria de la imagen de la textura aquí y en el cleanup
		stbi_image_free(data);
	}

	//Desbindeamos el objeto de memoria
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindAttribLocation(ourShader.ID, 0, "in_Position");
	glBindAttribLocation(ourShader.ID, 1, "in_Normal");
	if (texturePath != nullptr) {
		glBindAttribLocation(ourShader.ID, 2, "aTexCoord");
	}
	linkProgram(ourShader.ID);

	randomizedVec = randomize(1.f, 10.f);
}

Object::Object(const char* _modelPath, unsigned int _framebuffer, Shader _shader) : modelPath(_modelPath), fboTex(_framebuffer), ourShader(_shader) {
	//Cargamos el objeto
	bool res = loadOBJ(modelPath, vertices, uvs, normals);

	//Seteamos vertices, normales y UVs
	glGenVertexArrays(1, &objectVao); //Obtenemos ide del objeto
	glBindVertexArray(objectVao); //Bindeamos el objeto (empezamos a acceder a la información del objeto)
	glGenBuffers(3, objectVbo); //Cargamos los datos en GPU

	//Parametrizamos el modelo
	glBindBuffer(GL_ARRAY_BUFFER, objectVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

	glBindBuffer(GL_ARRAY_BUFFER, objectVbo[1]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * normals.size(), &normals[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)1, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(1);

	glBindBuffer(GL_ARRAY_BUFFER, objectVbo[2]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec2) * uvs.size(), &uvs[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)2, 2, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(2);

	glBindTexture(GL_TEXTURE_2D, fboTex); //Bindeamos la texztura (empezamos a acceder a la información de la textura)

	//Filtros de la textura
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

	//Desbindeamos el objeto de memoria
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindAttribLocation(ourShader.ID, 0, "in_Position");
	glBindAttribLocation(ourShader.ID, 1, "in_Normal");
	if (texturePath != nullptr) {
		glBindAttribLocation(ourShader.ID, 2, "aTexCoord");
	}
	linkProgram(ourShader.ID);

	randomizedVec = randomize(1.f, 10.f);
}

void Object::cleanup() {
	glDeleteVertexArrays(1, &objectVao);
	glDeleteBuffers(3, objectVbo);

	glDeleteProgram(ourShader.ID);
	glDeleteShader(objectShaders[0]);
	glDeleteShader(objectShaders[1]);
	if (texturePath != nullptr) glDeleteTextures(1, &texture);
}

void Object::update(const glm::mat4& _transform) {
	objMat = _transform;
}

void Object::draw(float _pos[], float _rotation, float _axisRotation[], float _scale[], float _color[], float _ambientColor[], float _ambientIntensity, float _difuseIntensity,
	float _difuseColor[], float _lightDirection[], float _pointPos[], float _specularColor[], float _specularIntensity,
	int _specularDensity, int _lightSelection, glm::mat4 _modelView, glm::mat4 _MVP) {
	glBindVertexArray(objectVao);

	ourShader.use(); //Activamos el shader antes de configurar los uniforms

	if (texturePath != nullptr) {
		glActiveTexture(GL_TEXTURE0); //Activamos la textura antes de bindearla
		glBindTexture(GL_TEXTURE_2D, texture); //Bindeamos la texztura (empezamos a acceder a la información de la textura)
	}

	glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(_pos[0], _pos[1], _pos[2]));
	glm::mat4 r = glm::rotate(glm::mat4(), glm::radians(_rotation), glm::vec3(_axisRotation[0], _axisRotation[1], _axisRotation[2]));
	glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(_scale[0], _scale[1], _scale[2]));
	objMat = t * r * s;

	ourShader.setMat4("objMat", objMat);
	ourShader.setMat4("mv_Mat", _modelView);
	ourShader.setMat4("mvpMat", _MVP);
	ourShader.setVec4("objectColor", _color[0], _color[1], _color[2], _color[3]);
	ourShader.setVec4("ambientColor", _ambientColor[0], _ambientColor[1], _ambientColor[2], _ambientColor[3]); /*sin(currentTime) * 0.5f + 0.5f, cos(currentTime) * 0.5f + 0.5f, 0.f, 1.f)*/ //Que el color cambie con el tiempo
	ourShader.setVec4("ambientIntensity", _ambientIntensity, _ambientIntensity, _ambientIntensity, _ambientIntensity);
	ourShader.setVec4("difuseIntensity", _difuseIntensity, _difuseIntensity, _difuseIntensity, _difuseIntensity);
	ourShader.setVec4("difuseColor", _difuseColor[0], _difuseColor[1], _difuseColor[2], _difuseColor[3]);
	ourShader.setVec4("directional_light", _lightDirection[0], _lightDirection[1], _lightDirection[2], _lightDirection[3]);
	ourShader.setVec4("pointLight_pos", _pointPos[0], _pointPos[1], _pointPos[2], _pointPos[3]);
	ourShader.setVec4("specularColor", _specularColor[0], _specularColor[1], _specularColor[2], _specularColor[3]);
	ourShader.setVec4("specularIntensity", _specularIntensity, _specularIntensity, _specularIntensity, _specularIntensity);
	ourShader.setFloat("specularDensity", _specularDensity);
	ourShader.setFloat("lightSelection", _lightSelection);
	ourShader.setVec3("randomizedVec", randomizedVec.x, randomizedVec.y, randomizedVec.z);
	ourShader.setFloat("time", ImGui::GetTime());

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glUseProgram(0);
	glBindVertexArray(0);
}
