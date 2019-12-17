#ifndef RENDERER_H
#define RENDERER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include "Rect.h"
#include <game/Texture.h>

class Window;
class Surface;
class SDLTexture;

class Renderer
{
public:
	Renderer();
	~Renderer();

	static Renderer* create(Window* window, int index, Uint32 flags);
	//初始化
	bool init(Window* window, int index, Uint32 flags);
	//清空渲染器
	int renderClear();
	//呈现
	void renderPresent();
	//创建Texture
	SDLTexture* createTexture(Uint32 format, int access, int w, int h);
	SDLTexture* createTextureFromSurface(Surface* surface);
	//绘制
	int renderCopy(SDLTexture* texture, const SDL_Rect* srcRect, SDL_Rect* destRect);
	int renderCopyEx(SDLTexture* texture, const SDL_Rect* srcRect, SDL_Rect* destRect, const double angle, const SDL_Point* center, SDL_RendererFlip flip);
	int renderCopyEx(SDLTexture* texture, const Rect& srcRect, const Rect& destRect, const double angle, const Point& center, SDL_RendererFlip flip);
	//创建Texture
	Texture* imgLoadTexture(const char* filePath);
	SDLTexture* imgLoadTextureThreadSafe(const char* filepath);
	SDLTexture* createTextureFromSurfaceThreadSafe(Surface* surface);

	int renderFillRect(const Rect& rect);
	//进行与渲染无关的分辨率
	int renderSetScale(float scaleX, float scaleY);
	//Size renderGetScale()const;
	//进行逻辑缩放
	int renderSetLogicalSize(int w, int h);
	void renderGetLogicalSize(int& w, int& h);
	//设置填充模式
	int setRenderDrawBlendMode(SDL_BlendMode mode);
	int getRenderDrawBlendMode(SDL_BlendMode* mode);
	//渲染区域
	int renderSetViewport(const SDL_Rect* rect);
	//这里，SDL返回的是void，所以API做了相应改变
	Rect renderGetViewport();
	/*改变渲染目标，为空时则改为默认渲染目标
	*@ texture 必须有SDL_TEXTUREACCESS_TARGET*/
	int setRenderTarget(SDLTexture* texture);
	/*设置遮挡*/
	int renderSetClipRect(const SDL_Rect* rect);
	Rect renderGetClipRect();
	SDL_Renderer* getSDL_Renderer()const;
private:
	SDL_Renderer* _renderer;
};

#endif