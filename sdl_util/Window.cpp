#include <iostream>
#include "sdl_util/Window.h"
#include "sdl_util/Surface.h"
#include "sdl_util/Size.h"
#include <glad/glad.h>

Window::Window()
{
	this->_window = NULL;
	this->main_glContext = NULL;
}

Window::~Window()
{
	SDL_GL_DeleteContext(this->main_glContext);
	this->main_glContext = nullptr;
	SDL_DestroyWindow(this->_window);
	this->_window = nullptr;
	SDL_Quit();
}

bool Window::InitWindow(const char*title, int x, int y, int width, int height, Uint32 flags)
{
	SDL_Init(SDL_INIT_EVERYTHING);
	//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
		//SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
		////SDL_GL_SetAttribute(SDL_GL_MULTISAMPLEBUFFERS, 1);
		////SDL_GL_SetAttribute(SDL_GL_MULTISAMPLESAMPLES, 4);
		//SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);	// Number of bits used to display shades of red
		//SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
		//SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);	// Sum of bits above. Space of a pixel
		//SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);	// Enable double buffer
		//SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 2);
	SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
	SDL_GL_SetAttribute(SDL_GL_BUFFER_SIZE, 32);
	SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
	SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
	this->_window = SDL_CreateWindow(title, x, y, width, height, flags);
	 if (NULL == this->_window) {
        // 如果窗体没有创建成功，后面的程序已经没有执行的必要了。
        printf("Could not create window: %s\n", SDL_GetError());
        return 1;
    }
	//1、使用SDL_Init()方法对SDL进行初始化。其中该初始化方法的参数类型为int类型，可以从SDL_INIT_HAPTIC、SDL_INIT_AUDIO、SDL_INIT_VIDEO、SDL_INIT_TIMER、SDL_INIT_JOYSTICK、
		//SDL_INIT_EVERYTHING、SDL_INIT_NOPARACHUTE七个类型中选择，分别表示
		//力反馈子系统、音频子系统、视频子系统、计时器子系统、摇杆子系统、全部和忽略致命信号。
		//2、在SDL初始化成功后使用SDL_CreateWindow()方法创建一个SDL窗口（SDL_Window）。在这里我们可以设置窗口的名称、对齐方式、窗口宽度和窗口高度。
		//3、在SDL窗口创建成功后使用SDL_CreateRenderer()方法创建一个SDL渲染器（SDL_Renderer）。
		//其中SDL渲染器有SDL_RENDERER_SOFTWARE、SDL_RENDERER_ACCELERATED、SDL_RENDERER_PRESENTVSYNC、
		//SDL_RENDERER_TARGETTEXTURE四种类型分别表示软件渲染、硬件加速、屏幕同步刷新渲染和支持渲染纹理。
		//4、SDL_RenderClear()清空SDL渲染器,SDL_RenderPresent()将渲染的结果显示出来
	//initlize
	this->_position.x = x;
	this->_position.y = y;
	this->_width = width;
	this->_height = height;

	// Create an OpenGL context.
		//gl = sdl::GLContext(SDL_GL_CreateContext(windowPtr.get()));
	this->main_glContext = SDL_GL_CreateContext(this->_window);
	//https://www.jianshu.com/p/11e1afb3972f
	//创建一个简单的OpenGL窗口
	if (NULL == this->main_glContext)
	{
		return false;
	}
	if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress)) {
		std::cerr << "Failed to initialize GLAD" << std::endl;
		return false;
	}
	//g_textureID = -1;
	//glClearColor(1.0, 1.0, 1.0, 1.0);
	// Set default OpenGL state.
	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC1_ALPHA);
	//glEnable(GL_BLEND);
	//glEnable(GL_TEXTURE_2D);
	//glViewport(0, 0, SCR_WIDTH, SCR_HEIGHT);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_STENCIL_TEST);

	return true;
}

Window*Window::create(const char*title, int x, int y, int width, int height, Uint32 flags)
{
	Window* window = new Window();
	if (window && window->InitWindow(title, x, y, width, height, flags)) {
		//window->autorelease();
	}
	else
	{
		delete window;
		window = NULL;
	}

	return window;
}

int Window::updateWindowSurface()
{
	if (_window != nullptr)
		return SDL_UpdateWindowSurface(_window);
	return -1;
}

void Window::SwapBuffers()
{
	// Swap buffers.
	SDL_GL_SwapWindow(this->_window);
}

void Window::setBgClear(float r, float g, float b, float a)
{
	glClearColor(r, g, b, a);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

Surface* Window::getWindowSurface()
{
	if (nullptr != this->_window)
		return Surface::create(SDL_GetWindowSurface(_window));
	return NULL;
}

void Window::showWindow()
{
	SDL_ShowWindow(_window);
}

void Window::hideWindow()
{
	SDL_HideWindow(_window);
}

Size Window::getWindowSize()const
{
	return Size(_width, _height);
}

Size Window::getRealWindowSize()const
{
	int w = 0;
	int h = 0;
	SDL_GetWindowSize(_window, &w, &h);

	return Size((float)w, (float)h);
}

SDL_Point Window::getWindowPosition()
{
	if (_window)
		SDL_GetWindowPosition(_window, &_position.x, &_position.y);
	return _position;
}

SDL_Window* Window::getSDL_WindowPtr()const
{
	return this->_window;
}