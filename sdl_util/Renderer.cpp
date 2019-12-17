#include "sdl_util/Renderer.h"
#include "sdl_util/Window.h"
#include "sdl_util/Surface.h"
#include <sdl_util/SDLTexture.h>
#include <sdl_util/Image.h>
#include <util/stb_image.h>

const int SCALE_FACTOR = 1;

Renderer::Renderer()
	:_renderer(nullptr)
{

}

Renderer::~Renderer()
{
	if (_renderer)
		SDL_DestroyRenderer(_renderer);

	_renderer = nullptr;
}

Renderer*Renderer::create(Window* window, int index, Uint32 flags)
{
	Renderer* renderer = new Renderer();
	if (renderer && renderer->init(window, index, flags))
	{
		//renderer->autorelease();
	}
	else {
		delete renderer;
		renderer = NULL;
	}
	return renderer;
}

bool Renderer::init(Window* window, int index, Uint32 flags)
{
	//以win作为参数创建renderer
	this->_renderer = SDL_CreateRenderer(window->getSDL_WindowPtr(), index, flags);
	return true;
}

int Renderer::renderClear()
{
	return SDL_RenderClear(_renderer);
}

void Renderer::renderPresent()
{
	SDL_RenderPresent(_renderer);
}

SDLTexture* Renderer::createTexture(Uint32 format, int access, int w, int h)
{
	if (NULL != this->_renderer)
		return SDLTexture::create(SDL_CreateTexture(_renderer, format, access, w, h));
	return nullptr;
}

SDLTexture* Renderer::createTextureFromSurface(Surface* surface)
{
	SDLTexture* texture = nullptr;
	if (_renderer != nullptr && surface != nullptr)
	{
		texture = SDLTexture::create(SDL_CreateTextureFromSurface(_renderer, surface->getSDL_Surface()));
	}
	return texture;
}

int Renderer::renderCopy(SDLTexture*texture, const SDL_Rect*srcRect, SDL_Rect*destRect)
{
	return SDL_RenderCopy(_renderer, texture->getSDL_Texture(), srcRect, destRect);
}

int Renderer::renderCopyEx(SDLTexture* texture, const SDL_Rect* srcRect, SDL_Rect* destRect, const double angle, const SDL_Point* center, SDL_RendererFlip flip)
{
	return SDL_RenderCopyEx(_renderer, texture->getSDL_Texture(), srcRect, destRect, angle, center, flip);
}

int Renderer::renderCopyEx(SDLTexture* texture, const Rect& srcRect, const Rect& destRect, const double angle, const Point& center, SDL_RendererFlip flip)
{
	SDL_Rect src = srcRect.getSDL_Rect();
	SDL_Rect dest = destRect.getSDL_Rect();
	SDL_Point c = center.getSDL_Point();

	return SDL_RenderCopyEx(_renderer, texture->getSDL_Texture(), &src, &dest, angle, &c, flip);
}

