#include <glm/gtc/random.hpp>
#include <../Object.h>
#include "../Billboard.h"
#include "../Framebuffer.h"
#include "../TextureManager.h"
#include "../CubeMap.h"

#define STB_IMAGE_IMPLEMENTATION
#include "../stb_image.h"

#pragma region Variables
//Texturas
TextureManager billboardTex, camaroTex, sueloTex;

//Shaders
Shader objectShader, texturedShader, billboardShader, toonShader, nonTexturedShader, texturedShaderNoWind, texturedShaderTransparency, skyBoxShader;

//Objetos
Object camaro, camaro2, retrovisor, suelo;

//SkyBox
CubeMap* skyBox;

//Billboards
std::vector<Billboard> billboards;

//Framebuffers
Framebuffer framebuffer; //Framebuffer por objeto o una vez se settea el framebuffer, devolver los datos a la variable _framebuffer
unsigned int fbo;
unsigned int fboTex;
unsigned int cubeTex;
bool activateFBO = false;

//Stencil
bool activateStencil = false;

//Variables de shader
namespace ShaderVariables {
	float ambientColor[4] = { 1.f, 1.f, 1.f, 1.f }; //Color de la luz ambiente
	float ambientIntensity = 0.5f; //Intensidad de la luz ambiente
	float difuseIntensity = 0.5f; //Intensidad de la luz difusa
	float difuseColor[4] = { 0.5f, 0.5f, 0.5f, 0.5f }; //Color de la luz difusa
	float lightDirection[4] = { 0.0f, 0.1f, 0.0f, 0.0f }; //Direccion de la luz direccional
	float pointPos[4] = { 0.0f, 40.0f, -20.0f, 0.0f }; //Posicion de la PointLight
	float specularColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f }; //Color de la luz especular
	float specularIntensity = 0.2f; //Intensidad de la luz especular
	int specularDensity = 2; //Densidad de la luz especular
} namespace SV = ShaderVariables;

//Variables de camara
float camPos[2] = { 0.0f, -24.5f }; //Posicion de la camara
float camRot[2] = { 180.f, 20.f }; //Rotacion de la camara
float zoom = 100.0f; //Posicion de la camara en el eje Z, o zoom
float fov = 65.0f; //Campo de vision de la camara
bool changeCamera = false;
bool cameraReset = false;

//Variables camara framebuffer
float camPosition[] = { 0.f, 0.f, 0.f };
float camRotation[] = { 0.f, 0.f, 0.f };
float fboCamPos[] = { -40.f, -105.f, 20.5f };
float fboCamRot[]{ 0.f, 0.f };
int fboWidth, fboHeight, otherFboWidth, otherFboHeight;

//Billboard
int bilboardsLimitDistance = 1000, billboardsOffset = 150, billboardAmount = 30;

//Other variables
int gWidth, gHeight;
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
	gWidth = width;
	gHeight = height;
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
	gWidth = width;
	gHeight = height;

	srand(time(nullptr));

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
	// 
	//CubeMap-SetTexture
	std::vector<std::string> faces =
	{
		"resources/skybox/right.jpg",
		"resources/skybox/left.jpg",
		"resources/skybox/top.jpg",
		"resources/skybox/bottom.jpg",
		"resources/skybox/front.jpg",
		"resources/skybox/back.jpg"
	};

	cubeTex = skyBox->Start(faces);

	//Inicializamos los objetos de las clases creadas
