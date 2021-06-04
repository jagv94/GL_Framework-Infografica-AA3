#include <glm/gtc/random.hpp>
#include <../Object.h>
#include "../Billboard.h"
#include "../Framebuffer.h"
#include "../TextureManager.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

#pragma region Variables
//Texturas
TextureManager metalTex;
TextureManager mesaTex;
TextureManager camaroTex;

//Shaders
Shader objectShader, texturedShader, billboardShader, toonShader, nonTexturedShader, texturedShaderNoWind, texturedShaderTransparency;

//Objetos
Object mesa, bmw, cube, camaro, camaro2;

//Billboards
//Billboard billboard;

//Framebuffers
Framebuffer framebuffer; //Framebuffer por objeto o una vez se settea el framebuffer, devolver los datos a la variable _framebuffer
unsigned int fbo;
unsigned int fboTex;

//Variables de shader
namespace ShaderVariables {
	float ambientColor[4] = { 1.f, 1.f, 1.f, 1.f }; //Color de la luz ambiente
	float ambientIntensity = 0.5f; //Intensidad de la luz ambiente
	float difuseIntensity = 0.5f; //Intensidad de la luz difusa
	float difuseColor[4] = { 0.5f, 0.5f, 0.5f, 0.5f }; //Color de la luz difusa
	float lightDirection[4] = { 0.0f, 0.1f, 0.0f, 0.0f }; //Direcci�n de la luz direccional
	float pointPos[4] = { 0.0f, 40.0f, -20.0f, 0.0f }; //Posici�n de la PointLight
	float specularColor[4] = { 1.0f, 0.0f, 0.0f, 0.0f }; //Color de la luz especular
	float specularIntensity = 0.5f; //Intensidad de la luz especular
	int specularDensity = 32; //Densidad de la luz especular
} namespace SV = ShaderVariables;

//Variables de c�mara
float camPos[2] = { 0.0f, -31.95f }; //Posici�n de la c�mara
float camRot[2] = { 0.0f, 0.5f }; //Rotaci�n de la c�mara
float zoom = 62.0f; //Posici�n de la c�mara en el eje Z, o zoom
float fov = 65.0f; //Campo de visi�n de la c�mara

//Other variables
int gWidth, gHeight; //Variables globales del ancho y alto de la pantalla
#pragma endregion

///////// fw decl
namespace ImGui {
	void Render();
}

namespace Axis {
	void setupAxis();
	void cleanupAxis();
	void drawAxis();
}
////////////////

namespace RenderVars {
	const float FOV = glm::radians(65.f);
	const float zNear = 1.f;
	const float zFar = 50.f;

	glm::mat4 _projection;
	glm::mat4 _modelView;
	glm::mat4 _MVP;
	glm::mat4 _inv_modelview;
	glm::vec4 _cameraPoint;

	struct prevMouse {
		float lastx, lasty;
		MouseEvent::Button button = MouseEvent::Button::None;
		bool waspressed = false;
	} prevMouse;

	float panv[3] = { 0.f, -5.f, -15.f };
	float rota[2] = { 0.f, 0.f };
}
namespace RV = RenderVars;

void GLResize(int width, int height) {
	glViewport(0, 0, width, height);
	if (height != 0) RV::_projection = glm::perspective(glm::radians(fov), (float)width / (float)height, RV::zNear, RV::zFar * 100);
	else RV::_projection = glm::perspective(glm::radians(fov), 0.f, RV::zNear, RV::zFar * 100);
}

void GLmousecb(MouseEvent ev) {
	if (RV::prevMouse.waspressed && RV::prevMouse.button == ev.button) {
		float diffx = ev.posx - RV::prevMouse.lastx;
		float diffy = ev.posy - RV::prevMouse.lasty;
		switch (ev.button) {
		case MouseEvent::Button::Left: // ROTATE
			RV::rota[0] += diffx * 0.005f;
			RV::rota[1] += diffy * 0.005f;
			break;
		case MouseEvent::Button::Right: // MOVE XY
			RV::panv[0] += diffx * 0.03f;
			RV::panv[1] -= diffy * 0.03f;
			break;
		case MouseEvent::Button::Middle: // MOVE Z
			RV::panv[2] += diffy * 0.05f;
			break;
		default: break;
		}
	}
	else {
		RV::prevMouse.button = ev.button;
		RV::prevMouse.waspressed = true;
	}
	RV::prevMouse.lastx = ev.posx;
	RV::prevMouse.lasty = ev.posy;
}