Texture* Renderer::imgLoadTexture(const char* filePath)
{
	SDL_Texture* new_texture = NULL;
	//SDL_Surface *image, *temp = 0;
		//cout << "  * load image = " << filename << flush << endl;
		//std::string filePath = std::string(this->data_path) + "resources" + "\\" + filename;

	Image pngData;
	pngData.nWidth = 0;
	pngData.nHeight = 0;
	pngData.imgData = nullptr;
	//format = GL_RGBA;
	//stbi_image_free(data);
	//data = NULL;
	//loadpng("res/mingyuexin.png",&pngData.nWidth,&pngData.nHeight,&pngData.imgData);

	//this->loadPngFile(filePath.c_str(), pngData);
	//this->loadpng(filename, &pngData.nWidth, &pngData.nHeight, &pngData.imgData);
	pngData.loadImage(filePath);
	int textureWidth = pngData.nWidth * SCALE_FACTOR;
	int textureHeight = pngData.nHeight * SCALE_FACTOR;
	int rr = (Uint8)pngData.getRAt(0, 0);
	int gg = (Uint8)pngData.getGAt(0, 0);
	int bb = (Uint8)pngData.getBAt(0, 0);
	for (int row = 0; row < textureHeight; row++)
	{
		for (int col = 0; col < textureWidth; col++)
		{
			int x = col / SCALE_FACTOR;
			int y = row / SCALE_FACTOR;
			Uint8 r, g, b, a;
			//0 == row || row == (screenWidth - 1) || 0 == col || col == (screenHeight - 1)
			int rr1 = (Uint8)pngData.getRAt(x, y);
			int gg1 = (Uint8)pngData.getGAt(x, y);
			int bb1 = (Uint8)pngData.getBAt(x, y);
			if (rr1 == 255 && gg == 0 && bb == 255)
			{
				r = 0;
				g = 0;
				b = 0;
				a = 1;
				pngData.setColorAt(x, y, r, g, b, a);
			}
			else {
				r = (Uint8)pngData.getRAt(x, y);
				g = (Uint8)pngData.getGAt(x, y);
				b = (Uint8)pngData.getBAt(x, y);
				a = (Uint8)pngData.getAAt(x, y);
			}
		}
	}
	new_texture = SDL_CreateTexture(this->_renderer,
		SDL_PIXELFORMAT_RGBA32, SDL_TEXTUREACCESS_STREAMING,
		textureWidth, textureHeight);
	//https://wiki.libsdl.org/SDL_PixelFormatEnum
	//IMG_LoadTexture(this->render, filePath.c_str());
	SDL_SetTextureBlendMode(new_texture, SDL_BLENDMODE_BLEND);
	//unsigned char alpha;
	//SDL_GetTextureAlphaMod(new_texture, &alpha);
	//SDL_SetTextureAlphaMod(new_texture, alpha);
	SDL_UpdateTexture(new_texture, NULL, pngData.imgData, textureWidth * 4);
	//Uint32 magenta = this->getpixel(image, 0, 0);
	//SDL_SetColorKey(image, SDL_TRUE, magenta);// SDL_SRCCOLORKEY | SDL_RLEACCEL
	//IMG_LoadTexture(_renderer, filePath);
	return Texture::create(new_texture, pngData, textureWidth, textureHeight, 4);
}

SDLTexture*Renderer::imgLoadTextureThreadSafe(const char* filepath)
{
	SDL_Texture* tex = IMG_LoadTexture(_renderer, filepath);
	if (NULL == tex) {
		return nullptr;
	}
	SDLTexture *texture = new SDLTexture(tex);
	return texture;
}

SDLTexture*Renderer::createTextureFromSurfaceThreadSafe(Surface* surface)
{
	SDLTexture* texture = nullptr;
	if (_renderer != nullptr && surface != nullptr)
	{
		SDL_Texture*tex = SDL_CreateTextureFromSurface(_renderer, surface->getSDL_Surface());
		if (NULL == tex) {
			return nullptr;
		}
		texture = new SDLTexture(tex);
	}
	return texture;
}

int Renderer::renderFillRect(const Rect& rect)
{
	SDL_Rect r = rect.getSDL_Rect();
	return SDL_RenderFillRect(_renderer, &r);
}

int Renderer::renderSetScale(float scaleX, float scaleY)
{
	return SDL_RenderSetScale(_renderer, scaleX, scaleY);
}

int Renderer::renderSetLogicalSize(int w, int h)
{
	return SDL_RenderSetLogicalSize(_renderer, w, h);
}

void Renderer::renderGetLogicalSize(int& w, int& h)
{
	SDL_RenderGetLogicalSize(_renderer, &w, &h);
}

int Renderer::setRenderDrawBlendMode(SDL_BlendMode mode)
{
	return SDL_SetRenderDrawBlendMode(_renderer, mode);
}

int Renderer::getRenderDrawBlendMode(SDL_BlendMode* mode)
{
	return SDL_GetRenderDrawBlendMode(_renderer, mode);
}

int Renderer::renderSetViewport(const SDL_Rect* rect)
{
	return SDL_RenderSetViewport(_renderer, rect);
}

Rect Renderer::renderGetViewport()
{
	SDL_Rect rect;

	SDL_RenderGetViewport(_renderer, &rect);

	return Rect(rect);
}

int Renderer::setRenderTarget(SDLTexture* texture)
{
	if (texture != nullptr)
		return SDL_SetRenderTarget(_renderer, texture->getSDL_Texture());
	else
		return SDL_SetRenderTarget(_renderer, nullptr);
}

int Renderer::renderSetClipRect(const SDL_Rect*rect)
{
	return SDL_RenderSetClipRect(_renderer, rect);
}

Rect Renderer::renderGetClipRect()
{
	SDL_Rect rect;
	SDL_RenderGetClipRect(_renderer, &rect);

	return Rect(rect);
}

SDL_Renderer* Renderer::getSDL_Renderer()const
{
	return _renderer;
}