#pragma region initClasses
	//Preparamos las texturas
	camaroTex = TextureManager("resources/Camaro_AlbedoTransparency_alt.png", true);
	sueloTex = TextureManager("resources/floor.png", true);
	billboardTex = TextureManager("resources/arbol.png", true);

	//Preparamos el framebuffer
	framebuffer = Framebuffer(fbo, fboTex, 600, 200);

	//Preparamos los shaders
	//texturedExplosionShader = Shader("shaders/vertexExplosion.vs", "shaders/texturedFragment.fs", "shaders/explosionGeometry.gs");
	texturedShader = Shader("shaders/texturedVertex.vs", "shaders/texturedFragment.fs");
	texturedShaderNoWind = Shader("shaders/texturedVertex.vs", "shaders/texturedFragmentDiscardWind.fs");
	texturedShaderTransparency = Shader("shaders/texturedVertex.vs", "shaders/texturedFragmentTransparency.fs");
	nonTexturedShader = Shader("shaders/vertex.vs", "shaders/fragment.fs");
	billboardShader = Shader("shaders/texturedVertex.vs", "shaders/billboardFragment.fs", "shaders/billboardGeometry.gs");
	toonShader = Shader("shaders/texturedVertex.vs", "shaders/toon.fs");
	skyBoxShader = Shader("shaders/skyBoxVertex.vs", "shaders/skyBoxFragment.fs");

	//Preparamos los objetos a utilizar
	camaro = Object("resources/Camaro.obj", camaroTex.GetImg(), texturedShaderNoWind);
	camaro2 = Object("resources/Camaro.obj", camaroTex.GetImg(), texturedShaderTransparency);
	suelo = Object("resources/floor.obj", sueloTex.GetImg(), texturedShader);
	retrovisor = Object("resources/retrovisor.obj", fboTex, texturedShader);
	skyBox = new CubeMap("resources/cube.obj", cubeTex, skyBoxShader);

	for (int i = 0; i < billboardAmount; i++) {
		glm::vec3 randomPos = randomize(-1500, 1500);

		while (randomPos.x > -billboardsOffset && randomPos.x < billboardsOffset && randomPos.z > -billboardsOffset && randomPos.z < billboardsOffset)
			randomPos = randomize(-bilboardsLimitDistance, bilboardsLimitDistance);

		billboards.push_back(Billboard(billboardTex.GetImg(), billboardShader));
		billboards.at(i).pos[1] = 56.f;
		billboards.at(i).pos[0] = randomPos.x;
		billboards.at(i).pos[2] = randomPos.z;
	}

	

#pragma endregion

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//Seteamos las posiciones iniciales de los objetos en escena
#pragma region initObjectSettings
	camaro.pos[0] = 0.f;
	camaro.pos[1] = 0.f;
	camaro.pos[2] = 0.f;
	camaro.scale[0] = 0.3f;
	camaro.scale[1] = 0.3f;
	camaro.scale[2] = 0.3f;

	camaro2.pos[0] = camaro.pos[0];
	camaro2.pos[1] = camaro.pos[1];
	camaro2.pos[2] = camaro.pos[2];
	camaro2.scale[0] = camaro.scale[0];
	camaro2.scale[1] = camaro.scale[1];
	camaro2.scale[2] = camaro.scale[2];

	retrovisor.scale[0] = 2.f;
	retrovisor.scale[1] = 2.f;
	retrovisor.scale[2] = 2.f;

	suelo.pos[1] = -170.f;

	RV::panv[0] = 0.f;
	RV::panv[1] = -24.f;
	RV::rota[0] = glm::radians(180.f);
	RV::rota[1] = glm::radians(20.f);
	RV::panv[2] = -150.f;
#pragma endregion

	/////////////////////////////////////////////////////////
}

void GLcleanup() {
	Axis::cleanupAxis();

	/////////////////////////////////////////////////////TODO
	// Do your cleanup code here

	camaro.cleanup();
	camaro2.cleanup();
	retrovisor.cleanup();
	suelo.cleanup();

	for each (Billboard var in billboards)
	{
		var.cleanup();
	}

	/////////////////////////////////////////////////////////
}

