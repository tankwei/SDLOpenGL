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
	//����
	int updateWindowSurface();
	void SwapBuffers();
	void setBgClear(float r, float g, float b, float a);
	//��ô��ڵ�surface
	Surface*getWindowSurface();
	//��ʾ����
	void showWindow();
	//���ش���
	void hideWindow();
	//��ô��ڴ�С
	Size getWindowSize()const;
	//��ô�����ʵ��С
	Size getRealWindowSize()const;
	//���´���λ�ò�����
	SDL_Point getWindowPosition();
	SDL_Window* getSDL_WindowPtr()const;
private:
	SDL_Window*_window;
	//��������� w h
	SDL_Point _position;
	int _width;
	int _height;
	SDL_GLContext main_glContext;
};

#endif