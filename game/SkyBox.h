#ifndef SKYBOX_H
#define SKYBOX_H

#include <util/camera.h>
#include <util/shader.h>

class SkyBox
{
public:
	SkyBox();
	~SkyBox();
public:
	void Init();
	void Render(Camera &camera, glm::mat4 &projection);
private:
	unsigned int loadCubemap(std::vector<std::string> &faces);
private:
	GLuint skyboxVAO, skyboxVBO;
	GLuint cubemapTexture;
	Shader *skyboxShader;
};


#endif // !SKYBOX_H