//////////////////////////////////////////////////
GLuint compileShader(const char* shaderStr, GLenum shaderType, const char* name = "") {
	GLuint shader = glCreateShader(shaderType);
	glShaderSource(shader, 1, &shaderStr, NULL);
	glCompileShader(shader);
	GLint res;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &res);
	if (res == GL_FALSE) {
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &res);
		char* buff = new char[res];
		glGetShaderInfoLog(shader, res, &res, buff);
		fprintf(stderr, "Error Shader %s: %s", name, buff);
		delete[] buff;
		glDeleteShader(shader);
		return 0;
	}
	return shader;
}

void linkProgram(GLuint program) {
	glLinkProgram(program);
	GLint res;
	glGetProgramiv(program, GL_LINK_STATUS, &res);
	if (res == GL_FALSE) {
		glGetProgramiv(program, GL_INFO_LOG_LENGTH, &res);
		char* buff = new char[res];
		glGetProgramInfoLog(program, res, &res, buff);
		fprintf(stderr, "Error Link: %s", buff);
		delete[] buff;
	}
}

glm::vec3 randomize(float _min, float _max) {
	float x = glm::linearRand(_min, _max);
	float y = glm::linearRand(_min, _max);
	float z = glm::linearRand(_min, _max);

	return glm::vec3(x, y, z);
}

////////////////////////////////////////////////// AXIS
namespace Axis {
	GLuint AxisVao;
	GLuint AxisVbo[3];
	GLuint AxisShader[2];
	GLuint AxisProgram;

	float AxisVerts[] = {
		0.0, 0.0, 0.0,
		1.0, 0.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 1.0, 0.0,
		0.0, 0.0, 0.0,
		0.0, 0.0, 1.0
	};
	float AxisColors[] = {
		1.0, 0.0, 0.0, 1.0,
		1.0, 0.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 1.0, 0.0, 1.0,
		0.0, 0.0, 1.0, 1.0,
		0.0, 0.0, 1.0, 1.0
	};
	GLubyte AxisIdx[] = {
		0, 1,
		2, 3,
		4, 5
	};
	const char* Axis_vertShader =
		"#version 330\n\
in vec3 in_Position;\n\
in vec4 in_Color;\n\
out vec4 vert_color;\n\
uniform mat4 mvpMat;\n\
void main() {\n\
	vert_color = in_Color;\n\
	gl_Position = mvpMat * vec4(in_Position, 1.0);\n\
}";
	const char* Axis_fragShader =
		"#version 330\n\
in vec4 vert_color;\n\
out vec4 out_Color;\n\
void main() {\n\
	out_Color = vert_color;\n\
}";

	void setupAxis() {
		glGenVertexArrays(1, &AxisVao);
		glBindVertexArray(AxisVao);
		glGenBuffers(3, AxisVbo);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[0]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisVerts, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)0, 3, GL_FLOAT, GL_FALSE, 0, 0);
		glEnableVertexAttribArray(0);

		glBindBuffer(GL_ARRAY_BUFFER, AxisVbo[1]);
		glBufferData(GL_ARRAY_BUFFER, sizeof(float) * 24, AxisColors, GL_STATIC_DRAW);
		glVertexAttribPointer((GLuint)1, 4, GL_FLOAT, false, 0, 0);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, AxisVbo[2]);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(GLubyte) * 6, AxisIdx, GL_STATIC_DRAW);

		glBindVertexArray(0);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

		AxisShader[0] = compileShader(Axis_vertShader, GL_VERTEX_SHADER, "AxisVert");
		AxisShader[1] = compileShader(Axis_fragShader, GL_FRAGMENT_SHADER, "AxisFrag");

		AxisProgram = glCreateProgram();
		glAttachShader(AxisProgram, AxisShader[0]);
		glAttachShader(AxisProgram, AxisShader[1]);
		glBindAttribLocation(AxisProgram, 0, "in_Position");
		glBindAttribLocation(AxisProgram, 1, "in_Color");
		linkProgram(AxisProgram);
	}
	void cleanupAxis() {
		glDeleteBuffers(3, AxisVbo);
		glDeleteVertexArrays(1, &AxisVao);

		glDeleteProgram(AxisProgram);
		glDeleteShader(AxisShader[0]);
		glDeleteShader(AxisShader[1]);
	}
	void drawAxis() {
		glBindVertexArray(AxisVao);
		glUseProgram(AxisProgram);
		glUniformMatrix4fv(glGetUniformLocation(AxisProgram, "mvpMat"), 1, GL_FALSE, glm::value_ptr(RV::_MVP));
		glDrawElements(GL_LINES, 6, GL_UNSIGNED_BYTE, 0);

		glUseProgram(0);
		glBindVertexArray(0);
	}
}

