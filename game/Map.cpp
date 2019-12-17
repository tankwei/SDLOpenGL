#include "Map.h"
#include <util/filesystem.h>
#include <util/stb_image.h>
#include <SDL2/SDL.h>
#include <util/GameImgLoader.h>
#include <sstream>
#include <iostream>
#include <string>

float width = MAP * 6;
float height = MAP * 3;
float length = MAP * 6;
float xx = MAP - width / 2;
float yy = MAP / 3 - height / 2;
float zz = -MAP - length / 2;

float groundVertices[] = {
	 -0.25f, -0.25f, -0.25f,
			-0.25f,  0.25f, -0.25f,
			 0.25f, -0.25f, -0.25f,
			 0.25f,  0.25f, -0.25f,
			 0.25f, -0.25f,  0.25f,
			 0.25f,  0.25f,  0.25f,
			-0.25f, -0.25f,  0.25f,
			-0.25f,  0.25f,  0.25f,
};

float groundVertices_test[] = {
	-0.75f,    0.35f, -0.25f,
	 0.05f,   -0.50f, 10.85f,
	 1.0f,     0.35f, -0.25f,
	 0.15f,   -0.50f, 10.85f,
	 -10.05f, -2.650f, 0.0f,
	 10.15f,  -2.650f, 0.0f,
	 -10.05f, -2.450f, 0.0f,
	 10.15f,  -2.450f, 0.0f,
	 -10.05f, -2.250f, 0.0f,
	 10.15f,  -2.250f, 0.0f,
	 -10.05f, -2.050f, 0.0f,
	 10.15f,  -2.050f, 0.0f,
	  -10.05f, -1.850f, 0.0f,
	 10.15f,  -1.850f, 0.0f,
	  -10.05f, -1.650f, 0.0f,
	 10.15f,  -1.650f, 0.0f,
	  -10.05f, -1.450f, 0.0f,
	 10.15f,  -1.450f, 0.0f,
	  -10.05f, -1.250f, 0.0f,
	 10.15f,  -1.250f, 0.0f,
	  -10.05f, -1.050f, -0.125f,
	 10.15f,  -1.050f, -0.125f,
	 -10.05f, -0.850f, -0.25f,
	 10.15f,  -0.850f, -0.25f,
	-10.05f, -0.650f, -0.5f,
	 10.15f,  -0.650f, -0.5f,
	-10.05f, -0.450f, -1.0f,
	 10.15f,  -0.450f, -1.0f,
	-10.05f, -0.250f, -2.0f,
	 10.15f,  -0.250f, -2.0f,
	-10.05f, -0.050f, -5.0f,
	 10.15f,  -0.050f, -5.0f,
	 -10.05f, -0.050f, 0.0f,
	 10.15f,  -0.050f, 0.0f,
	-10.05f,  0.20f, -10.0f,
	 10.15f,   0.20f, -10.0f,
	 -10.05f,  0.20f, -5.0f,
	 10.15f,   0.20f, -5.0f,
	 -10.05f,  0.20f, -2.5f,
	 10.15f,   0.20f, -2.5f,
	 -10.05f,  0.20f, 0.0f,
	 10.15f,   0.20f, 0.0f
};

//42
float groundVertices1[] = {
	-0.75f,    0.35f, -0.25f,
	 0.05f,   -0.50f, 10.85f,
	 1.0f,     0.35f, -0.25f,
	 0.15f,   -0.50f, 10.85f,
	-10.05f, -0.050f, -5.0f,
	 10.15f,  -0.050f, -5.0f,
	 -10.05f, -0.050f, 0.0f,
	 10.15f,  -0.050f, 0.0f,
	 -10.05f,  0.20f, -12.5f,
	 10.15f,   0.20f, -12.5f,
	-10.05f,  0.20f, -10.0f,
	 10.15f,   0.20f, -10.0f,
	 -10.05f,  0.20f, -7.5f,
	 10.15f,   0.20f, -7.5f,
	 -10.05f,  0.20f, -5.0f,
	 10.15f,   0.20f, -5.0f,
	 -10.05f,  0.20f, -2.5f,
	 10.15f,   0.20f, -2.5f,
	 -10.05f,  0.20f, 0.0f,
	 10.15f,   0.20f, 0.0f,
	 -10.05f,  0.20f, 10.0f,
	 10.15f,   0.20f, 10.0f
};

