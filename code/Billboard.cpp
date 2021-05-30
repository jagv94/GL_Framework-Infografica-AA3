#include "Billboard.h"
#include "stb_image.h"

extern void linkProgram(GLuint program);

Billboard::Billboard() {}

Billboard::Billboard(const char* _texturePath, bool _flip, Shader _shader) : texturePath(_texturePath), flip(_flip), ourShader(_shader)
{
	//Cargamos el objeto
	vertices.push_back(glm::vec3(0.0f, 0.0f, 0.0f));

	//Seteamos vertices, normales y UVs
	glGenVertexArrays(1, &objectVao); //Obtenemos ide del objeto
	glBindVertexArray(objectVao); //Bindeamos el objeto (empezamos a acceder a la información del objeto)
	glGenBuffers(3, objectVbo); //Cargamos los datos en GPU

	//Parametrizamos el modelo
	glBindBuffer(GL_ARRAY_BUFFER, objectVbo[0]);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::vec3) * vertices.size(), &vertices[0], GL_STATIC_DRAW);
	glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glEnableVertexAttribArray(0);

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

	//Liberamos memora de la imagen de la textura aquí y en el cleanup
	stbi_image_free(data);

	//Desbindeamos el objeto de memoria
	glBindVertexArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glBindAttribLocation(ourShader.ID, 0, "in_Position");

	linkProgram(ourShader.ID);
}

void Billboard::update(const glm::mat4& _transform)
{
	objMat = _transform;
}

void Billboard::draw(float _pos[], glm::mat4 _modelView, glm::mat4 _MVP)
{
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	glBindVertexArray(objectVao);
	ourShader.use(); //Activamos el shader antes de configurar los uniforms

	glActiveTexture(GL_TEXTURE0); //Activamos la textura antes de bindearla
	glBindTexture(GL_TEXTURE_2D, texture); //Bindeamos la texztura (empezamos a acceder a la información de la textura)

	glm::mat4 t = glm::translate(glm::mat4(), glm::vec3(_pos[0], _pos[1], _pos[2]));
	objMat = t;

	ourShader.setMat4("objMat", objMat);
	ourShader.setMat4("mv_Mat", _modelView);
	ourShader.setMat4("mvpMat", _MVP);

	glDrawArrays(GL_POINTS, 0, vertices.size());

	glUseProgram(0);
	glBindVertexArray(0);
	glDisable(GL_BLEND);
}

void Billboard::cleanup()
{
	glDeleteVertexArrays(1, &objectVao);
	glDeleteBuffers(3, objectVbo);

	glDeleteProgram(ourShader.ID);
	glDeleteShader(objectShaders[0]);
	glDeleteShader(objectShaders[1]);
	glDeleteTextures(1, &texture);
}
