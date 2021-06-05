#include <GL/glew.h>
#include <iostream>
#include <glm/detail/type_mat.hpp>

namespace RenderVars {
	extern glm::mat4 _projection;
	extern glm::mat4 _modelView;
	extern glm::mat4 _MVP;
	extern glm::mat4 _inv_modelview;
}
namespace RV = RenderVars;

class Framebuffer {
public:
	Framebuffer();
	Framebuffer(unsigned int& _fbo, unsigned int& _fboTex, int _width, int _height);
};