static const GLushort vertex_indices[] =
{
	0, 1, 2,
	2, 1, 3,
	2, 3, 4,
	4, 3, 5,
	4, 5, 6,
	6, 5, 7,
	6, 7, 0,
	0, 7, 1,
	6, 0, 2,
	2, 4, 6,
	7, 5, 3,
	7, 3, 1
};

Map::Map()
{
	memset(&this->groundVerticesArray[0], 0, sizeof(this->groundVerticesArray));
	memset(&this->treeVertices[0], 0, sizeof(this->treeVertices));
}

void Map::Init()
{
	//glGenBuffers(1, &index_buffer);
	//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer);
	//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(vertex_indices), vertex_indices, GL_STATIC_DRAW);
	char *base_path = SDL_GetBasePath();
	this->terrianShader = new Shader(FileSystem::getPath(base_path, "resources/shaderFiles/terrianVertexShader.vs").c_str(), 
	FileSystem::getPath(base_path, "resources/shaderFiles/terrianFragmentShader.fs").c_str());
	this->InitTerrain(5);
	this->InitTree();
}

void Map::InitGround()
{
	//-1.5f, 0.35f, -0.25f,
		//-1.5f, -0.50f, 10.85f
	int verticeCount = 0;
	float h_axis_xx_left = -3.75f;
	float h_axis_yy_left = -0.85f;
	float h_axis_zz_left = -0.25f;
	float h_axis_xx_right = -3.75f;
	float h_axis_yy_right = -0.950f;
	float h_axis_zz_right = 10.85f;
	float y_axis_xx_left = -20.05f;
	float y_axis_yy_left = -2.550f;
	float y_axis_zz_left = -12.5f;
	float y_axis_xx_right = 20.15f;
	float y_axis_yy_right = -2.550f;
	float y_axis_zz_right = -12.5f;
	for (int i = 0; i < 40; i++) {
		this->groundVerticesArray[verticeCount++] = h_axis_xx_left + i * 0.25;
		this->groundVerticesArray[verticeCount++] = h_axis_yy_left;
		this->groundVerticesArray[verticeCount++] = h_axis_zz_left;

		this->groundVerticesArray[verticeCount++] = h_axis_xx_right + i * 0.25;
		this->groundVerticesArray[verticeCount++] = h_axis_yy_right;
		this->groundVerticesArray[verticeCount++] = h_axis_zz_right;
	}

	for (int j = 0; j < 40; j++) {
		this->groundVerticesArray[verticeCount++] = y_axis_xx_left;
		this->groundVerticesArray[verticeCount++] = y_axis_yy_left;
		this->groundVerticesArray[verticeCount++] = y_axis_zz_left + j * (0.5);

		this->groundVerticesArray[verticeCount++] = y_axis_xx_right;
		this->groundVerticesArray[verticeCount++] = y_axis_yy_right;
		this->groundVerticesArray[verticeCount++] = y_axis_zz_right + j * (0.5);
	}//groundVertices1;

	char *base_path = SDL_GetBasePath();
	this->groundShader = new Shader(FileSystem::getPath(base_path, "resources/shaderFiles/groundVertexShader.vs").c_str(),
		FileSystem::getPath(base_path, "resources/shaderFiles/groundFragmentShader.fs").c_str());
	glGenVertexArrays(1, &groundVAO);
	glGenBuffers(1, &groundVBO);
	glBindVertexArray(groundVAO);
	glBindBuffer(GL_ARRAY_BUFFER, groundVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(groundVerticesArray), groundVerticesArray, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, NULL);
	glEnableVertexAttribArray(0);
}

bool Map::CheckCollision(glm::vec3 pos, Man* costar, int costar_num, bool bullet)
{
	return false;
}

void Map::Render(Camera &camera, glm::mat4 &projection, glm::mat4 &view)
{
	this->DrawSand(camera,projection, view);
}

