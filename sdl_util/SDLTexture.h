#ifndef SDLTEXTURE_H
#define SDLTEXTURE_H
#include <SDL2/SDL.h>
#include <glad/glad.h>

class SDLTexture
{
public:
	SDLTexture();
	SDLTexture(SDL_Texture* pTex);
	virtual~SDLTexture();

	//Texture(const Texture& rhs);
	void Bind(unsigned int uint);
	void Unbind();

	SDLTexture& operator=(const SDLTexture&);

	static SDLTexture* create(SDL_Texture* pTex);
	//�����ͼ������
	void queryTexture(Uint32* format, int* access, int* w, int* h);
	//����׷����ɫ srcColor = srcColor*(color/255)	//���׷����ɫ
	/*�� ���ԭ����ɫΪ(255,255,255) ����Ϊ(255,128,255)  ���Ϊ(255,128,255)*/
	int setTextureColorMod(Uint8 r, Uint8 g, Uint8 b)const;
	int getTextureColorMod(Uint8* r, Uint8* g, Uint8* b)const;
	//͸����
	int setTextureAlphaMod(Uint8 alpha)const;
	int getTextureAlphaMod(Uint8* alpha)const;
	//���û��,��Ҫ����SDL_RenderCopy
	/*SDL_BLENDMODE_NONE SDL_BLENDMODE_BLEND SDL_BLEND_ADD SDL_BLEND_ADD*/
	int setTextureBlendMode(SDL_BlendMode blendMode)const;
	int getTextureBlendMode(SDL_BlendMode* blendMode)const;
	//��ȡSDL_Texture
	SDL_Texture* getSDL_Texture()const;
private:
	bool ReciveTexture(SDL_Texture* pTex);
private:
	/* Handle to the texture data. */
	GLuint m_texture;
	SDL_Texture* _texture;
	unsigned int width;
	unsigned int height;

	const void* pixels;
	const int SCALE_FACTOR = 1;
};
#endif