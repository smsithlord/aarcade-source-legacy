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

#define WGL_DRAW_TO_WINDOW_ARB         0x2001
#define WGL_DOUBLE_BUFFER_ARB          0x2011
#define WGL_SUPPORT_OPENGL_ARB         0x2010
#define WGL_PIXEL_TYPE_ARB             0x201
#define WGL_TYPE_RGBA_ARB              0x202B
#define WGL_COLOR_BITS_ARB             0x2014
#define WGL_RED_BITS_ARB               0x2015
#define WGL_GREEN_BITS_ARB             0x2017
#define WGL_BLUE_BITS_ARB              0x2019
#define WGL_DEPTH_BITS_ARB             0x2022
#define WGL_STENCIL_BITS_ARB           0x2023
#include <windows.h>
#include <GL/gl.h>

namespace viewwnd
{
	HWND    hWnd;
	HDC     hDC;
	HGLRC   hRC;
};

namespace glrender
{
	int win_width;
	int win_height;
}

typedef HGLRC(*wglprocCreateContextAttribsARB)(HDC, HGLRC, const int *);
typedef BOOL(*wglprocChoosePixelFormatARB)(HDC, const int *, const FLOAT *, UINT, int *, UINT *);

#define VIEWCLASS   "ViewWnd"
#define VIEWSTYLE   WS_VISIBLE|WS_POPUP|WS_MAXIMIZE

BOOL OpenGLWindowCreate()
{
	//srand((unsigned)time(NULL));


	return TRUE;
}

void OnOpenGLWindowDestroy()
{
	using namespace viewwnd;
	wglMakeCurrent(NULL, NULL);
	wglDeleteContext(hRC);
	ReleaseDC(hWnd, hDC);
	UnregisterClass(VIEWCLASS, GetModuleHandle(NULL));
	PostQuitMessage(0);
}

void reshape(int w, int h)
{
	//win_width = w;
	//win_height = h;
}

void display()
{
	using namespace viewwnd;
	using namespace glrender;

	glViewport(0, 0, 1280, 720);// win_width, win_heighth);

	glClearColor(0., 0., 0., 1.);
	glClearDepth(1.);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(45, 1.3333, 0.1, 100);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();

	SwapBuffers(hDC);
}


