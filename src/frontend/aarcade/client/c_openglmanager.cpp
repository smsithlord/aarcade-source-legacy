#include "cbase.h"
#include "c_openglmanager.h"

// openGL stuff
#include <stdio.h>
#include <stdlib.h>
// Include GLEW. Always include it before gl.h and glfw.h, since it's a bit magic.
#include <GL/glew.h>
//#include <GLFW/glfw3.h>

// Include GLM
//#include <glm/glm.hpp>
//using namespace glm;

// memdbgon must be the last include file in a .cpp file!!!
#include "tier0/memdbgon.h"

C_OpenGLManager::C_OpenGLManager()
{
	DevMsg("OpenGLManage: Constructor\n");
}

C_OpenGLManager::~C_OpenGLManager()
{
	DevMsg("OpenGLManage: Destructor\n");
}

unsigned MyOpenGLThread(void *params)
{
	OpenGLInfo_t* info = (OpenGLInfo_t*)params; // always use a struct!

	if (!info->close)
	{
		if (glfwInit())
		{
			glfwWindowHint(GLFW_SAMPLES, 4);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
			glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
			glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
			glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); // To make MacOS happy; should not be needed
			//glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);	// invisible window

			//GLFWwindow* window = glfwCreateWindow(640, 480, "", NULL, NULL);
			GLFWwindow* window = window = glfwCreateWindow(640, 480, "My OPENGL", NULL, NULL);

			if (window)
			{
				glewExperimental = GL_TRUE; // Needed in core profile
				glfwMakeContextCurrent(window);
				glewInit();

				DevMsg("Window created!\n");
				info->window = window;

				// get version info
				const GLubyte* renderer = glGetString(GL_RENDERER); // get renderer string
				const GLubyte* version = glGetString(GL_VERSION); // version as a string
				DevMsg("Renderer: %s\n", renderer);
				DevMsg("OpenGL version supported %s\n", version);
				DevMsg("=========================\n");


				// create a triangle
				GLuint VertexArrayID;
				glGenVertexArrays(1, &VertexArrayID);
				glBindVertexArray(VertexArrayID);

				// An array of 3 vectors which represents 3 vertices
				static const GLfloat g_vertex_buffer_data[] = {
					-1.0f, -1.0f, 0.0f,
					1.0f, -1.0f, 0.0f,
					0.0f, 1.0f, 0.0f,
				};

				// This will identify our vertex buffer
				GLuint vertexbuffer;
				// Generate 1 buffer, put the resulting identifier in vertexbuffer
				glGenBuffers(1, &vertexbuffer);
				// The following commands will talk about our 'vertexbuffer' buffer
				glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
				// Give our vertices to OpenGL.
				glBufferData(GL_ARRAY_BUFFER, sizeof(g_vertex_buffer_data), g_vertex_buffer_data, GL_STATIC_DRAW);

				int state;
				while (!info->close && glfwWindowShouldClose(window) == 0) //&& glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
				{
					state = info->state;

					if (state == 0)
					{
						info->state = 1;
						// state left empty for the main thread to catch up and perform logic
					}
					else if (state == 1)
					{
						// background color
						glDisable(GL_DEPTH_TEST); // here for illustrative purposes, depth test is initially DISABLED (key!)
						glClearColor(0.3f, 0.4f, 0.1f, 1.0f);
						glClear(GL_COLOR_BUFFER_BIT);

						// 1rst attribute buffer : vertices
						glEnableVertexAttribArray(0);
						glBindBuffer(GL_ARRAY_BUFFER, vertexbuffer);
						glVertexAttribPointer(
							0,                  // attribute 0. No particular reason for 0, but must match the layout in the shader.
							3,                  // size
							GL_FLOAT,           // type
							GL_FALSE,           // normalized?
							0,                  // stride
							(void*)0            // array buffer offset
							);
						// Draw the triangle !
						glDrawArrays(GL_TRIANGLES, 0, 3); // Starting from vertex 0; 3 vertices total -> 1 triangle
						glDisableVertexAttribArray(0);

						glfwSwapBuffers(window);
						glfwPollEvents();
					}
				}
			}
			else
				DevMsg("Failed to create window!\n");
		}
		else
			DevMsg("Failed to init GLFW!\n");
	}

	if (info)
		delete info;

	DevMsg("Close opengl thread.\n");
	return 0;
}

void C_OpenGLManager::Init()
{
	m_info = new OpenGLInfo_t;
	m_info->state = 0;
	m_info->window = null;
	m_info->close = false;
	CreateSimpleThread(MyOpenGLThread, m_info);
}