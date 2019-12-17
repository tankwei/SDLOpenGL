#include "SDLTexture.h"

SDLTexture::SDLTexture()
{
	this->_texture = NULL;
}

SDLTexture::SDLTexture(SDL_Texture* pTex)
{
	this->ReciveTexture(pTex);
}

SDLTexture::~SDLTexture()
{
	if (this->_texture)
		SDL_DestroyTexture(this->_texture);
}

void SDLTexture::Bind(unsigned int unit)
{
	//assert(unit >= 0 && unit <= 31); 
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, this->m_texture);
}

void SDLTexture::Unbind() {
	glDisable(GL_TEXTURE_2D);
}

SDLTexture* SDLTexture::create(SDL_Texture* pTex)
{
	SDLTexture* texture = new SDLTexture(pTex);
	//texture->autorelease();
	return texture;
}

/* OK, this is a delicate topic, because assignment in this case will
 NOT copy the pixel data in memory rather than just copy the pointer
 to it (pixels). I just want it that way.*/
SDLTexture& SDLTexture::operator=(const SDLTexture& rhs)
{
	this->width = rhs.width;
	this->height = rhs.height;

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	glTexEnvi(GL_TEXTURE_ENV, GL_REPLACE, GL_REPLACE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rhs.pixels);


	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}

void SDLTexture::queryTexture(Uint32* format, int* access, int* w, int* h)
{
	if (_texture != nullptr){
		SDL_QueryTexture(_texture, format, access, w, h);
	}
}

int SDLTexture::setTextureColorMod(Uint8 r, Uint8 g, Uint8 b)const
{
	if (_texture){
		return SDL_SetTextureColorMod(_texture, r, g, b);
	}
	else{
		return -1;
	}
}
int SDLTexture::getTextureColorMod(Uint8* r, Uint8* g, Uint8* b)const
{
	if (_texture){
		return SDL_GetTextureColorMod(_texture, r, g, b);
	}
	else {
		return -1;
	}
}

int SDLTexture::setTextureAlphaMod(Uint8 alpha)const
{
	if (_texture != nullptr)
		return SDL_SetTextureAlphaMod(_texture, alpha);
	else
		return -1;
}

int SDLTexture::getTextureAlphaMod(Uint8* alpha)const
{
	if (_texture)
		return SDL_GetTextureAlphaMod(_texture, alpha);
	else
		return -1;
}

int SDLTexture::setTextureBlendMode(SDL_BlendMode blendMode)const
{
	if (_texture)
		return SDL_SetTextureBlendMode(_texture, blendMode);
	else
		return -1;
}

int SDLTexture::getTextureBlendMode(SDL_BlendMode* blendMode)const
{
	if (_texture)
		return SDL_GetTextureBlendMode(_texture, blendMode);
	else
		return -1;
}

SDL_Texture* SDLTexture::getSDL_Texture()const
{
	return _texture;
}

bool SDLTexture::ReciveTexture(SDL_Texture* pTex)
{
	if (pTex != nullptr){
		this->_texture = pTex;
		return true;
	}
	return false;
}
