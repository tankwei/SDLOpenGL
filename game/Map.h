#ifndef MAP_H
#define MAP_H

#include <util/camera.h>
#include <util/shader.h>
#include <util/wtypes.h>


#define MAP_W       32       // size of map along x-axis 32 
#define MAP_SCALE   24.0f     // the scale of the terrain map
#define MAP			MAP_W*MAP_SCALE/2
#define RAND_COORD(x)   ((float)rand()/RAND_MAX * (x))

class Man;

class Map
{
public:
	struct map_point
	{
		float x, z;
	};
	

	Map();
	~Map(){}
	void Init();
	void InitGround();
	void InitTerrain(float h);
	void InitTree();
	bool CheckCollision(glm::vec3 pos, Man* costar, int costar_num, bool bullet);
	void Render(Camera &camera, glm::mat4 &projection, glm::mat4 &view);
private:
	void DrawSand(Camera &camera,glm::mat4 projection, glm::mat4 view);
	void drawGround(glm::mat4 projection, glm::mat4 view);
	unsigned char* LoadBit(const char *filename);
	float GetHeight(float x, float z);
private:
	GLuint		map_texture;
	GLuint		map_Tree_Texture[4];
	GLfloat size;
	//Shape shape[10];
	map_point p[6];
	float groundVerticesArray[480];
	GLuint groundVBO;
	GLuint groundVAO;
	GLuint index_buffer;
	Shader *groundShader;

	float		g_terrain[MAP_W*MAP_W][3];
	GLuint		g_index[MAP_W*MAP_W * 2];
	float		g_texcoord[MAP_W*MAP_W][2];
	unsigned char *g_imageData;
	GLuint terrianVBO;
	GLuint terrianVAO;
	GLuint vbo_terrian_colors;
	Shader *terrianShader;
	Shader *treeShader;
	GLuint treeVBO;
	GLuint treeVAO;
	GLuint treeVEO;
	GLuint treeInstanceVBO;
	float treeVertices[20];
	void OutputDebugPrintf1(const char* strOutputString, ...)
	{
#ifdef _DEBUG 
		char strBuffer[4096] = { 0 };
		va_list vlArgs;
		va_start(vlArgs, strOutputString);
		_vsnprintf_s(strBuffer, sizeof(strBuffer) - 1, strOutputString, vlArgs);
		//vsprintf(strBuffer,strOutputString,vlArgs);
		va_end(vlArgs);
		int slength = sizeof(strBuffer) + 1;
		int unicodeLen = MultiByteToWideChar(CP_ACP, 0, (const char *)strBuffer, -1, NULL, 0);
		std::wstring w_str(unicodeLen, '\0');
		MultiByteToWideChar(CP_ACP, 0, (const char *)strBuffer, -1, (LPWSTR)&w_str[0], unicodeLen);
		OutputDebugString(w_str.c_str());//CA2W(strBuffer)
#endif
	}
};


#endif