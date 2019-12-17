#include "SkyBox.h"
#include <vector>
#include <string>
#include <util/filesystem.h>
#include <util/stb_image.h>
#include <SDL2/SDL.h>

using namespace std;

float skyboxVertices[] = {
	// positions          
	-1.0f,  1.0f, -1.0f,
	-1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f, -1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	 1.0f, -1.0f, -1.0f,
	 1.0f, -1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f, -1.0f, -1.0f,

	-1.0f, -1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f, -1.0f,  1.0f,
	-1.0f, -1.0f,  1.0f,

	-1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f, -1.0f,
	 1.0f,  1.0f,  1.0f,
	 1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f,  1.0f,
	-1.0f,  1.0f, -1.0f
};

SkyBox::SkyBox()
{
	this->skyboxVAO = 0;
	this->skyboxVBO = 0;
}

SkyBox::~SkyBox() 
{

}

void SkyBox::Init()
{
	//char *prebase_path = SDL_GetPrefPath("My Company", "My Awesome SDL 2 Game");
	char *base_path =  SDL_GetBasePath();
	vector<std::string> cubeMaps_faces{
			FileSystem::getPath(base_path,"resources/skybox/right.jpg"),
			FileSystem::getPath(base_path, "resources/skybox/left.jpg"),
			FileSystem::getPath(base_path, "resources/skybox/top.jpg"),
			FileSystem::getPath(base_path,"resources/skybox/bottom.jpg"),
			FileSystem::getPath(base_path,"resources/skybox/front.jpg"),
			FileSystem::getPath(base_path,"resources/skybox/back.jpg")
	};

	this->skyboxShader = new Shader(FileSystem::getPath(base_path, "resources/shaderFiles/skyBoxVertexShaderSource.vs").c_str(), FileSystem::getPath(base_path, "resources/shaderFiles/skyBoxFragmentShaderSource.fs").c_str());
	this->skyboxShader->use();
	this->skyboxShader->setInt("skybox", 0);
	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);
	glBindVertexArray(skyboxVAO);
	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (GLvoid*)0);
	this->cubemapTexture = this->loadCubemap(cubeMaps_faces);
}

void SkyBox::Render(Camera &camera, glm::mat4 &projection)
{
	glDepthMask(GL_FALSE);
	// Draw skybox as last
	glDepthFunc(GL_LEQUAL);
	// Change depth function so depth test passes when values are equal to depth buffer's content
	this->skyboxShader->use();
	glm::mat4 skyview = glm::mat4(glm::mat3(camera.GetViewMatrix()));
	this->skyboxShader->setMat4("view", skyview);
	this->skyboxShader->setMat4("projection", projection);
	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
	glDepthFunc(GL_LESS);// Set depth function back to default
	glDepthMask(GL_TRUE);
}

unsigned int SkyBox::loadCubemap(std::vector<std::string> &faces)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);
	glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

	int width, height, nrChannels;
	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char *data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
		if (data)
		{
			glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data
			);
			stbi_image_free(data);
		}
		else
		{
			std::cout << "Cubemap texture failed to load at path: " << faces[i] << std::endl;
			stbi_image_free(data);
		}
	}
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

	return textureID;
}

