#include <GL/glew.h>
#include <iostream>

class Framebuffer {
public:
	Framebuffer();
	Framebuffer(unsigned int& _fbo, unsigned int& _fboTex);
};