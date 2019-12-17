#ifndef APP_H
#define APP_H

#include <game/game.h>
#include "engine/engine.h"
#include "sdl_util/sdl_util.h"
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <png/png.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
//#include <wtypes.h>
#include <fstream>
#include <vector>
#include <string>
#include <iostream>

class Image;
class Renderer;
class Window;
class SDLTexture;

struct SDL_WindowEvent;
union SDL_Event;

namespace editor
{
	const unsigned int SCR_WIDTH = 1280;
	const unsigned int SCR_HEIGHT = 720;

	class App
	{
	public:
		App();
		~App();
		bool init();
		void drawByPixels();
		bool run_frame();
		void prepareGeo(const char *base_path);
		void loadTexture(const char *fileStr);
		void drawTriangle(glm::mat4 projection, glm::mat4 view);
		void drawGeo(glm::mat4 projection, glm::mat4 view);
		void drawTeapot(float angle, glm::mat4 view);
		SDL_Surface * LoadIMG(const char *filename);
		Uint32 getpixel(SDL_Surface *surface, int x, int y);
	private:
		bool handle_events();
		bool handle_event(const SDL_Event &e);
		void handle_drop(const char *path);
		void handle_window_event(const SDL_WindowEvent &e);
		void handle_mouse_event(const SDL_MouseMotionEvent &motion);
		void handle_scroll_event(const SDL_MouseWheelEvent &event);
		void processInput(const SDL_KeyboardEvent &event);
		void processMouseButton(const SDL_MouseButtonEvent &event);
		void draw();
		float rotate_degree;
		
		float PillarVerticesArray[168];
		float PillarVerticesArray1[168];
		float SolidConeVerticesArray[168];

		float lastX;
		float lastY;
		bool firstMouse;

		time_t init_now_time;

		//sdl::SdlContext sdl_content;
		sdl::WindowPtr windowPtr;
		Window *appWindow;
		//sdl::GLContext gl;
		SDL_GLContext mainContext;
		engine::Engine engine;
		GLuint g_textureID;
		SDL_Renderer* render;
		Renderer *appRenderer;
		SDLTexture *appTexture;
		SDL_Texture *texture;
		SDL_Rect sdlRect;
		char *data_path;
		
		game::GamePtr     game_ptr;
	private:
		float quit;
		//void testpng(std::string filePath);
		size_t loadpng(const char* filePath, int* pnWidth, int* pnHeight, unsigned char** cbData);
		void loadPngFile(const char *filename, Image &pngData);
		void OutputDebugPrintf(const char* strOutputString, ...)
		{
#ifdef _DEBUG 
			char strBuffer[4096] = { 0 };
			va_list vlArgs;
			va_start(vlArgs, strOutputString);
			_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
			//vsprintf(strBuffer,strOutputString,vlArgs);
			va_end(vlArgs);
			int slength = sizeof(strBuffer) + 1;
			int unicodeLen = MultiByteToWideChar(CP_ACP, 0, (const char *)strBuffer, -1, NULL, 0);
			std::wstring w_str(unicodeLen, '\0');
			MultiByteToWideChar(CP_ACP, 0, (const char *)strBuffer, -1, (LPWSTR)&w_str[0], unicodeLen);
			OutputDebugString(w_str.c_str());//CA2W(strBuffer)
#endif
		}
	};
}

#endif