namespace Textures {
	std::map<const char*, unsigned char*> imgLocation;
}

namespace Shaders {
	std::map<const char*, std::string> shaderLocation;
}

GLuint program;
GLuint VAO;
GLuint VBO;

float vertex[9] = {
	-0.5f, -0.5f, 0.0f,
	0.5, -0.5f, 0.0f,
	0.0, 0.5f, 0.0f
};

GLuint unifLocation;

#pragma region Main
void GLinit(int width, int height) {
	srand(time(nullptr));

	gWidth = width;
	gHeight = height;
	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	//glEnable(GL_STENCIL_TEST);
	//glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(glm::radians(fov), (float)width / (float)height, RV::zNear, RV::zFar * 100);

	// Setup shaders & geometry
	Axis::setupAxis();

	/////////////////////////////////////////////////////TODO
	// Do your init code here

	//Preparamos las texturas
	metalTex = TextureManager("resources/metal.jpg", true);
	mesaTex = TextureManager("resources/mesaColor.png", true);
	camaroTex = TextureManager("resources/Camaro_AlbedoTransparency_alt.png", true);

	//Preparamos el framebuffer
	framebuffer = Framebuffer(fbo, fboTex);

	//Preparamos los shaders
	//texturedExplosionShader = Shader("shaders/vertexExplosion.vs", "shaders/texturedFragment.fs", "shaders/explosionGeometry.gs");
	texturedShader = Shader("shaders/texturedVertex.vs", "shaders/texturedFragment.fs");
	texturedShaderNoWind = Shader("shaders/texturedVertex.vs", "shaders/texturedFragmentDiscardWind.fs");
	texturedShaderTransparency = Shader("shaders/texturedVertex.vs", "shaders/texturedFragmentTransparency.fs");
	nonTexturedShader = Shader("shaders/vertex.vs", "shaders/fragment.fs");
	//billboardShader = Shader("shaders/texturedVertex.vs", "shaders/billboardFragment.fs", "shaders/billboardGeometry.gs");
	toonShader = Shader("shaders/texturedVertex.vs", "shaders/toon.fs");

	//Preparamos los objetos a utilizar
	bmw = Object("resources/BMWX5.obj", metalTex.GetImg(), texturedShader);
	mesa = Object("resources/mesa.obj", mesaTex.GetImg(), toonShader);
	camaro = Object("resources/Camaro.obj", camaroTex.GetImg(), texturedShaderNoWind);
	camaro2 = Object("resources/Camaro.obj", camaroTex.GetImg(), texturedShaderTransparency);
	//bmw = Object("resources/BMWX5.obj", nullptr, true, nonTexturedShader);
	//mesa = Object("resources/mesa.obj", nullptr, true, nonTexturedShader);
	//cube = Object("resources/cube.obj", "resources/checker.jpg", true, texturedShader);
	cube = Object("resources/cube.obj", fboTex, texturedShader);

	//billboard = Billboard("resources/arbol.png", true, billboardShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	camaro.pos[0] = 20.f;
	camaro.pos[1] = 40.f;
	camaro.pos[2] = -12.f;
	camaro.scale[0] = 0.3f;
	camaro.scale[1] = 0.3f;
	camaro.scale[2] = 0.3f;
	/*billboard.pos[0] = -15.f;
	billboard.pos[1] = 50.f;*/
	cube.pos[0] = -20.f;
	cube.pos[1] = 60.f;
	cube.pos[2] = -2.f;
	cube.scale[0] = 10.f;
	cube.scale[1] = 10.f;
	cube.scale[2] = 10.f;

	/////////////////////////////////////////////////////////
}

