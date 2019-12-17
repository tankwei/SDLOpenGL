#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <iostream>
#include <stdlib.h>
#include <sdl_util/Image.h>
#include <util/stb_image.h>
#include "Texture.h"
#include <cassert>

Texture::Texture(const std::string & fileName)
{
	SDL_Surface* texture = NULL;
	texture = IMG_Load(fileName.c_str());
	if (NULL == texture) {
		std::cerr << "Unable to load texture image!" << std::endl;
		// 加载并生成纹理
		int w, h, nrChannels;
		stbi_set_flip_vertically_on_load(true);
		unsigned char *data = stbi_load(fileName.c_str(), &w, &h, &nrChannels, 0);
		if (data) {
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);//GL_BGR
		}
		this->width = w;
		this->height = h;
		int file_size = width * height * nrChannels;
		unsigned char *pData = new unsigned char[file_size];
		memcpy((void *)pData, data, file_size);
		this->pixels = pData;
		stbi_image_free(data);
		data = NULL;
	}
	else {
		//Uint32 fmt = texture->format->format;
		SDL_Surface* formattedTexture = SDL_ConvertSurfaceFormat(texture, SDL_PIXELFORMAT_RGB24, 0);

		this->width = formattedTexture->w;
		this->height = formattedTexture->h;
		this->pixels = formattedTexture->pixels;
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, this->width, this->height, 0, GL_RGB, GL_UNSIGNED_BYTE, this->pixels);
		SDL_FreeSurface(texture);
		SDL_FreeSurface(formattedTexture);
	}
	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);
	//glTexEnvi(GL_TEXTURE_ENV, GL_REPLACE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
}

/* copy Ctor should make a deep copy, so also copying the pixel data into a new buffer.
   TODO: Implement that later if I have time.
   */
   //Texture::Texture(const Texture& rhs) {
   //	m_texture = rhs.m_texture;
   //}

Texture::~Texture()
{
	glDeleteTextures(1, &m_texture);
}

Texture::Texture(SDL_Texture* pTex)
{
	this->ReciveTexture(pTex);
}

GLuint Texture::GenerateUnspecifiedUserID(){
	return 0;
}

GLuint Texture::LoadGLTexture2D(GLuint nUndefiedUserID, const wchar_t* texturePathStr, GLint nTexWrapMode){
	GLenum format = GL_RGB;
	int w = 0, h = 0, nrChannels = 1;
	stbi_set_flip_vertically_on_load(true);
	size_t len = wcslen(texturePathStr) + 1;
	char *cstr = (char *)malloc(len * sizeof(char));
	memset(cstr,0,len);
	int n = wcstombs(cstr,texturePathStr,len);
	unsigned char *data = stbi_load(cstr, &w, &h, &nrChannels, 0);
	if (data) {
		if (1 == nrChannels){
			format = GL_RED;//GL_BGR
		}
		else if (nrChannels == 3){
			format = GL_RGB;
		}
		else if (nrChannels == 4) {
			format = GL_RGBA;
		}
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);
	}
	this->width = w;
	this->height = h;
	int file_size = width * height * nrChannels;
	unsigned char *pData = new unsigned char[file_size];
	memcpy((void *)pData, data, file_size);
	this->pixels = pData;
	stbi_image_free(data);
	data = NULL;
	glGenTextures(1, &this->m_texture);
	glBindTexture(GL_TEXTURE_2D, this->m_texture);
	//glTexEnvi(GL_TEXTURE_ENV, GL_REPLACE, GL_REPLACE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, nTexWrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, nTexWrapMode);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glBindTexture(GL_TEXTURE_2D, 0);
	return this->m_texture;
}

Texture* Texture::create(SDL_Texture* pTex, Image &imgData, int width, int height,int nrChannels)
{
	GLenum format = GL_RGB;
	if (1 == nrChannels){
		format = GL_RED;
	}
	else if (nrChannels == 3){
		format = GL_RGB;
	}
	else if (nrChannels == 4) {
		format = GL_RGBA;
	}
	Texture* texture = new Texture(pTex);
	texture->width = width;
	texture->height = height;
	int file_size = width * height * nrChannels;
	unsigned char *pData = new unsigned char[file_size];
	memcpy((void *)pData, imgData.imgData, file_size);
	texture->pixels = pData;
	glGenTextures(1, &texture->m_texture);
	glBindTexture(GL_TEXTURE_2D, texture->m_texture);

	//glTexEnvi(GL_TEXTURE_ENV, GL_REPLACE, GL_REPLACE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, texture->pixels);

	glBindTexture(GL_TEXTURE_2D, 0);
	//texture->autorelease();
	return texture;
}

void Texture::Bind(unsigned int unit)
{
	//assert(unit >= 0 && unit <= 31); 
	glActiveTexture(GL_TEXTURE0 + unit);
	glBindTexture(GL_TEXTURE_2D, m_texture);
}

void Texture::Unbind() {
	glDisable(GL_TEXTURE_2D);
}

/* OK, this is a delicate topic, because assignment in this case will
 NOT copy the pixel data in memory rather than just copy the pointer
 to it (pixels). I just want it that way.*/
Texture& Texture::operator=(const Texture& rhs)
{
	width = rhs.width;
	height = rhs.height;

	glGenTextures(1, &m_texture);
	glBindTexture(GL_TEXTURE_2D, m_texture);

	//glTexEnvi(GL_TEXTURE_ENV, GL_REPLACE, GL_REPLACE);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, rhs.pixels);


	glBindTexture(GL_TEXTURE_2D, 0);

	return *this;
}

void Texture::queryTexture(Uint32* format, int* access, int* w, int* h)
{
	if (_texture != nullptr)
		SDL_QueryTexture(_texture, format, access, w, h);
}

int Texture::setTextureColorMod(Uint8 r, Uint8 g, Uint8 b)const
{
	if (_texture)
		return SDL_SetTextureColorMod(_texture, r, g, b);
	else
		return -1;
}
int Texture::getTextureColorMod(Uint8* r, Uint8* g, Uint8* b)const
{
	if (_texture)
		return SDL_GetTextureColorMod(_texture, r, g, b);
	else
		return -1;
}

int Texture::setTextureAlphaMod(Uint8 alpha)const
{
	if (_texture != nullptr)
		return SDL_SetTextureAlphaMod(_texture, alpha);
	else
		return -1;
}

int Texture::getTextureAlphaMod(Uint8* alpha)const
{
	if (_texture)
		return SDL_GetTextureAlphaMod(_texture, alpha);
	else
		return -1;
}

int Texture::setTextureBlendMode(SDL_BlendMode blendMode)const
{
	if (_texture)
		return SDL_SetTextureBlendMode(_texture, blendMode);
	else
		return -1;
}

int Texture::getTextureBlendMode(SDL_BlendMode* blendMode)const
{
	if (_texture)
		return SDL_GetTextureBlendMode(_texture, blendMode);
	else
		return -1;
}

SDL_Texture* Texture::getSDL_Texture()const
{
	return _texture;
}

bool Texture::ReciveTexture(SDL_Texture* pTex)
{
	if (pTex != nullptr){
		this->_texture = pTex;
		return true;
	}
	return false;
}

std::unique_ptr<Texture> Texture::instancePtr;