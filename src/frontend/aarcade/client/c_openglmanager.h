#ifndef C_OPENGL_MANAGER_H
#define C_OPENGL_MANAGER_H

// openGL stuff
#include <stdio.h>
#include <stdlib.h>
// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>
#include <GLFW/glfw3.h>
//#include <glm/glm.hpp>

struct OpenGLInfo_t
{
	int state;
	GLFWwindow* window;
	bool close;
};

class C_OpenGLManager
{
public:
	C_OpenGLManager();
	~C_OpenGLManager();

	void Init();

private:
	OpenGLInfo_t* m_info;
};

#endif