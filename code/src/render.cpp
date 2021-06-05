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

//Shaders
Shader objectShader, texturedShader, billboardShader, toonShader, nonTexturedShader;

//Objetos
Object mesa, bmw, retrovisor;

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
	float lightDirection[4] = { 0.0f, 0.1f, 0.0f, 0.0f }; //Dirección de la luz direccional
	float pointPos[4] = { 0.0f, 40.0f, -20.0f, 0.0f }; //Posición de la PointLight
	float specularColor[4] = { 1.0f, 0.0f, 0.0f, 0.0f }; //Color de la luz especular
	float specularIntensity = 0.5f; //Intensidad de la luz especular
	int specularDensity = 32; //Densidad de la luz especular
} namespace SV = ShaderVariables;

//Variables de cámara
float camPos[2] = { 0.0f, -31.95f }; //Posición de la cámara
float camRot[2] = { 0.0f, 0.5f }; //Rotación de la cámara
float zoom = 62.0f; //Posición de la cámara en el eje Z, o zoom
float fov = 65.0f; //Campo de visión de la cámara

//Variables cámara framebuffer
float fboCamPos[3] = { 13.5f, -51.f, 32.f };
float fboCamRot[2]{ 96.f, -18.5f };
int fboWidth, fboHeight, otherFboWidth, otherFboHeight;
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
	glm::mat4 _projectionAux;
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

	if (RV::_projectionAux == RV::_projection)
	{
		fboWidth = 800 / 1.3;
		fboHeight = 600 / 1.3;
		framebuffer = Framebuffer(fbo, fboTex, fboWidth, fboHeight);
	}

	else
	{
		otherFboWidth = 2560 / 1.3;
		otherFboHeight = 1440 / 1.3;
		framebuffer = Framebuffer(fbo, fboTex, otherFboWidth, otherFboHeight);
	}
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

float camPosition[] = { 0.f, 0.f, 0.f };
float camRotation[] = { 0.f, 0.f, 0.f };

#pragma region Main
void GLinit(int width, int height) {
	srand(time(nullptr));

	glViewport(0, 0, width, height);
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClearDepth(1.f);
	glDepthFunc(GL_LEQUAL);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);

	RV::_projection = glm::perspective(glm::radians(fov), (float)width / (float)height, RV::zNear, RV::zFar * 100);
	RV::_projectionAux = RV::_projection;

	// Setup shaders & geometry
	Axis::setupAxis();

	/////////////////////////////////////////////////////TODO
	// Do your init code here

	//Preparamos las texturas
	metalTex = TextureManager("resources/metal.jpg", true);
	mesaTex = TextureManager("resources/mesaColor.png", true);

	//Preparamos el framebuffer
	fboWidth = width / 1.3;
	fboHeight = height / 1.3;
	framebuffer = Framebuffer(fbo, fboTex, width, height);


	//Preparamos los shaders
	//texturedExplosionShader = Shader("shaders/vertexExplosion.vs", "shaders/texturedFragment.fs", "shaders/explosionGeometry.gs");
	texturedShader = Shader("shaders/texturedVertex.vs", "shaders/texturedFragment.fs");
	nonTexturedShader = Shader("shaders/vertex.vs", "shaders/fragment.fs");
	//billboardShader = Shader("shaders/texturedVertex.vs", "shaders/billboardFragment.fs", "shaders/billboardGeometry.gs");
	toonShader = Shader("shaders/texturedVertex.vs", "shaders/toon.fs");

	//Preparamos los objetos a utilizar
	bmw = Object("resources/BMWX5.obj", metalTex.GetImg(), texturedShader);
	mesa = Object("resources/mesa.obj", mesaTex.GetImg(), texturedShader);
	//bmw = Object("resources/BMWX5.obj", nullptr, true, nonTexturedShader);
	//mesa = Object("resources/mesa.obj", nullptr, true, nonTexturedShader);
	//cube = Object("resources/cube.obj", "resources/checker.jpg", true, texturedShader);
	retrovisor = Object("resources/retrovisor.obj", fboTex, texturedShader);

	//billboard = Billboard("resources/arbol.png", true, billboardShader);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	bmw.pos[0] = 20.f;
	bmw.pos[1] = 40.f;
	bmw.pos[2] = -12.f;
	bmw.scale[0] = 0.3f;
	bmw.scale[1] = 0.3f;
	bmw.scale[2] = 0.3f;
	/*billboard.pos[0] = -15.f;
	billboard.pos[1] = 50.f;*/
	/*retrovisor.pos[0] = 0.f;
	retrovisor.pos[1] = 50.f;
	retrovisor.pos[2] = -40.f;*/
	retrovisor.pos[0] = bmw.pos[0]-3.f;
	retrovisor.pos[1] = bmw.pos[1]+17.5f;
	retrovisor.pos[2] = bmw.pos[2]-0.25f;
	retrovisor.scale[0] = 1.f;
	retrovisor.scale[1] = 1.f;
	retrovisor.scale[2] = 1.f;
	retrovisor.rotation = 90.f;
	camPosition[0] = retrovisor.pos[0];
	camPosition[1] = retrovisor.pos[1];
	camPosition[2] = retrovisor.pos[2];

	/////////////////////////////////////////////////////////
}