void GLcleanup() {
	Axis::cleanupAxis();

	/////////////////////////////////////////////////////TODO
	// Do your cleanup code here

	camaro.cleanup();
	mesa.cleanup();
	cube.cleanup();
	//billboard.cleanup();

	/////////////////////////////////////////////////////////
}

void GLrender(float dt) {
#pragma region FrameBuffer
	//FrameBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);
	//glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
	//glStencilMask(0x00);
	//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo);
	//////////
	//RV::_projection = glm::perspective(glm::radians(fov), (float)gWidth / (float)gHeight, RV::zNear, RV::zFar * 100); //Aumentamos la distancia de dibujado a por cien para evitar problemas en el dibujado de nuestros objetos.

	//En la transformaci�n y rotaci�n del model view para la c�mara, �tilizamos las variables previamente preparadas camPos, camRot y zoom, para controlar la c�mara desde la interfaz de ser necesario.
	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0] + camPos[0], RV::panv[1] + camPos[1], RV::panv[2] - zoom));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1] + camRot[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0] + camRot[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_MVP = RV::_projection * RV::_modelView;

	Axis::drawAxis();

	/////////////////////////////////////////////////////TODO
	// Do your render code here

	camaro.draw();
	mesa.draw();
	cube.draw(fboTex);

	//billboard.draw();

	/////////////////////////////////////////////////////////

	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion

#pragma region ClassicRender
	
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//RV::_projection = glm::perspective(glm::radians(fov), (float)gWidth / (float)gHeight, RV::zNear, RV::zFar * 100); //Aumentamos la distancia de dibujado a por cien para evitar problemas en el dibujado de nuestros objetos.

	//En la transformaci�n y rotaci�n del model view para la c�mara, �tilizamos las variables previamente preparadas camPos, camRot y zoom, para controlar la c�mara desde la interfaz de ser necesario.
	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0] + camPos[0], RV::panv[1] + camPos[1], RV::panv[2] - zoom));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1] + camRot[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0] + camRot[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_MVP = RV::_projection * RV::_modelView;

	Axis::drawAxis();

	/////////////////////////////////////////////////////TODO
	// Do your render code here

	mesa.draw(mesa.pos, mesa.rotation, mesa.axisRotation, mesa.scale, mesa.color, ambientColor, ambientIntensity, difuseIntensity, difuseColor, lightDirection, pointPos,
		specularColor, specularIntensity, specularDensity, lightSelection, RenderVars::_modelView, RenderVars::_MVP);

	cube.draw(cube.pos, cube.rotation, cube.axisRotation, cube.scale, fboTex, RenderVars::_modelView, RenderVars::_MVP);

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF);

	camaro.draw(camaro.pos, camaro.rotation, camaro.axisRotation, camaro.scale, camaro.color, ambientColor, ambientIntensity, difuseIntensity, difuseColor, lightDirection, pointPos,
		specularColor, specularIntensity, specularDensity, lightSelection, RenderVars::_modelView, RenderVars::_MVP);

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0x00);
	glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	camaro2.draw(camaro.pos, camaro.rotation, camaro.axisRotation, camaro.scale, camaro.color, ambientColor, ambientIntensity, difuseIntensity, difuseColor, lightDirection, pointPos,
		specularColor, specularIntensity, specularDensity, lightSelection, RenderVars::_modelView, RenderVars::_MVP);

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);
	

	//billboard.draw(billboard.pos, RenderVars::_modelView, RenderVars::_MVP);

		/////////////////////////////////////////////////////////

	glBindVertexArray(0);

	ImGui::Render();
#pragma endregion
}

