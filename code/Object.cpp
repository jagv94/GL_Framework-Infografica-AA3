#include "object.h"

//Acceso externo a loadOBJ, que nos ayudará a cargar nuestro modelo 3D posteriormente.
extern bool loadOBJ(const char* path, std::vector < glm::vec3 >& out_vertices,
	std::vector < glm::vec2 >& out_uvs, std::vector < glm::vec3 >& out_normals);

extern void linkProgram(GLuint program);
extern glm::vec3 randomize(float _min, float _max);

Object::Object() {}
Object::Object(const char* _modelPath, unsigned int _texture, Shader _shader) : modelPath(_modelPath), texture(_texture), ourShader(_shader) {
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

	//Desbindeamos el objeto de memoria
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindAttribLocation(ourShader.ID, 0, "in_Position");
	glBindAttribLocation(ourShader.ID, 1, "in_Normal");
	if (texture > 0) {
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

void Object::draw() {
	glBindVertexArray(objectVao);

	ourShader.use(); //Activamos el shader antes de configurar los uniforms

	if (texturePath != nullptr) {
		glActiveTexture(GL_TEXTURE0); //Activamos la textura antes de bindearla
		glBindTexture(GL_TEXTURE_2D, texture); //Bindeamos la texztura (empezamos a acceder a la información de la textura)
	}

	glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(pos[0], pos[1], pos[2]));
	glm::mat4 r = glm::rotate(glm::mat4(), glm::radians(rotation), glm::vec3(0, 1, 0));
	glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(scale[0], scale[1], scale[2]));
	objMat = t * r * s;

	ourShader.setMat4("objMat", objMat);
	ourShader.setMat4("mv_Mat", RV::_modelView);
	ourShader.setMat4("mvpMat", RV::_MVP);
	ourShader.setVec4("objectColor", color[0], color[1], color[2], color[3]);
	ourShader.setVec4("ambientColor", SV::ambientColor[0], SV::ambientColor[1], SV::ambientColor[2], SV::ambientColor[3]); /*sin(currentTime) * 0.5f + 0.5f, cos(currentTime) * 0.5f + 0.5f, 0.f, 1.f)*/ //Que el color cambie con el tiempo
	ourShader.setVec4("ambientIntensity", SV::ambientIntensity, SV::ambientIntensity, SV::ambientIntensity, SV::ambientIntensity);
	ourShader.setVec4("difuseIntensity", SV::difuseIntensity, SV::difuseIntensity, SV::difuseIntensity, SV::difuseIntensity);
	ourShader.setVec4("difuseColor", SV::difuseColor[0], SV::difuseColor[1], SV::difuseColor[2], SV::difuseColor[3]);
	ourShader.setVec4("directional_light", SV::lightDirection[0], SV::lightDirection[1], SV::lightDirection[2], SV::lightDirection[3]);
	ourShader.setVec4("pointLight_pos", SV::pointPos[0], SV::pointPos[1], SV::pointPos[2], SV::pointPos[3]);
	ourShader.setVec4("specularColor", SV::specularColor[0], SV::specularColor[1], SV::specularColor[2], SV::specularColor[3]);
	ourShader.setVec4("specularIntensity", SV::specularIntensity, SV::specularIntensity, SV::specularIntensity, SV::specularIntensity);
	ourShader.setFloat("specularDensity", SV::specularDensity);
	ourShader.setVec3("randomizedVec", randomizedVec.x, randomizedVec.y, randomizedVec.z);
	ourShader.setFloat("time", ImGui::GetTime());

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glUseProgram(0);
	glBindVertexArray(0);
}

void Object::draw(unsigned int _framebuffer) {
	glBindVertexArray(objectVao);

	ourShader.use(); //Activamos el shader antes de configurar los uniforms


	glActiveTexture(GL_TEXTURE0); //Activamos la textura antes de bindearla
	glBindTexture(GL_TEXTURE_2D, _framebuffer); //Bindeamos la texztura (empezamos a acceder a la información de la textura)

	glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(pos[0], pos[1], pos[2]));
	glm::mat4 r = glm::rotate(glm::mat4(), glm::radians(rotation), glm::vec3(0, 1, 0));
	glm::mat4 s = glm::scale(glm::mat4(), glm::vec3(scale[0], scale[1], scale[2]));
	objMat = t * r * s;

	ourShader.setMat4("objMat", objMat);
	ourShader.setMat4("mv_Mat", RV::_modelView);
	ourShader.setMat4("mvpMat", RV::_MVP);
	ourShader.setVec3("randomizedVec", randomizedVec.x, randomizedVec.y, randomizedVec.z);
	ourShader.setFloat("time", ImGui::GetTime());

	glDrawArrays(GL_TRIANGLES, 0, vertices.size());

	glUseProgram(0);
	glBindVertexArray(0);
}
