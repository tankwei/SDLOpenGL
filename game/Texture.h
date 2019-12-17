#ifndef TEXTURE_H
#define TEXTURE_H

#include <memory>
#include <mutex>
#include <string>
#include <SDL2/SDL.h>
//#include <GL/glew.h>
#include "glad/glad.h"
#include <sdl_util/Image.h>


class Texture
{
public:
	Texture() { m_texture = 0; width = 0; height = 0; pixels = NULL; }
	Texture(const std::string& fileName);
	Texture(SDL_Texture* pTex);
	static Texture& GetInstancePtr(){
		static std::once_flag s_flag;
		std::call_once(s_flag,[&](){
			instancePtr.reset(new Texture());
		});
		return *instancePtr;
	}
	GLuint GenerateUnspecifiedUserID();
	GLuint LoadGLTexture2D(GLuint nUndefiedUserID, const wchar_t* texturePathStr, GLint nTexWrapMode);
	//Texture(const Texture& rhs);
	void Bind(unsigned int uint);
	void Unbind();

	Texture& operator=(const Texture&);

	virtual ~Texture();

	static Texture* create(SDL_Texture* pTex, Image &imgData, int width, int height,int nrChannels);
	//获得贴图的属性
	void queryTexture(Uint32* format, int* access, int* w, int* h);
	//设置追加颜色 srcColor = srcColor*(color/255)	//获得追加颜色
	/*即 如果原来颜色为(255,255,255) 设置为(255,128,255)  结果为(255,128,255)*/
	int setTextureColorMod(Uint8 r, Uint8 g, Uint8 b)const;
	int getTextureColorMod(Uint8* r, Uint8* g, Uint8* b)const;
	//透明度
	int setTextureAlphaMod(Uint8 alpha)const;
	int getTextureAlphaMod(Uint8* alpha)const;
	//设置混合,主要用于SDL_RenderCopy
	/*SDL_BLENDMODE_NONE SDL_BLENDMODE_BLEND SDL_BLEND_ADD SDL_BLEND_ADD*/
	int setTextureBlendMode(SDL_BlendMode blendMode)const;
	int getTextureBlendMode(SDL_BlendMode* blendMode)const;
	//获取SDL_Texture
	SDL_Texture* getSDL_Texture()const;
private:
	bool ReciveTexture(SDL_Texture* pTex);
private:
    static std::unique_ptr<Texture> instancePtr;
	/* Handle to the texture data. */
	GLuint m_texture;
	SDL_Texture* _texture;
	unsigned int width;
	unsigned int height;

	const void* pixels;
	const int SCALE_FACTOR = 1;
};

#endif