void GUI() {
	bool show = true;
	ImGui::Begin("Physics Parameters", &show, 0);

	{
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		/////////////////////////////////////////////////////TODO
		// Do your GUI code here....

		if (ImGui::CollapsingHeader("Iluminacion")) {
			ImGui::Indent();
			if (ImGui::CollapsingHeader("Luz")) {
				ImGui::Indent();
				ImGui::DragFloat4("Direccion Luz Direccional", SV::lightDirection, 0.05f, -1.0f, 1.0f);
				ImGui::Unindent();
			}

			if (ImGui::CollapsingHeader("Ambiente")) {
				ImGui::Indent();
				ImGui::ColorEdit4("Color Ambiente", SV::ambientColor);
				ImGui::DragFloat("Intensidad Ambiente", &SV::ambientIntensity, 0.005f, 0.0f, 1.0f);
				ImGui::Unindent();
			}

			if (ImGui::CollapsingHeader("Difuso")) {
				ImGui::Indent();
				ImGui::ColorEdit4("Color Difuso", SV::difuseColor);
				ImGui::DragFloat("Intensidad Difuso", &SV::difuseIntensity, 0.005f, 0.0f, 1.0f);
				ImGui::Unindent();
			}

			if (ImGui::CollapsingHeader("Specular")) {
				ImGui::Indent();
				ImGui::ColorEdit4("Color Specular", SV::specularColor);
				ImGui::DragFloat("Intensidad Specular", &SV::specularIntensity, 0.005f, 0.0f, 5.0f);
				ImGui::DragInt("Densidad Specular", &SV::specularDensity, 2, 0, 512);
				ImGui::Unindent();
			}
			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("BMW")) {
			ImGui::Indent();
			ImGui::ColorEdit4("Color BMW", camaro.color);
			ImGui::DragFloat3("Posicion BMW", camaro.pos, 0.5f, -150, 150);
			ImGui::DragFloat3("Eje de rotacion BMW", camaro.axisRotation, 1.f, 0.f, 1.f);
			ImGui::DragFloat("Rotacion BMW", &camaro.rotation, 0.5f, -180, 180);
			ImGui::DragFloat3("Escala BMW", camaro.scale, 0.1f, -100, 100);
			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("Mesa")) {
			ImGui::Indent();
			ImGui::ColorEdit4("Color mesa", mesa.color);
			ImGui::DragFloat3("Posicion mesa", mesa.pos, 0.5f, -150, 150);
			ImGui::DragFloat3("Eje de rotacion mesa", mesa.axisRotation, 1.f, 0.f, 1.f);
			ImGui::DragFloat("Rotacion mesa", &mesa.rotation, 0.5f, -180, 180);
			ImGui::DragFloat3("Escala mesa", mesa.scale, 0.1f, -100, 100);
			ImGui::Unindent();
		}

		if (ImGui::CollapsingHeader("Cubo")) {
			ImGui::Indent();
			ImGui::ColorEdit4("Color cubo", cube.color);
			ImGui::DragFloat3("Posicion cubo", cube.pos, 0.5f, -150, 150);
			ImGui::DragFloat3("Eje de rotacion cubo", cube.axisRotation, 1.f, 0.f, 1.f);
			ImGui::DragFloat("Rotacion cubo", &cube.rotation, 0.5f, -180, 180);
			ImGui::DragFloat3("Escala cubo", cube.scale, 0.1f, -100, 100);
			ImGui::Unindent();
		}

		/*if (ImGui::CollapsingHeader("Billboard")) {
			ImGui::Indent();
			ImGui::ColorEdit4("Color billboard", billboard.color);
			ImGui::DragFloat3("Posicion billboard", billboard.pos, 0.5f, -150, 150);
			ImGui::Unindent();
		}*/

		if (ImGui::CollapsingHeader("Camara")) {
			ImGui::Indent();
			ImGui::DragFloat2("PosX/PosY", camPos, 0.05f, -250, 250);
			ImGui::DragFloat2("RotX/RotY", camRot, 0.05f, -5.0f, 5.0f);
			ImGui::DragFloat("Zoom", &zoom, 0.5f, 1.0f, 100.0f);
			ImGui::DragFloat("Fov", &fov, 0.5f, 30.0f, 110.0f);
			ImGui::Unindent();
		}
	}
	/////////////////////////////////////////////////////////

	ImGui::End();

	// Example code -- ImGui test window. Most of the sample code is in ImGui::ShowTestWindow()
	bool show_test_window = false;
	if (show_test_window) {
		ImGui::SetNextWindowPos(ImVec2(650, 20), ImGuiSetCond_FirstUseEver);
		ImGui::ShowTestWindow(&show_test_window);
	}
}
#pragma endregion