void Map::InitTerrain(float h)
{
	char *base_path = SDL_GetBasePath();
	this->g_imageData = LoadBit(FileSystem::getPath(base_path, "resources/Terrain1.bmp").c_str());
	GameImgLoader gameImgLoader;
	gameImgLoader.loadTextureData(&map_texture, FileSystem::getPath(base_path, "resources/sand0.bmp").c_str());
	int index = 0;
	int Vertex;
	for (int z = 0; z < MAP_W; z++) {
		for (int x = 0; x < MAP_W; x++) {
			Vertex = z * MAP_W + x;
			g_terrain[Vertex][0] = float(x) * MAP_SCALE;
			g_terrain[Vertex][1] = (float)(g_imageData[(z * MAP_W + x) * 3] / 3);
			g_terrain[Vertex][2] = -float(z) * MAP_SCALE;
			g_texcoord[Vertex][0] = (float)x;
			g_texcoord[Vertex][1] = (float)z;
			g_index[index++] = Vertex;
			g_index[index++] = Vertex + MAP_W;
		}
	}
	glGenVertexArrays(1, &terrianVAO);
	glGenBuffers(1, &terrianVBO);
	glBindVertexArray(terrianVAO);
	glBindBuffer(GL_ARRAY_BUFFER, terrianVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_terrain), g_terrain, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	glGenBuffers(1, &vbo_terrian_colors);
	glBindBuffer(GL_ARRAY_BUFFER, vbo_terrian_colors);
	glBufferData(GL_ARRAY_BUFFER, sizeof(g_texcoord), g_texcoord, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), NULL);
	glEnableVertexAttribArray(1);
}

void Map::InitTree()
{
	char *base_path = SDL_GetBasePath();
	GameImgLoader gameImgLoader;
	gameImgLoader.loadTextureData(&map_Tree_Texture[0], FileSystem::getPath(base_path, "resources/tree/tree5.bmp").c_str());
	gameImgLoader.loadTextureData(&map_Tree_Texture[1], FileSystem::getPath(base_path, "resources/tree/tree2.bmp").c_str());
	gameImgLoader.loadTextureData(&map_Tree_Texture[2], FileSystem::getPath(base_path, "resources/tree/tree3.bmp").c_str());
	gameImgLoader.loadTextureData(&map_Tree_Texture[3], FileSystem::getPath(base_path, "resources/tree/tree4.bmp").c_str());
	this->treeShader = new Shader(FileSystem::getPath(base_path, "resources/shaderFiles/treeVertexShader.vs").c_str(), FileSystem::getPath(base_path, "resources/shaderFiles/treeFragmentShader.fs").c_str());
	srand(100);
	//for (int i = 0; i < 1; i++)
	//{
	//	float x = RAND_COORD((MAP_W - 1)*MAP_SCALE);
	//	float z = RAND_COORD((MAP_W - 1)*MAP_SCALE);
	//	float size = 4.0f + rand() % 4;
	//	float h = -size / 10;
	//	int cactus = rand() % 4 + 11;
	//	//ShowTree(x, z, size, h, cactus);

	//	//glEnable(GL_BLEND);
	//	//glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//	//glEnable(GL_ALPHA_TEST);
	//	//glAlphaFunc(GL_GREATER, 0);
	//	float mat[16];
	//	//glGetFloatv(GL_MODELVIEW_MATRIX, mat);
	//https://blog.csdn.net/zulinlv/article/details/89140211
	//https://gamedev.stackexchange.com/questions/34944/glgetfloatv-should-it-be-avoided-can-it-be-replaced
	//	//glm::mat4 mv = viewM * trans1;
	//	//float *matArray = glm::value_ptr(mv);
	//	glm::vec3 X(mat[0], mat[4], mat[8]);
	//	glm::vec3 Z(mat[1], mat[5], mat[9]);
	//	//glBindTexture(GL_TEXTURE_2D, g_cactus[cactus]);
	//	glm::vec3 pos(x, 0.0, -z);
	//	pos.y = GetHeight(x, -z) + h + size;
	//	//glBegin(GL_QUADS);
	//	//glTexCoord2f(0.0, 0.0); glVertex3fv((pos + (X + Z)*-h).v);//���µ�
	//	glm::vec3 temp = (pos + (X + Z)*-h);
	//	treeVertices[i] = temp.x;
	//	treeVertices[i + 1] = temp.y;
	//	treeVertices[i + 2] = temp.z;
	//	treeVertices[i + 3] = 0.0;
	//	treeVertices[i + 4] = 0.0;
	//	//glTexCoord2f(1.0, 0.0); glVertex3fv((pos + (X - Z)* h).v);//���µ�
	//	temp = (pos + (X - Z)* h);
	//	treeVertices[i + 5] = temp.x;
	//	treeVertices[i + 6] = temp.y;
	//	treeVertices[i + 7] = temp.z;
	//	treeVertices[i + 8] = 1.0;
	//	treeVertices[i + 9] = 0.0;
	//	//glTexCoord2f(1.0, 1.0); glVertex3fv((pos + (X + Z)* h).v);//���ϵ�
	//	temp = (pos + (X + Z)* h);
	//	treeVertices[i + 10] = temp.x;
	//	treeVertices[i + 11] = temp.y;
	//	treeVertices[i + 12] = temp.z;
	//	treeVertices[i + 13] = 1.0;
	//	treeVertices[i + 14] = 1.0;
	//	//glTexCoord2f(0.0, 1.0); glVertex3fv((pos + (Z - X)* h).v);//���ϵ�
	//	temp = (pos + (Z - X)* h);
	//	treeVertices[i + 15] = temp.x;
	//	treeVertices[i + 16] = temp.y;
	//	treeVertices[i + 17] = temp.z;
	//	treeVertices[i + 18] = 0.0;
	//	treeVertices[i + 19] = 1.0;
	//}
	float x = RAND_COORD((MAP_W - 1)*MAP_SCALE);
	float z = RAND_COORD((MAP_W - 1)*MAP_SCALE);
	float tree_Vertices[] = {
		// positions              // texture coords
			 0.5f,  0.5f, 0.0f,    1.0f, 1.0f, // top right
			 0.5f, -0.5f, 0.0f,    1.0f, 0.0f, // bottom right
			-0.5f, -0.5f, 0.0f,    0.0f, 0.0f, // bottom left
			-0.5f,  0.5f, 0.0f,    0.0f, 1.0f  // top left 

		/*-0.5f, -0.5f, 0.0f,
		 0.5f, -0.5f, 0.0f,
		 0.0f,  0.5f, 0.0f*/
	};
	unsigned int indices[] = {
	   0, 1, 3, // first triangle
	   1, 2, 3  // second triangle
	};
	// store instance data in an array buffer
	// --------------------------------------
	
	/*glGenBuffers(1, &treeInstanceVBO);
	glBindBuffer(GL_ARRAY_BUFFER, treeInstanceVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(glm::mat4), &translations[0], GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);*/
	glGenVertexArrays(1, &treeVAO);
	glGenBuffers(1, &treeVBO);
	glGenBuffers(1, &treeVEO);
	glBindVertexArray(treeVAO);
	glBindBuffer(GL_ARRAY_BUFFER, treeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(tree_Vertices), tree_Vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), NULL);
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, treeVEO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
	
	//glBindBuffer(GL_ARRAY_BUFFER, treeInstanceVBO); // this attribute comes from a different vertex buffer
	//glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)0);
	//glEnableVertexAttribArray(2);
	//glBindBuffer(GL_ARRAY_BUFFER, 0);
	//glVertexAttribDivisor(2, 1); // tell OpenGL this is an instanced vertex attribute.
}