void GLcleanup() {
	Axis::cleanupAxis();

	/////////////////////////////////////////////////////TODO
	// Do your cleanup code here

	bmw.cleanup();
	mesa.cleanup();
	retrovisor.cleanup();
	//billboard.cleanup();

	/////////////////////////////////////////////////////////
}

void GLrender(float dt) {
#pragma region FrameBuffer
	//FrameBuffer
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	//glEnable(GL_DEPTH_TEST); // enable depth testing (is disabled for rendering screen-space quad)
	glClearColor(0.2f, 0.2f, 0.2f, 1.f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	//////////

	//En la transformación y rotación del model view para la cámara, útilizamos las variables previamente preparadas camPos, camRot y zoom, para controlar la cámara desde la interfaz de ser necesario.
	glm::mat4 fboModelView = glm::mat4(1.f); //Creamos el modelView para la cámara del framebuffer
	fboModelView = glm::translate(fboModelView, glm::vec3(fboCamPos[0], fboCamPos[1], fboCamPos[2])); //Ajustamos la translación
	fboModelView = glm::rotate(fboModelView, glm::radians(fboCamRot[1]), glm::vec3(1.f, 0.f, 0.f)); //Ajustamos la rotación en Y
	fboModelView = glm::rotate(fboModelView, glm::radians(fboCamRot[0]), glm::vec3(0.f, 1.f, 0.f));  //Ajustamos la rotación en X

	RV::_MVP = RV::_projection * fboModelView;

	Axis::drawAxis();

	/////////////////////////////////////////////////////TODO
	// Do your render code here

	bmw.draw();
	mesa.draw();
	retrovisor.draw(fboTex);

	//billboard.draw();

	/////////////////////////////////////////////////////////

	glBindVertexArray(0);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
#pragma endregion

#pragma region ClassicRender
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	//En la transformación y rotación del model view para la cámara, útilizamos las variables previamente preparadas camPos, camRot y zoom, para controlar la cámara desde la interfaz de ser necesario.
	RV::_modelView = glm::mat4(1.f);
	RV::_modelView = glm::translate(RV::_modelView, glm::vec3(RV::panv[0] + camPos[0], RV::panv[1] + camPos[1], RV::panv[2] - zoom));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[1] + camRot[1], glm::vec3(1.f, 0.f, 0.f));
	RV::_modelView = glm::rotate(RV::_modelView, RV::rota[0] + camRot[0], glm::vec3(0.f, 1.f, 0.f));

	RV::_MVP = RV::_projection * RV::_modelView;

	Axis::drawAxis();

	/////////////////////////////////////////////////////TODO
	// Do your render code here

	bmw.draw();
	mesa.draw();
	retrovisor.draw(fboTex);

	//billboard.draw();

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
			ImGui::ColorEdit4("Color BMW", bmw.color);
			ImGui::DragFloat3("Posicion BMW", bmw.pos, 0.5f, -150, 150);
			ImGui::DragFloat3("Eje de rotacion BMW", bmw.axisRotation, 1.f, 0.f, 1.f);
			ImGui::DragFloat("Rotacion BMW", &bmw.rotation, 0.5f, -180, 180);
			ImGui::DragFloat3("Escala BMW", bmw.scale, 0.1f, -100, 100);
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

		if (ImGui::CollapsingHeader("Retrovisor")) {
			ImGui::Indent();
			ImGui::ColorEdit4("Color retrovisor", retrovisor.color);
			ImGui::DragFloat3("Posicion retrovisor", retrovisor.pos, 0.5f, -150, 150);
			ImGui::DragFloat3("Eje de rotacion retrovisor", retrovisor.axisRotation, 1.f, 0.f, 1.f);
			ImGui::DragFloat("Rotacion retrovisor", &retrovisor.rotation, 0.5f, -180, 180);
			ImGui::DragFloat3("Escala retrovisor", retrovisor.scale, 0.1f, -100, 100);
			ImGui::DragFloat3("Posicion camara retrovisor", fboCamPos, 0.1f, -100, 100);
			ImGui::DragFloat2("Rotacion camara  retrovisor", fboCamRot, 0.5f, -180.f, 180.f);
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