#ifndef __SDL_Surface_H__
#define __SDL_Surface_H__
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <string>

class Surface
{
private:
	SDL_Surface*_surface;
public:
	Surface();
	virtual~Surface();
	//调用这个函数后，就把内存交给了引擎处理，就不需要主动释放，若主动释放，会出错
	static Surface*create(SDL_Surface*surface);
	static Surface*create(const std::string&filepath);
	static Surface*createWithBMP(const std::string&filepath);
	static Surface*create(Uint32 flags,int width,int height,int depth,Uint32 Rmask,Uint32 Gmask,Uint32 Bmask,Uint32 Amask);
	
	bool init(SDL_Surface*sur);
	bool init(const std::string&filepath);
	bool initWithBMP(const std::string&filepath);
	bool init(Uint32 flags,int width,int height,int depth,Uint32 Rmask,Uint32 Gmask,Uint32 Bmask,Uint32 Amask);

	//画 可切割，但不能缩放
	int blitSurface(const SDL_Rect*srcRect,Surface*dest,SDL_Rect*destRect);
	//SDL_ConvertSurface(SDL_Surface*src,const SDL_PixelFormat* fmt,Uint32flags)
	Surface*convertSurface(const SDL_PixelFormat*fmt,Uint32 flags);

	//画图函数
	int fillRect(const SDL_Rect*rect,Uint32 color);
	//Renderer是友元
	//设置关键色并返回关键色
	Uint32 mapRGB(Uint8 r,Uint8 g,Uint8 b)const;
	//设置关键色并返回关键色
	Uint32 mapRGBA(Uint8 r,Uint8 g,Uint8 b,Uint8 a)const;
	//返回经过关键色处理后的Surface，与前面那个相同
	//对png图片格式尤其有效
	int setColorKey(Uint32 colorkey)const;
	//获得对应位置像素的ARGB值
	Uint32 getARGB(int x,int y)const;
	//混合模式
	int setSurfaceBlendMode(SDL_BlendMode blendMode);
	int getSurfaceBlendMode(SDL_BlendMode*blendMode);

	SDL_Surface*getSDL_Surface()const;
};
#endif