float Map::GetHeight(float x, float z)
{
	float CameraX = x / MAP_SCALE;
	float CameraZ = -z / MAP_SCALE;
	int Col0 = int(CameraX);
	int Row0 = int(CameraZ);
	int Col1 = Col0 + 1;
	int Row1 = Row0 + 1;
	if (Col1 > MAP_W)	Col1 = 0;
	if (Row1 > MAP_W)	Row1 = 0;
	float h00 = g_terrain[Col0 + Row0 * MAP_W][1];
	float h01 = g_terrain[Col1 + Row0 * MAP_W][1];
	float h11 = g_terrain[Col1 + Row1 * MAP_W][1];
	float h10 = g_terrain[Col0 + Row1 * MAP_W][1];
	float tx = CameraX - int(CameraX);
	float ty = CameraZ - int(CameraZ);
	float txty = tx * ty;
	return h00 * (1.0f - ty - tx + txty)
		+ h01 * (tx - txty)
		+ h11 * txty
		+ h10 * (ty - txty);
}

void Map::DrawSand(Camera &camera,glm::mat4 projection, glm::mat4 view)
{
	this->terrianShader->use();
	glBindVertexArray(terrianVAO);
	glBindTexture(GL_TEXTURE_2D, this->map_texture);
	
	//float yyy = GetHeight(20.0f, 20.0);
	//float yyy_scale = yyy;//3.45f
	glm::mat4 anim =
	glm::translate(glm::mat4(1.0f), glm::vec3(65.0f, 0.0f, 0.0f)) *
		glm::rotate(glm::mat4(1.0f), 1.0f * 141.29f, glm::vec3(0, 0, 1)) *
		glm::rotate(glm::mat4(1.0f), -1.0f * 0.003f, glm::vec3(1, 0, 0)) *
		glm::scale(glm::mat4(1.0), glm::vec3(1.0, 1.0, 1.0));
	/*glm::translate(glm::mat4(1.0f), glm::vec3(55.0f, 15.0f, 0.0)) *
		glm::rotate(glm::mat4(1.0f), 1 * 22.0f, glm::vec3(0, 0, 1)) *
		glm::scale(glm::mat4(1.0), glm::vec3(0.5, 0.5 * 3.0 / 4.0, 0.5));*/
	this->terrianShader->setMat4("transform", anim);
	terrianShader->setMat4("projection",projection);
	terrianShader->setMat4("view", view);
	for (int z = 0; z < MAP_W - 1; z++)
		glDrawElements(GL_TRIANGLE_STRIP, MAP_W * 2, GL_UNSIGNED_INT, &g_index[z*MAP_W * 2]);

	//https://www.cnblogs.com/yizhen/p/10730500.html
	//https://blog.csdn.net/augusdi/article/details/19989595

	//https://github.com/franjaviersans/MD2-OpenGL-Loader  GLSL
	//http://www.zwqxin.com/archives/opengl/md2-model-format-import-animation.html
	//https://www.dssz.com/665646.html

	//https://www.cnblogs.com/sgamerw/p/4108877.html//[��]ѧϰ�ִ�OpenGL
	//http://ogldev.atspace.co.uk/index.html
	// https://blog.csdn.net/zju_fish1996/article/details/52450008
	//https://blog.csdn.net/theonegis/article/details/50245459
	//https://blog.csdn.net/cordova/article/details/53350395

	//https://github.com/tomdalling/opengl-series
	//http://huangwei.pro/2015-05/modern-opengl1/
	//https://bbs.csdn.net/topics/392006905
	//https://blog.csdn.net/ZJU_fish1996/article/details/51876197
	//https://download.csdn.net/download/bselff/7963447
	//https://my.oschina.net/gujianhan/blog/166853
	//http://blog.chinaunix.net/uid-25411469-id-3145016.html
	//https://blog.csdn.net/dqmengxiang/article/details/5791740
	//https://gameinstitute.qq.com/community/detail/113742
	//https://www.cnblogs.com/AnKen/p/8400067.html
	//https://www.jianshu.com/p/2bed156e3236 ���Ƶ��ε�òhttps://github.com/SquarePants1991/OpenGLESLearn
	//
	//https://www.gamedev.net/forums/topic/703949-terrain-mapping-more-earthquake-when-closer-to-ground/?tab=comments#comment-5413297
	
	
	glBindTexture(GL_TEXTURE_2D, this->map_Tree_Texture[0]);
	this->treeShader->use();
	glBindVertexArray(treeVAO);
	glm::mat4 model = glm::mat4(1.0);
	//0.0f, -45.0f, -35.0f
	model = glm::translate(model, glm::vec3(0.0f, -48.3f, -35.0f));
	//model = glm::scale(model, glm::vec3(10.0, 10.0, 1.0));
	//treeShader.setMat4("transform", model);
	glm::mat4 translations[100];
	int index = 0;
	float z_offset = -1.2f;
	float x_offset = -1.1f;
	srand(100);
	//0.0f, -45.0f, -35.0f
	glm::mat4 viewM = camera.GetViewMatrix();
	for (int i = 0; i < 100; i++)
	{
		float x_index = RAND_COORD((100 - 1));
		float z_index = RAND_COORD((100 - 1));
		float xx = 0.0f + x_offset * x_index;
		float zz = -35.0f + z_offset * z_index;
		float yy = this->GetHeight(xx, zz);

		glm::mat4 trans = glm::mat4(1.0);
		trans = glm::translate(trans, glm::vec3(xx, -45.1f, zz));
		trans = glm::scale(trans, glm::vec3(10.0, 10.0, 1.0));
		translations[i] = trans;
	}
	for (unsigned int i = 0; i < 100; i++)
	{
		std::stringstream ss;
		std::string index;
		ss << i;
		index = ss.str();
		treeShader->setMat4(("offsets[" + index + "]").c_str(), translations[i]);
	}
	this->treeShader->setMat4("projection", projection);
	treeShader->setMat4("view", view);
	//glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
	glDrawElementsInstanced(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0, 100);
	//glDrawArraysInstanced(GL_TRIANGLES, 0, 6, 100);
}