LRESULT CALLBACK ViewProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	using namespace viewwnd;
	switch (uMsg)
	{
	case WM_DESTROY:
		OnOpenGLWindowDestroy();
		break;
	case WM_PAINT:
		display();
		break;
	case WM_SIZE:
		reshape(LOWORD(lParam), HIWORD(lParam));
		break;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

unsigned MyOpenGLThread(void *params)
{
	HWND myHWnd = FindWindow(null, "AArcade: Source");
	OpenGLInfo_t* info = (OpenGLInfo_t*)params; // always use a struct!

	if (!info->close)
	{
		using namespace viewwnd;
		{
			WNDCLASS wc;
			memset(&wc, 0, sizeof(wc));
			wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
			wc.lpfnWndProc = ViewProc;
			wc.hInstance = GetModuleHandle(NULL);
			wc.lpszClassName = VIEWCLASS;
			RegisterClass(&wc);
		}

		/* Create a temporaray context to get address of wgl extensions. */
		CSysModule* myModule = Sys_LoadModule("opengl32.dll");// VarArgs("%s\\bin\\opengl.dll", engine->GetGameDirectory()));
		if (myModule)
			DevMsg("Loaded openGL dll\n");
		else
			DevMsg("Failed to load openGL dll\n");

		//HINSTANCE hInstance = reinterpret_cast<HINSTANCE>(myModule);
		HINSTANCE hInstance = (HINSTANCE)GetModuleHandle(NULL);
		HWND hTempWnd = CreateWindowEx(WS_EX_APPWINDOW,
			VIEWCLASS,
			"Simple",
			VIEWSTYLE,
			0, 0, 0, 0,
			myHWnd, NULL,
			hInstance,
			NULL);
		if (!hTempWnd)
			DevMsg("failed A\n");
	//		return;

		HDC hTempDC = GetDC(hTempWnd);
		if (!hTempDC) {
			DestroyWindow(hTempWnd);
			DevMsg("failed B\n");
			//return;
		}

		PIXELFORMATDESCRIPTOR pfd;
		memset(&pfd, 0, sizeof(pfd));
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 24;
		pfd.iPixelType = PFD_TYPE_RGBA;
		pfd.iLayerType = PFD_MAIN_PLANE;

		HGLRC hTempRC;

		int iPF;
		if ((!(iPF = ChoosePixelFormat(hTempDC, &pfd)) || !SetPixelFormat(hTempDC, iPF, &pfd)) ||

			(!(hTempRC = wglCreateContext(hTempDC)) || !wglMakeCurrent(hTempDC, hTempRC))) {
			ReleaseDC(hTempWnd, hTempDC);	// modified by me
			DestroyWindow(hTempWnd);
			DevMsg("failed C\n");
//			return;
		}
		DevMsg("here A\n");
		/* Like all extensions in Win32, the function pointers returned by wglGetProcAddress are tied
		* to the render context they were obtained with. Since this is a temporary context, we
		* place those function pointers in automatic storage of the window and context creation function. */
		wglprocCreateContextAttribsARB wglCreateContextAttribsARB = (wglprocCreateContextAttribsARB)wglGetProcAddress("wglCreateContextAttribsARB");
		wglprocChoosePixelFormatARB wglChoosePixelFormatARB = (wglprocChoosePixelFormatARB)wglGetProcAddress("wglChoosePixelFormatARB");

		if (wglChoosePixelFormatARB && wglCreateContextAttribsARB) {
			/* good we have access to extended pixelformat and context attributes */
			hWnd = CreateWindowEx(WS_EX_APPWINDOW,
				VIEWCLASS,
				"Simple",
				VIEWSTYLE,
				0, 0, 0, 0,
				myHWnd, NULL,
				hInstance,
				NULL);

			if (!hWnd) {
				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(hTempRC);
				ReleaseDC(hTempWnd, hTempDC);	// modified by me
				DestroyWindow(hTempWnd);

				DevMsg("failed D\n");
//				return;
			}

			hDC = GetDC(hWnd);
			if (!hDC) {
				DestroyWindow(hWnd);

				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(hTempRC);
				ReleaseDC(hTempWnd, hTempDC);	// modified by me
				DestroyWindow(hTempWnd);

				DevMsg("failed E\n");
			//	return;
			}

			int attribs[] = {
				WGL_DRAW_TO_WINDOW_ARB, TRUE,
				WGL_DOUBLE_BUFFER_ARB, TRUE,
				WGL_SUPPORT_OPENGL_ARB, TRUE,
				WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
				WGL_COLOR_BITS_ARB, 24,
				WGL_RED_BITS_ARB, 8,
				WGL_GREEN_BITS_ARB, 8,
				WGL_BLUE_BITS_ARB, 8,
				WGL_DEPTH_BITS_ARB, 24,
				WGL_STENCIL_BITS_ARB, 8,
				0, 0
			};
			UINT num_formats_choosen;
			BOOL choose_pf_success = wglChoosePixelFormatARB(
				hDC,
				attribs,
				NULL,
				1,
				&iPF,
				&num_formats_choosen);

			/* now this is a kludge; we need to pass something in the PIXELFORMATDESCRIPTOR
			* to SetPixelFormat; it will be ignored, mostly. OTOH we want to send something
			* sane, we're nice people after all - it doesn't hurt if this fails. */
			DescribePixelFormat(hDC, iPF, sizeof(pfd), &pfd);
			ShowWindow(hWnd, SW_SHOW);
			UpdateWindow(hWnd);
			if (!(choose_pf_success &&
				num_formats_choosen >= 1 &&
				SetPixelFormat(hDC, iPF, &pfd))) {
				ReleaseDC(hTempWnd, hDC);	// modified by me
				DestroyWindow(hWnd);

				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(hTempRC);
				ReleaseDC(hTempWnd, hTempDC);	// modified by me
				DestroyWindow(hTempWnd);

				DevMsg("failed F\n");
//				return;
			}

			/* we request a OpenGL-3 compatibility profile */
			int context_attribs[] = {
				0x2091, 3,//WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
				0x2092, 0,//WGL_CONTEXT_MINOR_VERSION_ARB, 0,
				0x9126, 0x00000001 | 0x00000002,//WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB | WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB,
				0, 0
			};
			hRC = wglCreateContextAttribsARB(hDC, NULL, context_attribs);
			if (!hRC) {
				ReleaseDC(hTempWnd, hDC);	// modified by me
				DestroyWindow(hWnd);

				wglMakeCurrent(NULL, NULL);
				wglDeleteContext(hTempRC);
				ReleaseDC(hTempWnd, hTempDC);	// modified by me
				DestroyWindow(hTempWnd);

				DevMsg("failed G\n");
//				return;
			}
			wglMakeCurrent(hDC, hRC);

			/* now that we've created the proper window, DC and RC
			* we can delete the temporaries */
			wglMakeCurrent(NULL, NULL);
			wglDeleteContext(hTempRC);
			ReleaseDC(hTempWnd, hTempDC);	// modified by me
			DestroyWindow(hTempWnd);

		}
		else {
			/* extended pixelformats and context attributes not supported
			* => use temporary window and context as the proper ones */
			hWnd = hTempWnd;
			hDC = hTempDC;
			hRC = hTempRC;
		}

		ShowWindow(hWnd, SW_SHOW);
		UpdateWindow(hWnd);

		if (true)
		{
			while (!info->close) //&& glfwGetKey(window, GLFW_KEY_ESCAPE) != GLFW_PRESS
			{
				// idle
			}
		}
		else if(glfwInit())
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
				if (glewInit() != GLEW_OK)
					DevMsg("Failed to initialize GLEW\n");

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