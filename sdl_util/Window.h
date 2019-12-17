#ifndef __Window_H__
#define __Window_H__
#include <SDL2/SDL.h>

class Surface;
class Size;

class Window
{
public:
	Window();
	~Window();
	static Window*create(const char*title, int x, int y, int width, int height, Uint32 flags);
	bool InitWindow(const char*title, int x, int y, int width, int height, Uint32 flags);
	//更新
	int updateWindowSurface();
	void SwapBuffers();
	void setBgClear(float r, float g, float b, float a);
	//获得窗口的surface
	Surface*getWindowSurface();
	//显示窗口
	void showWindow();
	//隐藏窗口
	void hideWindow();
	//获得窗口大小
	Size getWindowSize()const;
	//获得窗口真实大小
	Size getRealWindowSize()const;
	//更新窗口位置并返回
	SDL_Point getWindowPosition();
	SDL_Window* getSDL_WindowPtr()const;
private:
	SDL_Window*_window;
	//窗口坐标和 w h
	SDL_Point _position;
	int _width;
	int _height;
	SDL_GLContext main_glContext;
};

#endif