void Map::drawGround(glm::mat4 projection, glm::mat4 view)
{
	this->groundShader->use();
	glBindVertexArray(groundVAO);
	glDrawArrays(GL_LINES, 0, 480);

	//glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	//glEnable(GL_LINE_SMOOTH);
	glm::mat4 trans = glm::mat4(1.0f);
	trans = glm::translate(trans, glm::vec3(0.0f, 0.0f, -4.0));
	this->groundShader->setMat4("transform", trans);

	groundShader->setMat4("projection", projection);
	// camera/view transformation
	//glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
	//glm::vec3 cameraRight = glm::normalize(glm::cross(up, cameraPos + cameraFront));
	groundShader->setMat4("view", view);
}

unsigned char* Map::LoadBit(const char *filename)
{
	//https://www.linuxidc.com/Linux/2012-05/61456.htm
	// /https://blog.csdn.net/songyulong8888/article/details/94001862
	//https://www.cnblogs.com/lidabo/p/3326500.html
	//https://blog.csdn.net/weiaipan1314/article/details/51753288
	//https://blog.csdn.net/wangjunchao001/article/details/52798304
	//https://bbs.csdn.net/topics/390209790
	//https://zhidao.baidu.com/question/240675687911717044.html
	//http://www.itkeyword.com/doc/1114926412907587021/qt-desktop-shader-compilation-issue
	FILE *filePtr = nullptr;
	BITMAPINFOHEADER bmpInfor_Header;
	BITMAPFILEHEADER bmpFile_Header;
	unsigned short fileType = 0;
	unsigned char* ImageData = NULL;
	unsigned int imageIdx = 0;
	unsigned char tempRGB;
	filePtr = fopen(filename, "rb");
	if (NULL == filePtr){
		return NULL;
	}
	fread(&fileType,sizeof(unsigned short),1,filePtr);
	if(0x4D42 == fileType){
		fread(&bmpFile_Header, sizeof(BITMAPFILEHEADER), 1, filePtr);
		printf("\n***************bmp文件头信息************************\n"); 
        printf("文件大小：%d \n", bmpFile_Header.bfSize); 
        printf("保留字：%d \n", bmpFile_Header.bfReserved1); 
        printf("保留字：%d \n", bmpFile_Header.bfReserved2); 
        printf("位图数据偏移字节数：%d \n", bmpFile_Header.bfOffBits);
		fread(&bmpInfor_Header, sizeof(BITMAPINFOHEADER), 1, filePtr);
		//fseek(filePtr, bmpInfor_Header.bfOffBits, SEEK_SET);
		/*if (bmpInfor_Header.bfType != BITMAP_ID){
			fclose(filePtr);
			return NULL;
		}*/
	    int channels = 0;
		int width = 0;
		int height = 0;
		int bmp_biBitCount = bmpInfor_Header.biBitCount;
		if(8 == bmp_biBitCount){
			
		}
		else if(24 == bmp_biBitCount){
			channels = 3;
			width = bmpInfor_Header.biWidth;
			height = bmpInfor_Header.biHeight;
			ImageData = (unsigned char *)malloc(sizeof(unsigned char *) * width * height);
			if (nullptr == ImageData){
				free(ImageData);
				fclose(filePtr);
				filePtr = NULL;
				return NULL;
			}
			fread(ImageData, 1, bmpInfor_Header.biSizeImage, filePtr);
			if (NULL == ImageData){
				fclose(filePtr);
				return NULL;
			}
			for (imageIdx = 0; imageIdx < bmpInfor_Header.biSizeImage; imageIdx += 3){
				tempRGB = ImageData[imageIdx];
				ImageData[imageIdx] = ImageData[imageIdx + 2];
				ImageData[imageIdx + 2] = tempRGB;
			}
		}
	}
	fclose(filePtr);
	filePtr = NULL;
	return ImageData;
}