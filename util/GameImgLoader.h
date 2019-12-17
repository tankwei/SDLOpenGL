#ifndef GAMEIMGLOADER_H
#define GAMEIMGLOADER_H

class GameImgLoader
{
public:
	GameImgLoader();
	~GameImgLoader() {}
	void loadTextureData(unsigned int *texture, const char *filename);
};

#endif // !GAMEIMGLOADER_H