void GLrender(float dt) {
	//Seteamos las posiciones que queremos que siempre sean iguales
	retrovisor.rotation = camaro.rotation;
	retrovisor.pos[0] = camaro.pos[0] - 0.15f;
	retrovisor.pos[1] = camaro.pos[1] + 28.f;
	retrovisor.pos[2] = camaro.pos[2] + 8.f;
	fboCamPos[0] = -retrovisor.pos[0];
	fboCamPos[1] = -retrovisor.pos[1];
	fboCamPos[2] = -retrovisor.pos[2];

	//Seteamos las posiciones para los dos modos de camara
	if (changeCamera) {
		RV::panv[0] = 8.45f;
		RV::panv[1] = -24;
		//RV::rota[0] = glm::radians(180.f);
		RV::rota[1] = glm::radians(0.f);
		RV::panv[2] = -5;
		cameraReset = true;
	}

	else if(cameraReset) {
		RV::panv[0] = 0.f;
		RV::panv[1] = -24.f;
		RV::rota[0] = glm::radians(180.f);
		RV::rota[1] = glm::radians(20.f);
		RV::panv[2] = -150.f;
		cameraReset = false;
	}

#pragma region FrameBuffer
	if (activateFBO) {
		//FrameBuffer
		glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		
		glViewport(0, 0, 600, 200);
		RV::_projection = glm::perspective(glm::radians(fov), (float)600 / (float)200, RV::zNear, RV::zFar * 100);

		//glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
		glClearColor(0.2f, 0.2f, 0.2f, 1.f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
		//glStencilMask(0x00);
		//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, fbo);
		//////////

		//En la transformacion y rotacion del model view para la camara, utilizamos las variables previamente preparadas camPos, camRot y zoom, para controlar la camara desde la interfaz de ser necesario.
		glm::mat4 fboModelView = glm::mat4(1.f); //Creamos el modelView para la camara del framebuffer
		fboModelView = glm::translate(fboModelView, glm::vec3(camaro.pos[0] + fboCamPos[0], camaro.pos[1] + fboCamPos[1], camaro.pos[2] + fboCamPos[2])); //Ajustamos la translacion
		fboModelView = glm::rotate(fboModelView, glm::radians(fboCamRot[1]), glm::vec3(1.f, 0.f, 0.f)); //Ajustamos la rotacion en Y
		fboModelView = glm::rotate(fboModelView, glm::radians(-camaro.rotation + fboCamRot[0]), glm::vec3(0.f, 1.f, 0.f));  //Ajustamos la rotacion en X

		RV::_MVP = RV::_projection * fboModelView;

		Axis::drawAxis();

		/////////////////////////////////////////////////////TODO
		// Do your render code here

		suelo.draw();
		camaro.draw();
		retrovisor.draw(fboTex);

		for each (Billboard var in billboards)
		{
			var.draw();
		}
	}

	/////////////////////////////////////////////////////////

	glBindVertexArray(0);
#pragma endregion

#pragma region ClassicRender
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	GLResize(gWidth, gHeight);
	glStencilMask(0xFF);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

	//En la transformacion y rotacion del model view para la camara, utilizamos las variables previamente preparadas camPos, camRot y zoom, para controlar la camara desde la interfaz de ser necesario.
	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0], RV::panv[1], RV::panv[2]));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_MVP = RV::_projection * RV::_modelView;

	skyBox->Draw(RV::_modelView, RV::_projection);

	Axis::drawAxis();

	/////////////////////////////////////////////////////TODO
	// Do your render code here



	suelo.draw();

	

	if (activateFBO) retrovisor.draw(fboTex);

	for each (Billboard var in billboards)
	{
		var.draw();
	}

	

	glEnable(GL_STENCIL_TEST);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0xFF);

	camaro.draw();

	glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	glStencilMask(0x00);
	//glDisable(GL_DEPTH_TEST);

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	if (activateStencil) {
		camaro2.draw();
	}

	glStencilMask(0xFF);
	glStencilFunc(GL_ALWAYS, 1, 0xFF);
	glStencilOp(GL_KEEP, GL_KEEP, GL_REPLACE);
	//glEnable(GL_DEPTH_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	

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

		if(ImGui::Button("Cambiar camara")) {
			activateFBO = !activateFBO;
			activateStencil = !activateStencil;
			changeCamera = !changeCamera;
		}

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