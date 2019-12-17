#include "glad/glad.h"
#include "app.h"
#include "game/Map.h"
#include "sdl_util/gl/glut.h"
#include <SDL2/SDL_filesystem.h>
#include <png/png.h>
#include "sdl_util/Renderer.h"
#include "sdl_util/Image.h"
#include "sdl_util/Window.h"
#include "sdl_util/SDLTexture.h"
#include "util/stb_image.h"
//#include <windows.h>
#include <util/camera.h>
#include <util/shader.h>
#include <util/filesystem.h>
#include <util/GameImgLoader.h>
#include "util/stb_image_write.h"

using namespace std;

int savePngDataToFile (const char *path, unsigned long width, unsigned long height){
    FILE* fp;
    png_structp png_ptr = NULL;
    png_infop info_ptr = NULL;
    size_t x, y;
    png_bytep *row_pointers = NULL;
    /* "status" contains the return value of this function. At first
       it is set to a value which means 'failure'. When the routine
       has finished its work, it is set to a value which means
       'success'. */
    int status = -1;
    /* The following number is set by trial and error only. I cannot
       see where it it is documented in the libpng manual.
    */
    int pixel_size = 3;
    int depth = 8;
    
    fp = fopen (path, "wb");
    if (!fp) {
        return status;
    }
    png_ptr = png_create_write_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose(fp);
		return status;
    }
    info_ptr = png_create_info_struct (png_ptr);
    if (info_ptr == NULL) {
        png_destroy_write_struct (&png_ptr, &info_ptr);
		return status;
    }
    /* Set up error handling. */
    if (setjmp (png_jmpbuf (png_ptr))) {
        png_destroy_write_struct (&png_ptr, &info_ptr);
		fclose (fp);
		return status;
    }
    
    /* Set image attributes. */

    png_set_IHDR (png_ptr,
                  info_ptr,
                  width,
                  height,
                  depth,
                  PNG_COLOR_TYPE_RGB,
                  PNG_INTERLACE_NONE,
                  PNG_COMPRESSION_TYPE_DEFAULT,
                  PNG_FILTER_TYPE_DEFAULT);
    
    /* Initialize rows of PNG. */
	//row_pointers = new png_bytep[height * (sizeof(png_bytep))];
	// for(int i = 0;i < bitmap->height;i++){
	// 	 row_pointers[i] = (png_bytep)png_malloc (png_ptr, bitmap->height * sizeof (png_byte *));
	// }
    // for (y = 0; y < height; y++) {
    //     //png_byte *row =  (png_byte *)png_malloc (png_ptr, sizeof(uint8_t) * width * pixel_size);
    //     row_pointers[y] = pdata;
    //     for (x = 0; x < width; x++) {
            
    //     }
    // }
	//int alignment = 4;
	//glPixelStorei(GL_UNPACK_ALIGNMENT,alignment);//GL_BGR
	unsigned char* pdata = (unsigned char*)malloc(width * height * 3 * sizeof(png_byte));
	memset(pdata,0,width * height * 3);
	glReadPixels(0,0,width,height,GL_RGB,GL_UNSIGNED_BYTE,pdata);
	row_pointers = (png_bytep *)malloc(height * sizeof(png_bytep));
	for (int i = 0; i < height; i++){
        row_pointers[i] = (png_bytep)pdata + (height - i) * width * 3;
    }
    /* Write the image data to "fp". */
    png_init_io (png_ptr, fp);
    png_set_rows (png_ptr, info_ptr, row_pointers);
    png_write_png (png_ptr, info_ptr, PNG_TRANSFORM_IDENTITY, NULL);

    /* The routine has successfully written the file, so we set
       "status" to a value which indicates success. */
    status = 0;
    for (y = 0; y < height; y++) {
        //png_free (png_ptr, row_pointers[y]);
    }
    //png_free (png_ptr, row_pointers);
	fclose(fp);
	fp = NULL;

    return status;
}

const unsigned int FPS = 1000 / 100000;
unsigned int _FPS_Timer;

float deltaTime = 0.0f;
float lastFrame = 0.0f;

// camera
Camera appcamera(glm::vec3(0.0f, 0.0f, 3.0f));


const int SCALE_FACTOR = 1;

#define PNG_BYTES_TO_CHECK 8

// camera
glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);


GLubyte color[3][4] = {
	{ 255, 0, 0, 255 },
	{ 0, 255, 0, 255 },
	{ 0, 255, 0, 255 }
};

GLfloat vTriangle[3][3] = {
	0.0f, 0.5f, 0.0f,
	0.5f, 0.0f, 0.0f,
	-0.5f, 0.0f, 0.0f
};

unsigned int triangleVBO;
unsigned int triangleVAO;
GLuint triangleVEO;
unsigned int triangleShaderProgram;
GLuint triangleTexture;

float vertices[] = {
	// positions          // colors           // texture coords
		 1.0f,  1.0f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f, // top right
		 1.0f, -1.0f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f, // bottom right
		-1.0f, -1.0f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f, // bottom left
		-1.0f,  1.0f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f  // top left 

	/*-0.5f, -0.5f, 0.0f,
	 0.5f, -0.5f, 0.0f,
	 0.0f,  0.5f, 0.0f*/
};

unsigned int indices[] = {
	   0, 1, 3, // first triangle
	   1, 2, 3  // second triangle
};

float pointVertices[] = {
	 -0.6f, 0.0f, 0.0f
	 -0.6f, 0.5f, 0.0f,
	 -0.6f, 0.8f, 0.0f
};

float pointVertices1[] = {
	 0.0f, 1.0f, -1.0f,
	 -1.0f, -1.0f, 0.0f,
	 1.0f,  -1.0f, 0.0f
};

float Vertices1[] = {
	 0.0f, 1.0f, -1.0f,
	 -1.0f, -1.0f, 0.0f,
	 1.0f,  -1.0f, 0.0f
};

float Esquare[] = {
	     -0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,

		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f, -0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,

		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,

		-0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f, -0.5f,
		 0.5f, -0.5f,  0.5f,
		 0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f,  0.5f,
		-0.5f, -0.5f, -0.5f,

		-0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f, -0.5f,
		 0.5f,  0.5f,  0.5f,
		 0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f,  0.5f,
		-0.5f,  0.5f, -0.5f
};

// Shaders
const char* pointVertexShaderSource = "#version 320 es\n"
"layout (location = 0) in mediump vec3 aPos;\n"
"void main(){gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);}";
const char* pointFragmentShaderSource = 
"#version 320 es\n"
"out mediump vec4 FragColor;\n"
"void main(){FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}";

const char* triangleVertexShaderSource = "#version 320 es\n"
"layout (location = 0) in mediump vec3 aPos;\n"
"layout(location = 1) in mediump vec3 aColor;\n"
"layout(location = 2) in mediump vec2 aTexCoord;\n"
"out mediump vec3 ourColor;\n"
"out mediump vec2 TexCoord;\n"
"uniform mat4 transform;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"gl_Position = projection * view * transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
"ourColor = aColor;\n"
"TexCoord = aTexCoord;\n"
"}";

const char* triangleFragmentShaderSource = "#version 320 es\n"
"out mediump vec4 FragColor;\n"
"in mediump vec3 ourColor;\n"
"in mediump vec2 TexCoord;\n"
"uniform sampler2D texture1;\n"
"void main(){\n"
"FragColor = texture(texture1, TexCoord);//vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
"}";

const char* pillarVertexShaderSource = "#version 320 es\n"
"layout (location = 0) in mediump vec3 aPos;\n"
"uniform mat4 transform;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main(){gl_Position = projection * view * transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);}";
const char* pillarFragmentShaderSource = "#version 320 es\n"
"out mediump vec4 FragColor;void main(){FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);}";

const char* teapotVertexShaderSource = "#version 320 es\n"
"layout (location = 0) in mediump vec3 aPos;\n"
"layout (location = 1) in mediump vec3 a_color;\n"
"uniform mat4 transform;\n"
"out mediump vec3 f_color;\n"
"void main(){\n"
"gl_Position = transform * vec4(aPos.x, aPos.y, aPos.z, 1.0);"
"f_color=a_color;"
"}";
const char* teapotFragmentShaderSource = "#version 320 es\n"
"out mediump vec4 FragColor;\n"
"in mediump vec3 f_color;\n"
"void main(){\n"
"FragColor = vec4(f_color, 1.0f);"
"}";

const char* cubeVertexShaderSource = "#version 320 es\n"
"layout (location = 0) in vec3 aPos;\n"
"out mediump vec3 TexCoord;\n"
"uniform mat4 view;\n"
"uniform mat4 projection;\n"
"void main(){\n"
"gl_Position = projection * view * vec4(aPos.x, aPos.y, aPos.z, 1.0);TexCoord=aPos;}";
const char* cubeFragmentShaderSource = "#version 320 es\n"
"out mediump vec4 FragColor;\n"
"in mediump vec3 TexCoord;\n"
"uniform samplerCube skybox;\n"
"void main(){\n"
"FragColor = texture(skybox,TexCoord);}";


unsigned int EsquareVBO;
unsigned int EsquareVAO;
unsigned int EsquareShaderProgram;

unsigned int pointVBO;
unsigned int pointVAO;
unsigned int pointShaderProgram;
unsigned int pillarVBO;
unsigned int pillarVAO;
unsigned int pillarShaderProgram;
unsigned int pillar1VBO;
unsigned int pillar1VAO;
unsigned int pillarShaderProgram1;
unsigned int  SolidConeVBO;
unsigned int  SolidConeVAO;
unsigned int  SolidConeShaderProgram;

unsigned int teapot_ShaderProgram;
unsigned int vbo_teapot_vertices;
unsigned int vao_teapot;
unsigned int ibo_teapot_elements;
GLuint vbo_teapot_colors, vbo_teapot_cp_vertices, vao_teapot_cp_element,ibo_teapot_cp_elements;

struct vertex {GLfloat x, y, z;};

#define ORDER 3
struct vertex control_points_k[ORDER + 1][ORDER + 1] = {
  { { 1, 2, 3}, { 4, 5, 6}, { 7, 8, 9}, {10,11,12} },
  { {13,14,15}, {16,17,18}, {19,20,21}, {22,23,24} }
};

//The indices :

#define TEAPOT_NB_PATCHES 28
GLushort teapot_patches[][ORDER + 1][ORDER + 1] = {
	// rim
  { {   1,   2,   3,   4 }, {   5,   6,   7,   8 }, {   9,  10,  11,  12 }, {  13,  14,  15,  16, } },
  { {   4,  17,  18,  19 }, {   8,  20,  21,  22 }, {  12,  23,  24,  25 }, {  16,  26,  27,  28, } },
  { {  19,  29,  30,  31 }, {  22,  32,  33,  34 }, {  25,  35,  36,  37 }, {  28,  38,  39,  40, } },
  { {  31,  41,  42,   1 }, {  34,  43,  44,   5 }, {  37,  45,  46,   9 }, {  40,  47,  48,  13, } },
  // body
  { {  13,  14,  15,  16 }, {  49,  50,  51,  52 }, {  53,  54,  55,  56 }, {  57,  58,  59,  60, } },
  { {  16,  26,  27,  28 }, {  52,  61,  62,  63 }, {  56,  64,  65,  66 }, {  60,  67,  68,  69, } },
  { {  28,  38,  39,  40 }, {  63,  70,  71,  72 }, {  66,  73,  74,  75 }, {  69,  76,  77,  78, } },
  { {  40,  47,  48,  13 }, {  72,  79,  80,  49 }, {  75,  81,  82,  53 }, {  78,  83,  84,  57, } },
  { {  57,  58,  59,  60 }, {  85,  86,  87,  88 }, {  89,  90,  91,  92 }, {  93,  94,  95,  96, } },
  { {  60,  67,  68,  69 }, {  88,  97,  98,  99 }, {  92, 100, 101, 102 }, {  96, 103, 104, 105, } },
  { {  69,  76,  77,  78 }, {  99, 106, 107, 108 }, { 102, 109, 110, 111 }, { 105, 112, 113, 114, } },
  { {  78,  83,  84,  57 }, { 108, 115, 116,  85 }, { 111, 117, 118,  89 }, { 114, 119, 120,  93, } },
  // handle
  { { 121, 122, 123, 124 }, { 125, 126, 127, 128 }, { 129, 130, 131, 132 }, { 133, 134, 135, 136, } },
  { { 124, 137, 138, 121 }, { 128, 139, 140, 125 }, { 132, 141, 142, 129 }, { 136, 143, 144, 133, } },
  { { 133, 134, 135, 136 }, { 145, 146, 147, 148 }, { 149, 150, 151, 152 }, {  69, 153, 154, 155, } },
  { { 136, 143, 144, 133 }, { 148, 156, 157, 145 }, { 152, 158, 159, 149 }, { 155, 160, 161,  69, } },
  // spout
  { { 162, 163, 164, 165 }, { 166, 167, 168, 169 }, { 170, 171, 172, 173 }, { 174, 175, 176, 177, } },
  { { 165, 178, 179, 162 }, { 169, 180, 181, 166 }, { 173, 182, 183, 170 }, { 177, 184, 185, 174, } },
  { { 174, 175, 176, 177 }, { 186, 187, 188, 189 }, { 190, 191, 192, 193 }, { 194, 195, 196, 197, } },
  { { 177, 184, 185, 174 }, { 189, 198, 199, 186 }, { 193, 200, 201, 190 }, { 197, 202, 203, 194, } },
  // lid
  { { 204, 204, 204, 204 }, { 207, 208, 209, 210 }, { 211, 211, 211, 211 }, { 212, 213, 214, 215, } },
  { { 204, 204, 204, 204 }, { 210, 217, 218, 219 }, { 211, 211, 211, 211 }, { 215, 220, 221, 222, } },
  { { 204, 204, 204, 204 }, { 219, 224, 225, 226 }, { 211, 211, 211, 211 }, { 222, 227, 228, 229, } },
  { { 204, 204, 204, 204 }, { 226, 230, 231, 207 }, { 211, 211, 211, 211 }, { 229, 232, 233, 212, } },
  { { 212, 213, 214, 215 }, { 234, 235, 236, 237 }, { 238, 239, 240, 241 }, { 242, 243, 244, 245, } },
  { { 215, 220, 221, 222 }, { 237, 246, 247, 248 }, { 241, 249, 250, 251 }, { 245, 252, 253, 254, } },
  { { 222, 227, 228, 229 }, { 248, 255, 256, 257 }, { 251, 258, 259, 260 }, { 254, 261, 262, 263, } },
  { { 229, 232, 233, 212 }, { 257, 264, 265, 234 }, { 260, 266, 267, 238 }, { 263, 268, 269, 242, } },
  // no bottom!
};

struct vertex teapot_cp_vertices[] = {
	// 1
	{  1.4   ,   0.0   ,  2.4     },
	{  1.4   ,  -0.784 ,  2.4     },
	{  0.784 ,  -1.4   ,  2.4     },
	{  0.0   ,  -1.4   ,  2.4     },
	{  1.3375,   0.0   ,  2.53125 },
	{  1.3375,  -0.749 ,  2.53125 },
	{  0.749 ,  -1.3375,  2.53125 },
	{  0.0   ,  -1.3375,  2.53125 },
	{  1.4375,    0.0  ,  2.53125 },
	{  1.4375,  -0.805 ,  2.53125 },
	// 11
	{  0.805 ,  -1.4375,  2.53125 },
	{  0.0   ,  -1.4375,  2.53125 },
	{  1.5   ,   0.0   ,  2.4     },
	{  1.5   ,  -0.84  ,  2.4     },
	{  0.84  ,  -1.5   ,  2.4     },
	{  0.0   ,  -1.5   ,  2.4     },
	{ -0.784 ,  -1.4   ,  2.4     },
	{ -1.4   ,  -0.784 ,  2.4     },
	{ -1.4   ,   0.0   ,  2.4     },
	{ -0.749 ,  -1.3375,  2.53125 },
	// 21
	{ -1.3375,  -0.749 ,  2.53125 },
	{ -1.3375,   0.0   ,  2.53125 },
	{ -0.805 ,  -1.4375,  2.53125 },
	{ -1.4375,  -0.805 ,  2.53125 },
	{ -1.4375,   0.0   ,  2.53125 },
	{ -0.84  ,  -1.5   ,  2.4     },
	{ -1.5   ,  -0.84  ,  2.4     },
	{ -1.5   ,   0.0   ,  2.4     },
	{ -1.4   ,   0.784 ,  2.4     },
	{ -0.784 ,   1.4   ,  2.4     },
	// 31
	{  0.0   ,   1.4   ,  2.4     },
	{ -1.3375,   0.749 ,  2.53125 },
	{ -0.749 ,   1.3375,  2.53125 },
	{  0.0   ,   1.3375,  2.53125 },
	{ -1.4375,   0.805 ,  2.53125 },
	{ -0.805 ,   1.4375,  2.53125 },
	{  0.0   ,   1.4375,  2.53125 },
	{ -1.5   ,   0.84  ,  2.4     },
	{ -0.84  ,   1.5   ,  2.4     },
	{  0.0   ,   1.5   ,  2.4     },
	// 41
	{  0.784 ,   1.4   ,  2.4     },
	{  1.4   ,   0.784 ,  2.4     },
	{  0.749 ,   1.3375,  2.53125 },
	{  1.3375,   0.749 ,  2.53125 },
	{  0.805 ,   1.4375,  2.53125 },
	{  1.4375,   0.805 ,  2.53125 },
	{  0.84  ,   1.5   ,  2.4     },
	{  1.5   ,   0.84  ,  2.4     },
	{  1.75  ,   0.0   ,  1.875   },
	{  1.75  ,  -0.98  ,  1.875   },
	// 51
	{  0.98  ,  -1.75  ,  1.875   },
	{  0.0   ,  -1.75  ,  1.875   },
	{  2.0   ,   0.0   ,  1.35    },
	{  2.0   ,  -1.12  ,  1.35    },
	{  1.12  ,  -2.0   ,  1.35    },
	{  0.0   ,  -2.0   ,  1.35    },
	{  2.0   ,   0.0   ,  0.9     },
	{  2.0   ,  -1.12  ,  0.9     },
	{  1.12  ,  -2.0   ,  0.9     },
	{  0.0   ,  -2.0   ,  0.9     },
	// 61
	{ -0.98  ,  -1.75  ,  1.875   },
	{ -1.75  ,  -0.98  ,  1.875   },
	{ -1.75  ,   0.0   ,  1.875   },
	{ -1.12  ,  -2.0   ,  1.35    },
	{ -2.0   ,  -1.12  ,  1.35    },
	{ -2.0   ,   0.0   ,  1.35    },
	{ -1.12  ,  -2.0   ,  0.9     },
	{ -2.0   ,  -1.12  ,  0.9     },
	{ -2.0   ,   0.0   ,  0.9     },
	{ -1.75  ,   0.98  ,  1.875   },
	// 71
	{ -0.98  ,   1.75  ,  1.875   },
	{  0.0   ,   1.75  ,  1.875   },
	{ -2.0   ,   1.12  ,  1.35    },
	{ -1.12  ,   2.0   ,  1.35    },
	{  0.0   ,   2.0   ,  1.35    },
	{ -2.0   ,   1.12  ,  0.9     },
	{ -1.12  ,   2.0   ,  0.9     },
	{  0.0   ,   2.0   ,  0.9     },
	{  0.98  ,   1.75  ,  1.875   },
	{  1.75  ,   0.98  ,  1.875   },
	// 81
	{  1.12  ,   2.0   ,  1.35    },
	{  2.0   ,   1.12  ,  1.35    },
	{  1.12  ,   2.0   ,  0.9     },
	{  2.0   ,   1.12  ,  0.9     },
	{  2.0   ,   0.0   ,  0.45    },
	{  2.0   ,  -1.12  ,  0.45    },
	{  1.12  ,  -2.0   ,  0.45    },
	{  0.0   ,  -2.0   ,  0.45    },
	{  1.5   ,   0.0   ,  0.225   },
	{  1.5   ,  -0.84  ,  0.225   },
	// 91
	{  0.84  ,  -1.5   ,  0.225   },
	{  0.0   ,  -1.5   ,  0.225   },
	{  1.5   ,   0.0   ,  0.15    },
	{  1.5   ,  -0.84  ,  0.15    },
	{  0.84  ,  -1.5   ,  0.15    },
	{  0.0   ,  -1.5   ,  0.15    },
	{ -1.12  ,  -2.0   ,  0.45    },
	{ -2.0   ,  -1.12  ,  0.45    },
	{ -2.0   ,   0.0   ,  0.45    },
	{ -0.84  ,  -1.5   ,  0.225   },
	// 101
	{ -1.5   ,  -0.84  ,  0.225   },
	{ -1.5   ,   0.0   ,  0.225   },
	{ -0.84  ,  -1.5   ,  0.15    },
	{ -1.5   ,  -0.84  ,  0.15    },
	{ -1.5   ,   0.0   ,  0.15    },
	{ -2.0   ,   1.12  ,  0.45    },
	{ -1.12  ,   2.0   ,  0.45    },
	{  0.0   ,   2.0   ,  0.45    },
	{ -1.5   ,   0.84  ,  0.225   },
	{ -0.84  ,   1.5   ,  0.225   },
	// 111
	{  0.0   ,   1.5   ,  0.225   },
	{ -1.5   ,   0.84  ,  0.15    },
	{ -0.84  ,   1.5   ,  0.15    },
	{  0.0   ,   1.5   ,  0.15    },
	{  1.12  ,   2.0   ,  0.45    },
	{  2.0   ,   1.12  ,  0.45    },
	{  0.84  ,   1.5   ,  0.225   },
	{  1.5   ,   0.84  ,  0.225   },
	{  0.84  ,   1.5   ,  0.15    },
	{  1.5   ,   0.84  ,  0.15    },
	// 121
	{ -1.6   ,   0.0   ,  2.025   },
	{ -1.6   ,  -0.3   ,  2.025   },
	{ -1.5   ,  -0.3   ,  2.25    },
	{ -1.5   ,   0.0   ,  2.25    },
	{ -2.3   ,   0.0   ,  2.025   },
	{ -2.3   ,  -0.3   ,  2.025   },
	{ -2.5   ,  -0.3   ,  2.25    },
	{ -2.5   ,   0.0   ,  2.25    },
	{ -2.7   ,   0.0   ,  2.025   },
	{ -2.7   ,  -0.3   ,  2.025   },
	// 131
	{ -3.0   ,  -0.3   ,  2.25    },
	{ -3.0   ,   0.0   ,  2.25    },
	{ -2.7   ,   0.0   ,  1.8     },
	{ -2.7   ,  -0.3   ,  1.8     },
	{ -3.0   ,  -0.3   ,  1.8     },
	{ -3.0   ,   0.0   ,  1.8     },
	{ -1.5   ,   0.3   ,  2.25    },
	{ -1.6   ,   0.3   ,  2.025   },
	{ -2.5   ,   0.3   ,  2.25    },
	{ -2.3   ,   0.3   ,  2.025   },
	// 141
	{ -3.0   ,   0.3   ,  2.25    },
	{ -2.7   ,   0.3   ,  2.025   },
	{ -3.0   ,   0.3   ,  1.8     },
	{ -2.7   ,   0.3   ,  1.8     },
	{ -2.7   ,   0.0   ,  1.575   },
	{ -2.7   ,  -0.3   ,  1.575   },
	{ -3.0   ,  -0.3   ,  1.35    },
	{ -3.0   ,   0.0   ,  1.35    },
	{ -2.5   ,   0.0   ,  1.125   },
	{ -2.5   ,  -0.3   ,  1.125   },
	// 151
	{ -2.65  ,  -0.3   ,  0.9375  },
	{ -2.65  ,   0.0   ,  0.9375  },
	{ -2.0   ,  -0.3   ,  0.9     },
	{ -1.9   ,  -0.3   ,  0.6     },
	{ -1.9   ,   0.0   ,  0.6     },
	{ -3.0   ,   0.3   ,  1.35    },
	{ -2.7   ,   0.3   ,  1.575   },
	{ -2.65  ,   0.3   ,  0.9375  },
	{ -2.5   ,   0.3   ,  1.1255  },
	{ -1.9   ,   0.3   ,  0.6     },
	// 161
	{ -2.0   ,   0.3   ,  0.9     },
	{  1.7   ,   0.0   ,  1.425   },
	{  1.7   ,  -0.66  ,  1.425   },
	{  1.7   ,  -0.66  ,  0.6     },
	{  1.7   ,   0.0   ,  0.6     },
	{  2.6   ,   0.0   ,  1.425   },
	{  2.6   ,  -0.66  ,  1.425   },
	{  3.1   ,  -0.66  ,  0.825   },
	{  3.1   ,   0.0   ,  0.825   },
	{  2.3   ,   0.0   ,  2.1     },
	// 171
	{  2.3   ,  -0.25  ,  2.1     },
	{  2.4   ,  -0.25  ,  2.025   },
	{  2.4   ,   0.0   ,  2.025   },
	{  2.7   ,   0.0   ,  2.4     },
	{  2.7   ,  -0.25  ,  2.4     },
	{  3.3   ,  -0.25  ,  2.4     },
	{  3.3   ,   0.0   ,  2.4     },
	{  1.7   ,   0.66  ,  0.6     },
	{  1.7   ,   0.66  ,  1.425   },
	{  3.1   ,   0.66  ,  0.825   },
	// 181
	{  2.6   ,   0.66  ,  1.425   },
	{  2.4   ,   0.25  ,  2.025   },
	{  2.3   ,   0.25  ,  2.1     },
	{  3.3   ,   0.25  ,  2.4     },
	{  2.7   ,   0.25  ,  2.4     },
	{  2.8   ,   0.0   ,  2.475   },
	{  2.8   ,  -0.25  ,  2.475   },
	{  3.525 ,  -0.25  ,  2.49375 },
	{  3.525 ,   0.0   ,  2.49375 },
	{  2.9   ,   0.0   ,  2.475   },
	// 191
	{  2.9   ,  -0.15  ,  2.475   },
	{  3.45  ,  -0.15  ,  2.5125  },
	{  3.45  ,   0.0   ,  2.5125  },
	{  2.8   ,   0.0   ,  2.4     },
	{  2.8   ,  -0.15  ,  2.4     },
	{  3.2   ,  -0.15  ,  2.4     },
	{  3.2   ,   0.0   ,  2.4     },
	{  3.525 ,   0.25  ,  2.49375 },
	{  2.8   ,   0.25  ,  2.475   },
	{  3.45  ,   0.15  ,  2.5125  },
	// 201
	{  2.9   ,   0.15  ,  2.475   },
	{  3.2   ,   0.15  ,  2.4     },
	{  2.8   ,   0.15  ,  2.4     },
	{  0.0   ,   0.0   ,  3.15    },
	{  0.0   ,  -0.002 ,  3.15    },
	{  0.002 ,   0.0   ,  3.15    },
	{  0.8   ,   0.0   ,  3.15    },
	{  0.8   ,  -0.45  ,  3.15    },
	{  0.45  ,  -0.8   ,  3.15    },
	{  0.0   ,  -0.8   ,  3.15    },
	// 211
	{  0.0   ,   0.0   ,  2.85    },
	{  0.2   ,   0.0   ,  2.7     },
	{  0.2   ,  -0.112 ,  2.7     },
	{  0.112 ,  -0.2   ,  2.7     },
	{  0.0   ,  -0.2   ,  2.7     },
	{ -0.002 ,   0.0   ,  3.15    },
	{ -0.45  ,  -0.8   ,  3.15    },
	{ -0.8   ,  -0.45  ,  3.15    },
	{ -0.8   ,   0.0   ,  3.15    },
	{ -0.112 ,  -0.2   ,  2.7     },
	// 221
	{ -0.2   ,  -0.112 ,  2.7     },
	{ -0.2   ,   0.0   ,  2.7     },
	{  0.0   ,   0.002 ,  3.15    },
	{ -0.8   ,   0.45  ,  3.15    },
	{ -0.45  ,   0.8   ,  3.15    },
	{  0.0   ,   0.8   ,  3.15    },
	{ -0.2   ,   0.112 ,  2.7     },
	{ -0.112 ,   0.2   ,  2.7     },
	{  0.0   ,   0.2   ,  2.7     },
	{  0.45  ,   0.8   ,  3.15    },
	// 231
	{  0.8   ,   0.45  ,  3.15    },
	{  0.112 ,   0.2   ,  2.7     },
	{  0.2   ,   0.112 ,  2.7     },
	{  0.4   ,   0.0   ,  2.55    },
	{  0.4   ,  -0.224 ,  2.55    },
	{  0.224 ,  -0.4   ,  2.55    },
	{  0.0   ,  -0.4   ,  2.55    },
	{  1.3   ,   0.0   ,  2.55    },
	{  1.3   ,  -0.728 ,  2.55    },
	{  0.728 ,  -1.3   ,  2.55    },
	// 241
	{  0.0   ,  -1.3   ,  2.55    },
	{  1.3   ,   0.0   ,  2.4     },
	{  1.3   ,  -0.728 ,  2.4     },
	{  0.728 ,  -1.3   ,  2.4     },
	{  0.0   ,  -1.3   ,  2.4     },
	{ -0.224 ,  -0.4   ,  2.55    },
	{ -0.4   ,  -0.224 ,  2.55    },
	{ -0.4   ,   0.0   ,  2.55    },
	{ -0.728 ,  -1.3   ,  2.55    },
	{ -1.3   ,  -0.728 ,  2.55    },
	// 251
	{ -1.3   ,   0.0   ,  2.55    },
	{ -0.728 ,  -1.3   ,  2.4     },
	{ -1.3   ,  -0.728 ,  2.4     },
	{ -1.3   ,   0.0   ,  2.4     },
	{ -0.4   ,   0.224 ,  2.55    },
	{ -0.224 ,   0.4   ,  2.55    },
	{  0.0   ,   0.4   ,  2.55    },
	{ -1.3   ,   0.728 ,  2.55    },
	{ -0.728 ,   1.3   ,  2.55    },
	{  0.0   ,   1.3   ,  2.55    },
	// 261
	{ -1.3   ,   0.728 ,  2.4     },
	{ -0.728 ,   1.3   ,  2.4     },
	{  0.0   ,   1.3   ,  2.4     },
	{  0.224 ,   0.4   ,  2.55    },
	{  0.4   ,   0.224 ,  2.55    },
	{  0.728 ,   1.3   ,  2.55    },
	{  1.3   ,   0.728 ,  2.55    },
	{  0.728 ,   1.3   ,  2.4     },
	{  1.3   ,   0.728 ,  2.4     },
};

#define RESU 10
#define RESV 10
struct vertex teapot_vertices[TEAPOT_NB_PATCHES * RESU*RESV];
GLfloat teapot_colors[TEAPOT_NB_PATCHES * RESU*RESV * 3];
GLushort teapot_elements[TEAPOT_NB_PATCHES * (RESU - 1)*(RESV - 1) * 2 * 3];
GLfloat teapot_cp_colors[269 * 3];
GLushort teapot_cp_elements[TEAPOT_NB_PATCHES][ORDER + 1][ORDER + 1];

void build_control_points_k(int p, struct vertex control_points_k[][ORDER + 1]) {
	for (int i = 0; i <= ORDER; i++)
		for (int j = 0; j <= ORDER; j++)
			control_points_k[i][j] = teapot_cp_vertices[teapot_patches[p][i][j] - 1];
}

int factorial(int n) {
	assert(n >= 0);
	int result = 1;
	for (int i = n; i > 1; i--)
		result *= i;
	return result;
}

float binomial_coefficient(int i, int n) {
	assert(i >= 0); assert(n >= 0);
	return 1.0f * factorial(n) / (factorial(i) * factorial(n - i));
}

float bernstein_polynomial(int i, int n, float u) {
	return binomial_coefficient(i, n) * powf(u, i) * powf(1 - u, n - i);
}

struct vertex compute_position(struct vertex control_points_k[][ORDER + 1], float u, float v) {
	struct vertex result = { 0.0, 0.0, 0.0 };
	for (int i = 0; i <= ORDER; i++) {
		for (int j = 0; j <= ORDER; j++) {
			float poly_i = bernstein_polynomial(i, ORDER, u);
			float poly_j = bernstein_polynomial(j, ORDER, v);
			result.x += poly_i * poly_j * control_points_k[i][j].x;
			result.y += poly_i * poly_j * control_points_k[i][j].y;
			result.z += poly_i * poly_j * control_points_k[i][j].z;
		}
	}
	return result;
}

void build_teapot() {
	// Vertices
	for (int p = 0; p < TEAPOT_NB_PATCHES; p++) {
		struct vertex control_points_k[ORDER + 1][ORDER + 1];
		build_control_points_k(p, control_points_k);
		for (int ru = 0; ru <= RESU - 1; ru++) {
			float u = 1.0 * ru / (RESU - 1);
			for (int rv = 0; rv <= RESV - 1; rv++) {
				float v = 1.0 * rv / (RESV - 1);
				teapot_vertices[p*RESU*RESV + ru * RESV + rv] = compute_position(control_points_k, u, v);
				teapot_colors[p*RESU*RESV * 3 + ru * RESV * 3 + rv * 3 + 0] = 1.0 * p / TEAPOT_NB_PATCHES;
				teapot_colors[p*RESU*RESV * 3 + ru * RESV * 3 + rv * 3 + 1] = 1.0 * p / TEAPOT_NB_PATCHES;
				teapot_colors[p*RESU*RESV * 3 + ru * RESV * 3 + rv * 3 + 2] = 0.8;
			}
		}
	}

	// Elements
	int n = 0;
	for (int p = 0; p < TEAPOT_NB_PATCHES; p++){
		for (int ru = 0; ru < RESU - 1; ru++){
			for (int rv = 0; rv < RESV - 1; rv++) {
				// 1 square ABCD = 2 triangles ABC + CDA
				// ABC
				teapot_elements[n] = p * RESU*RESV + ru * RESV + rv; n++;
				teapot_elements[n] = p * RESU*RESV + ru * RESV + (rv + 1); n++;
				teapot_elements[n] = p * RESU*RESV + (ru + 1)*RESV + (rv + 1); n++;
				// CDA
				teapot_elements[n] = p * RESU*RESV + (ru + 1)*RESV + (rv + 1); n++;
				teapot_elements[n] = p * RESU*RESV + (ru + 1)*RESV + rv; n++;
				teapot_elements[n] = p * RESU*RESV + ru * RESV + rv; n++;
			}
		}
	}
	// Control points elements for debugging
	//memset(teapot_cp_colors, 0, sizeof(teapot_cp_colors)); // black
	for (int p = 0; p < TEAPOT_NB_PATCHES; p++){
		for (int i = 0; i < (ORDER + 1); i++){
			for (int j = 0; j < (ORDER + 1); j++){
				teapot_cp_elements[p][i][j] = teapot_patches[p][i][j] - 1;
			}
		}
	}
}

static bool m_isClosed = false;


void LoadObjects(unsigned int *VBO,unsigned int *VAO, float vertices[],int sizeOfVertices)
{
	glGenBuffers(1, VBO);
	glGenVertexArrays(1, VAO);
	glBindVertexArray(*VAO);
	glBindBuffer(GL_ARRAY_BUFFER, *VBO);
	glBufferData(GL_ARRAY_BUFFER, sizeOfVertices, vertices, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
	glEnableVertexAttribArray(0);
}

namespace editor
{
	App::App(){
		this->texture = NULL;
		this->rotate_degree = 0;
		this->lastX = (float)SCR_WIDTH / 2.0;
		this->lastY = (float)SCR_HEIGHT / 2.0;
		this->firstMouse = true;
		this->quit = true;
		memset(&this->PillarVerticesArray[0], 0, sizeof(this->PillarVerticesArray));
		memset(&this->PillarVerticesArray1[0], 0, sizeof(this->PillarVerticesArray1));
		memset(&this->SolidConeVerticesArray[0], 0, sizeof(this->SolidConeVerticesArray));
	}

	App::~App(){
		if(NULL != this->texture)
			SDL_DestroyTexture(this->texture);
		if(NULL != this->appRenderer)
			SDL_DestroyRenderer(this->appRenderer->getSDL_Renderer());
	}

	bool App::init(){
		// Initialise SDL.
		//this->sdl_content = sdl::SdlContext(0 == SDL_Init(SDL_INIT_EVERYTHING));
		//SDL_INIT_VIDEO

		//SDL_WINDOWPOS_UNDEFINED,
		//SDL_WINDOWPOS_UNDEFINED,
		this->appWindow = Window::create("Editor",
			SDL_WINDOWPOS_CENTERED, 
			SDL_WINDOWPOS_CENTERED,
			1280, 720, SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
		
		// Create a window. 640 360
		/*window = sdl::WindowPtr(SDL_CreateWindow(
		"Editor",
		SDL_WINDOWPOS_UNDEFINED,
		SDL_WINDOWPOS_UNDEFINED,
		640,
		360,
		SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL),
		SDL_DestroyWindow);*/
		//window = sdl::WindowPtr(appWindow);
		/*if (!window)
		{
		   return false;
		}*/
		if (!appWindow){
			return false;
		}
		g_textureID = -1;

		char *prebase_path = SDL_GetPrefPath("My Company", "My Awesome SDL 2 Game");
		char *base_path =  SDL_GetBasePath();//"C:\\workspace\\SDLOpenGL\\";
		if (base_path) {
			this->data_path = "C:\\workspace\\SDLOpenGL\\";
		}
		else {
			data_path = SDL_strdup("./");
		}

		// Enable drag and drop.
		SDL_EventState(SDL_DROPFILE, SDL_ENABLE);

		this->appRenderer = Renderer::create(this->appWindow, -1,
			SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
		this->sdlRect.x = 5;
		this->sdlRect.y = 5;
		
		Size windowSize = this->appWindow->getRealWindowSize();
		int window_w = windowSize.width;
		int window_h = windowSize.height;
		this->sdlRect.w = window_w - 10;// fullscreen_mode.w;
		this->sdlRect.h = window_h - 10;// fullscreen_mode.h;


		//SDL_DisplayMode fullscreen_mode;
		//https://www.gamedev.net/articles/programming/engines-and-middleware/direct3d-90-with-sdl-r2249
		//https://www.cnblogs.com/tocy/p/sdl-screen-info-visible-region.html
		//http://wiki.libsdl.org/SDL_GetWindowSize
		//if (0 == SDL_GetWindowDisplayMode(this->appWindow, &fullscreen_mode)) {
			//this->sdlRect.w = fullscreen_mode.w;
			//this->sdlRect.h = fullscreen_mode.h;}
		//SDL_SetRenderDrawColor(this->appRenderer->getSDL_Renderer(),
			//255, 0, 0, 255);
		//SDL_RenderClear(this->appRenderer->getSDL_Renderer());

		//SDL_SetRenderDrawColor(this->render, (Uint8)255, (Uint8)255, (Uint8)255, 0xff);
		//SDL_RenderClear(this->render);

		//SDL_SetRenderDrawColor(this->render, 0x00, 0x00, 0x00, 0xff);
		//SDL_Rect fillRect = { 0, 0, screenWidth, screenHeight };
		//SDL_RenderFillRect(this->render, &fillRect);
		
		//std::string bgImgPath = std::string("C:\\workspace\\newGousheng\\") + "resources" + "\\" + "bg01.bmp";
		//loadTexture(bgImgPath.c_str());
		//SDL_Surface *background = LoadIMG("player1.png");
		//SDL_CreateTextureFromSurface(this->render, background);



		this->sdlRect.x = window_w * 0.5 - 5;
		this->sdlRect.y = window_h * 0.5 - 5;
		this->sdlRect.w = (window_w - 10) * 0.125;
		this->sdlRect.h = (window_h - 10) * 0.125;
		//this->texture;
		//std::string filePath = std::string("C:\\workspace\\newGousheng\\") + "resources" + "\\" + "player1.png";
		//this->appTexture = this->appRenderer->imgLoadTexture("C:\\workspace\\SDLOpenGL\\resources\\Faerie.png");

		int pitch, w, h, access;
		unsigned int format = 0;//unsigned char* pixels;
		//SDL_QueryTexture(this->appTexture->getSDL_Texture(), &format, &access, &w, &h);
		//this->sdlRect.w = w;
		//this->sdlRect.h = h;
		//SDL_RenderCopy(this->appRenderer->getSDL_Renderer(), this->appTexture->getSDL_Texture(),
			//NULL, &(this->sdlRect));
		// Initialise the engine.
		if (!engine.init()){
			return false;
		}
		//this->drawByPixels();

		this->game_ptr = game::Game::create();
		this->game_ptr->Init(window_w, window_h,this->appRenderer,base_path);

		//prepareGeo(base_path);

		this->init_now_time = time(NULL);

		glEnable(GL_DEPTH_TEST);
		glEnable(GL_TEXTURE_2D);

		return true;
	}

	void App::drawByPixels(){
		Image pngData;
		pngData.nWidth = 0;
		pngData.nHeight = 0;
		pngData.imgData = nullptr;
		//loadpng("res/mingyuexin.png",&pngData.nWidth,&pngData.nHeight,&pngData.imgData);
		this->loadpng("player1.png", &pngData.nWidth, &pngData.nHeight, &pngData.imgData);
		
		int screenWidth = pngData.nWidth * SCALE_FACTOR;
		int screenHeight = pngData.nHeight * SCALE_FACTOR;

		int rr = (Uint8)pngData.getRAt(0, 0);
		int gg = (Uint8)pngData.getGAt(0, 0);
		int bb = (Uint8)pngData.getBAt(0, 0);
		for (int row = 0; row < screenHeight; row++){
			for (int col = 0; col < screenWidth; col++){
				int x = col / SCALE_FACTOR;
				int y = row / SCALE_FACTOR;
				Uint8 r, g, b, a;
				//0 == row || row == (screenWidth - 1) || 0 == col || col == (screenHeight - 1)
				int rr1 = (Uint8)pngData.getRAt(x, y);
				int gg1 = (Uint8)pngData.getGAt(x, y);
				int bb1 = (Uint8)pngData.getBAt(x, y);
				if (rr1 == 255 && gg == 0 && bb == 255)
				{
					r = 248;
					g = 248;
					b = 255;
					a = 1;
				}
				else {
					r = (Uint8)pngData.getRAt(x, y);
					g = (Uint8)pngData.getGAt(x, y);
					b = (Uint8)pngData.getBAt(x, y);
					a = (Uint8)pngData.getAAt(x, y);
				}
				SDL_SetRenderDrawBlendMode(this->appRenderer->getSDL_Renderer(), SDL_BLENDMODE_BLEND);
				SDL_SetRenderDrawColor(this->appRenderer->getSDL_Renderer(), r, g, b, a);
				SDL_RenderDrawPoint(this->appRenderer->getSDL_Renderer(), this->sdlRect.x - this->sdlRect.w + col, this->sdlRect.y + row);
			}
		}
	}

	bool App::run_frame()
	{
		if (!handle_events()){
			return false;
		}
		draw();
		//SDL_GL_SwapWindow(appWindow);
		//if (SDL_GetTicks() - _FPS_Timer < FPS)
		//{
		//	//SDL_Delay(FPS - SDL_GetTicks() + _FPS_Timer);
		//}
		_FPS_Timer = SDL_GetTicks();
		float currentFrame = _FPS_Timer;
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		//SDL_Delay(20);
		return engine.update();
	}

	bool App::handle_events()
	{
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0){
			if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE) {
				//isClosed = true;
			}
			if (e.key.keysym.scancode == SDL_SCANCODE_1){
				this->game_ptr->activeCamera(1);
			}
			if (e.key.keysym.scancode == SDL_SCANCODE_2){
				this->game_ptr->activeCamera(2);
			}
			if (!handle_event(e)){
				return false;
			}
			if (e.key.keysym.scancode == SDL_SCANCODE_ESCAPE){
				SDL_Quit();
				return false;
			}
			if (SDL_KEYDOWN == e.type){
				//processInput(e.key);
				//keycodes[event.key.keysym.scancode] = 1;
				this->game_ptr->setGameKeyCodes(e.key.keysym.scancode, 1);
			}
			if (SDL_KEYUP == e.type){
				//keycodes[event.key.keysym.scancode] = 0;
				this->game_ptr->setGameKeyCodes(e.key.keysym.scancode, 0);
			}
			if(SDL_MOUSEWHEEL == e.type){
				SDL_MouseWheelEvent wheel = e.wheel;
				int y = wheel.direction;
				if(y > 0){
					int b = 1;
					b += 2;
				}
				else{
					int a = 0;
					a += 1;
				}
			}
		}
		return this->quit;
	}

	bool App::handle_event(const SDL_Event &e)
	{
		int eType = e.type;
		switch (eType)
		{
		case SDL_DROPFILE:
		{
			// Take ownership of the buffer.
			//const sdl::BufferPtr path(e.drop.file);
			handle_drop(e.drop.file);//path.get()
		}
		break;

		case SDL_QUIT:
		{
			SDL_Quit();
		}
			return false;

		case SDL_WINDOWEVENT:
			handle_window_event(e.window);
			break;
		case SDL_MOUSEMOTION:
			handle_mouse_event(e.motion);
			break;
		case SDL_MOUSEWHEEL:
			handle_scroll_event(e.wheel);
			break;
		case SDL_MOUSEBUTTONDOWN:
			processMouseButton(e.button);
			break;
		default:
			break;
		}
		
		return true;
	}

	void App::handle_drop(const char *path)
	{

	}

	void App::handle_window_event(const SDL_WindowEvent &e)
	{
		switch (e.event)
		{
		case SDL_WINDOWEVENT_SIZE_CHANGED:
		{
			const int w = e.data1;
			const int h = e.data2;
			glViewport(0, 0, w, h);
			glScissor(0, 0, w, h);
		}
		break;
		}
	}

	void App::processMouseButton(const SDL_MouseButtonEvent &event)
	{
		/*if (SDL_BUTTON_LEFT == event.button) {

		}
		else(SDL_BUTTON_RIGHT == event.button) {

		}*/
	}

	void App::processInput(const SDL_KeyboardEvent &event)
	{
		//SDL_KEYDOWN
		//float deltaTime = event.timestamp;
		int sym = event.keysym.sym;
		if (SDLK_w == sym) {
			this->game_ptr->gameProcessKeyboard(FORWARD, deltaTime);
		}
		else if(SDLK_s == sym) {
			this->game_ptr->gameProcessKeyboard(BACKWARD, deltaTime);
		}
		else if(SDLK_a == sym) {
			this->game_ptr->gameProcessKeyboard(LEFT, deltaTime);
		}
		else if(SDLK_d == sym) {
			this->game_ptr->gameProcessKeyboard(RIGHT, deltaTime);
		}
		else if (SDLK_k == sym) {
			this->game_ptr->gameProcessKeyboard(FORWARD, deltaTime);
		}
		else if (SDLK_UP == sym) {
			
		}
		else if (SDLK_DOWN == sym) {
			
		}
		else if (SDLK_LEFT == sym) {

		}
		else if (SDLK_RIGHT == sym) {

		}
		else if (SDLK_ESCAPE == sym) {
			SDL_Quit();
			this->quit = false;
		}
	}

	void App::handle_mouse_event(const SDL_MouseMotionEvent &motion)
	{
		int xpos = motion.x;
		int ypos = motion.y;
		if (firstMouse)
		{
			lastX = xpos;
			lastY = ypos;
			firstMouse = false;
		}

		float xoffset = xpos - lastX;
		float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

		lastX = xpos;
		lastY = ypos;

		this->game_ptr->gameProcessMouseMovement(xoffset, yoffset);
	}

	void App::handle_scroll_event(const SDL_MouseWheelEvent &event)
	{
		double xoffset;
		double yoffset = event.y;
		//appcamera.ProcessMouseScroll(yoffset);
	}

	void App::draw()
	{
		this->appWindow->setBgClear(1.0f, 1.0f, 1.0f, 1.0f);
		glEnable(GL_DEPTH_TEST);
		glDepthFunc(GL_LESS);
		// Draw the engine.
		engine.draw();


		//SDL_SetRenderDrawBlendMode(this->appRenderer->getSDL_Renderer(), SDL_BLENDMODE_BLEND);
		//SDL_RenderPresent(this->appRenderer->getSDL_Renderer());

		float angle = SDL_GetTicks() * glm::radians(0.05);
		//SDL_LogDebug(SDL_LOG_CATEGORY_INPUT, "angle=%f\n", angle);
	    //OutputDebugPrintf("angle=%f\n", angle);
		//glutGet(GLUT_ELAPSED_TIME) / 1000.0 * glm::radians(15.0);
		
		glm::mat4 view1 = glm::lookAt(cameraPos, cameraPos + cameraFront, cameraUp);
		glm::mat4 view = appcamera.GetViewMatrix();
		glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		
		this->game_ptr->Render();

		//glDepthMask(GL_FALSE);
		//glDepthFunc(GL_LEQUAL);
		//glm::mat4 skyview = glm::mat4(glm::mat3(camera.GetViewMatrix()));
		//// glDepthFunc(GL_LESS);
		//this->drawTriangle(projection, skyview);
		//glDepthMask(GL_TRUE);

		//this->drawGround(projection, view1);
		//glEnable(GL_CULL_FACE);
		//glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_SHORT, 0);
		//glm::lookAt(glm::vec3(0.0, 0.0, 8.0), glm::vec3(0.0, 0.0, 2.0), glm::vec3(0.0, 1.0, 0.0));
		//this->DrawSand(projection, view,angle);
		//OutputDebugPrintf("this->rotate_degree=%f\n", this->rotate_degree);
		
		//this->drawGeo(projection, view1);

		//this->drawTeapot(angle, view);

		// glUseProgram(reconstructColorProgram);
		// glBindVertexArray(reconstructColorVAO);
		// glm::mat4 anim =
		// 	glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 0.5)) *
		// 	glm::scale(glm::mat4(1.0), glm::vec3(20, 20, 20));
		// glm::mat4 model = glm::mat4(1.0f);
		// glm::mat4 projectionTeapot = glm::perspective(glm::radians(45.0f), 1.0f * 800 / 600, 0.1f, 100.0f);
		// glm::mat4 mvp = projectionTeapot * view * model * anim;
		// GLuint reconstructColor_transformLoc = glGetUniformLocation(reconstructColorProgram, "transform");
		// glUniformMatrix4fv(reconstructColor_transformLoc, 1, GL_FALSE, glm::value_ptr(mvp));
		// reconstructColor_transformLoc = glGetUniformLocation(reconstructColorProgram, "view");
		// glUniformMatrix4fv(reconstructColor_transformLoc, 1, GL_FALSE, glm::value_ptr(view));
		// reconstructColor_transformLoc = glGetUniformLocation(triangleShaderProgram, "projection");
		// glUniformMatrix4fv(reconstructColor_transformLoc, 1, GL_FALSE, glm::value_ptr(projection));
		// glDrawElements(GL_TRIANGLES, fset[0].size(), GL_UNSIGNED_SHORT, 0);
		
		//size_t offset = 0;  // size_t instead of GLushort to fix 'warning: cast to pointer from integer of different size [-Wint-to-pointer-cast]'
		//for (int p = 0; p < TEAPOT_NB_PATCHES; p++)
		//	for (int i = 0; i < ORDER + 1; i++, offset += (ORDER + 1) * sizeof(GLushort))
		//		glDrawElements(GL_LINE_LOOP, ORDER + 1, GL_UNSIGNED_SHORT, (GLvoid*)offset);

		this->appWindow->SwapBuffers();
		this->rotate_degree += 1;
		
		if (this->rotate_degree - 360 > 0)
			this->rotate_degree = 0;
	}

	void App::loadTexture(const char *fileStr)
	{
		//glGenTextures(1,&g_textureID);
		//glBindTexture(GL_TEXTURE_2D,g_textureID);
		//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
		//glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_LINEAR);
		//std::string filePath = 
		//std::string(this->data_path) + "resources" + "\\" + fileStr;
		//SDL_Surface *background = SDL_LoadBMP(filePath.c_str());
		//SDL_Surface *picturenew = SDL_ConvertSurfaceFormat(background, SDL_PIXELFORMAT_ABGR8888, 0);
		//gluBuild2DMipmaps(GL_TEXTURE_2D, GL_RGB, picturenew->w, picturenew->h, GL_RGB, GL_UNSIGNED_BYTE, picturenew->pixels);
		//SDL_GL_SetSwapInterval(1);
		//SDL_BlitSurface(background, 0, picturenew, 0);
		this->texture = IMG_LoadTexture(this->appRenderer->getSDL_Renderer(), fileStr);
		//SDL_CreateTextureFromSurface(this->render, background);
		SDL_RenderCopy(this->appRenderer->getSDL_Renderer(), texture, NULL, &(this->sdlRect));
	}

	void App::prepareGeo(const char *base_path){
		Shader shader;
		triangleShaderProgram = shader.LoadShaders(triangleVertexShaderSource, triangleFragmentShaderSource);
		//LoadObjects(&triangleVBO, &triangleVAO, vertices, sizeof(vertices));
		glGenVertexArrays(1, &triangleVAO);
		glGenBuffers(1, &triangleVBO);
		glGenBuffers(1, &triangleVEO);
		glBindVertexArray(triangleVAO);
		glBindBuffer(GL_ARRAY_BUFFER, triangleVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, triangleVEO);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
		// position attribute
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(0);
		// color attribute
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(1);
		// texture coord attribute
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glEnableVertexAttribArray(2);

		GameImgLoader gameImgLoader;
		gameImgLoader.loadTextureData(&triangleTexture, FileSystem::getPath(base_path, "resources/bgPic/oFront.bmp").c_str());

		int pillarVCount = 0;
		for (int i = 0; i <= 780; i += 30){
			float p = 0.5 * (float)(i * 3.141593 / 180);
			//glVertex3f((float)sin(p) / 2, (float)cos(p) / 2, 1.0f);
			this->PillarVerticesArray[pillarVCount++] = (float)cos(p) / 8;
			this->PillarVerticesArray[pillarVCount++] = (float)sin(p) / 8;
			this->PillarVerticesArray[pillarVCount++] = 1.0;
			//glVertex3f((float)sin(p) / 2, (float)cos(p) / 2, 0.0f);
			this->PillarVerticesArray[pillarVCount++] = (float)cos(p) / 4;
			this->PillarVerticesArray[pillarVCount++] = (float)sin(p) / 4;
			this->PillarVerticesArray[pillarVCount++] = 0.9;
		}
		pillarVCount = 0;
		for (int i = 0; i <= 780; i += 30){
			float p = 0.5 * (float)(i * 3.141593 / 180);
			this->PillarVerticesArray1[pillarVCount++] = (float)cos(p) / 4;
			this->PillarVerticesArray1[pillarVCount++] = (float)sin(p) / 4;
			this->PillarVerticesArray1[pillarVCount++] = 1.0;
			this->PillarVerticesArray1[pillarVCount++] = (float)cos(p) / 4.3;
			this->PillarVerticesArray1[pillarVCount++] = (float)sin(p) / 4.3;
			this->PillarVerticesArray1[pillarVCount++] = 0.9;
		}

		pillarShaderProgram = shader.LoadShaders(pillarVertexShaderSource, pillarFragmentShaderSource);
		LoadObjects(&pillarVBO, &pillarVAO, &this->PillarVerticesArray[0], sizeof(this->PillarVerticesArray));

		pillarShaderProgram1 = shader.LoadShaders(pillarVertexShaderSource, pillarFragmentShaderSource);
		LoadObjects(&pillar1VBO, &pillar1VAO, &this->PillarVerticesArray1[0], sizeof(this->PillarVerticesArray1));

		build_teapot();
		teapot_ShaderProgram = shader.LoadShaders(teapotVertexShaderSource, teapotFragmentShaderSource);
		glGenVertexArrays(1, &vao_teapot);
		glGenBuffers(1, &vbo_teapot_vertices);
		glGenBuffers(1, &ibo_teapot_elements);
		glGenBuffers(1, &ibo_teapot_cp_elements);
		glBindVertexArray(vao_teapot);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_teapot_vertices);
		glBufferData(GL_ARRAY_BUFFER, sizeof(teapot_vertices), teapot_vertices, GL_STATIC_DRAW);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);

		glGenBuffers(1, &vbo_teapot_colors);
		glBindBuffer(GL_ARRAY_BUFFER, vbo_teapot_colors);
		glBufferData(GL_ARRAY_BUFFER, sizeof(teapot_colors), teapot_colors, GL_STATIC_DRAW);
		glVertexAttribPointer(
			1, // attribute
			3,                 // number of elements per vertex, here (x,y,z)
			GL_FLOAT,          // the type of each element
			GL_FALSE,          // take our values as-is
			0,                 // no extra data between each position
			0                  // offset of first element
		);
		glEnableVertexAttribArray(1);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_teapot_elements);
		glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(teapot_elements), teapot_elements, GL_STATIC_DRAW);

		//glGenBuffers(1, &vbo_teapot_cp_vertices);
		//glBindBuffer(GL_ARRAY_BUFFER, vbo_teapot_cp_vertices);
		//glBufferData(GL_ARRAY_BUFFER, sizeof(teapot_cp_vertices), teapot_cp_vertices, GL_STATIC_DRAW);
		//glVertexAttribPointer(
		//	0, // attribute
		//	3,                 // number of elements per vertex, here (x,y,z)
		//	GL_FLOAT,          // the type of each element
		//	GL_FALSE,          // take our values as-is
		//	0,                 // no extra data between each position
		//	0                  // offset of first element
		//);

		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_teapot_cp_elements);
		//glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(teapot_cp_elements), teapot_cp_elements, GL_STATIC_DRAW);


		pillarVCount = 0;
		for (int i = 0; i <= 780; i += 30){
			float p = 0.5 * (float)(i * 3.1415926 / 180);
			//glVertex3f((float)sin(p) / 2, (float)cos(p) / 2, 1.0f);
			this->SolidConeVerticesArray[pillarVCount++] = (float)cos(p) / 3.1415926;
			this->SolidConeVerticesArray[pillarVCount++] = (float)sin(p) / 3.1415926;
			this->SolidConeVerticesArray[pillarVCount++] = 3.0;
			//glVertex3f((float)sin(p) / 2, (float)cos(p) / 2, 0.0f);
			this->SolidConeVerticesArray[pillarVCount++] = 0.0;
			this->SolidConeVerticesArray[pillarVCount++] = 0.0;
			this->SolidConeVerticesArray[pillarVCount++] = -20.5;
		}

		SolidConeShaderProgram = shader.LoadShaders(pillarVertexShaderSource, pillarFragmentShaderSource);
		LoadObjects(&SolidConeVBO, &SolidConeVAO, &this->SolidConeVerticesArray[0], sizeof(this->SolidConeVerticesArray));

		pointShaderProgram = shader.LoadShaders(pointVertexShaderSource, pointFragmentShaderSource);
		LoadObjects(&pointVBO, &pointVAO, pointVertices, sizeof(pointVertices));

		EsquareShaderProgram = shader.LoadShaders(triangleVertexShaderSource, triangleFragmentShaderSource);
		LoadObjects(&EsquareVBO, &EsquareVAO, Esquare, sizeof(Esquare));
	}

	void App::drawTriangle(glm::mat4 projection, glm::mat4 view){
		glBindTexture(GL_TEXTURE_2D, triangleTexture);
		glUseProgram(triangleShaderProgram);
		glBindVertexArray(triangleVAO);
		//glDrawArrays(GL_TRIANGLES, 0, 3);
		glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
		glm::mat4 trans = glm::mat4(1.0f);
		trans = glm::translate(trans, glm::vec3(0.0f, 20.0f, -80));
		trans = glm::rotate(trans, glm::radians(180.0f), glm::vec3(1.0, 0.0, 0.0));
		trans = glm::scale(trans, glm::vec3(42.2, 3.0 * 42.2 / 4.0, 1.0));
		GLuint transformLoc = glGetUniformLocation(triangleShaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
		transformLoc = glGetUniformLocation(triangleShaderProgram, "view");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));
		transformLoc = glGetUniformLocation(triangleShaderProgram, "projection");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));
	}

	void App::drawGeo(glm::mat4 projection, glm::mat4 view)
	{
		glUseProgram(pointShaderProgram);
		glBindVertexArray(pointVAO);
		glDrawArrays(GL_POINTS, 0, 3);

		glUseProgram(pillarShaderProgram);
		glBindVertexArray(pillarVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 50);
		glm::mat4 trans = glm::mat4(1.0f);
		//trans = glm::translate(trans, glm::vec3(-0.15f, 0.15f, -0.35f));
		trans = glm::translate(trans, glm::vec3(0.0f, 0.20f, 0.0));
		trans = glm::rotate(trans, glm::radians(this->rotate_degree), glm::vec3(1.0, 0.0, 0.0));
		//trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
		GLuint transformLoc = glGetUniformLocation(pillarShaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
		transformLoc = glGetUniformLocation(pillarShaderProgram, "view");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));
		transformLoc = glGetUniformLocation(pillarShaderProgram, "projection");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glUseProgram(pillarShaderProgram1);
		glBindVertexArray(pillar1VAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 50);
		glm::mat4 pillarAnim =
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, 7.5)) *
			glm::rotate(glm::mat4(1.0f), glm::radians(this->rotate_degree), glm::vec3(1.0, 0.0, 0.0));
		//glm::scale(glm::mat4(1.0), glm::vec3(0.5, 0.5, 0.5));
		transformLoc = glGetUniformLocation(pillarShaderProgram1, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(pillarAnim));
		transformLoc = glGetUniformLocation(pillarShaderProgram1, "view");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));
		transformLoc = glGetUniformLocation(pillarShaderProgram1, "projection");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));


		glUseProgram(SolidConeShaderProgram);
		glBindVertexArray(SolidConeVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 49);
		trans = glm::mat4(1.0f);
		trans = glm::translate(trans, glm::vec3(-0.5f, 0.35f, -0.35f));
		trans = glm::rotate(trans, glm::radians(this->rotate_degree), glm::vec3(1.0, 1.0, 1.0));
		trans = glm::scale(trans, glm::vec3(0.5, 0.5, 0.5));
		transformLoc = glGetUniformLocation(SolidConeShaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
		transformLoc = glGetUniformLocation(SolidConeShaderProgram, "view");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));
		transformLoc = glGetUniformLocation(SolidConeShaderProgram, "projection");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));

		glUseProgram(EsquareShaderProgram);
		glBindVertexArray(EsquareVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		trans = glm::mat4(1.0f);
		trans = glm::translate(trans, glm::vec3(-0.55f, -0.3f, 0.6f));
		trans = glm::rotate(trans, glm::radians(this->rotate_degree), glm::vec3(1.0, 1.0, 1.0));
		trans = glm::scale(trans, glm::vec3(0.2, 0.2, 0.2));
		transformLoc = glGetUniformLocation(EsquareShaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(trans));
		transformLoc = glGetUniformLocation(EsquareShaderProgram, "view");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(view));
		transformLoc = glGetUniformLocation(EsquareShaderProgram, "projection");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(projection));
	}

	void App::drawTeapot(float angle, glm::mat4 view)
	{
		glUseProgram(teapot_ShaderProgram);
		glBindVertexArray(vao_teapot);
		//glBindBuffer(GL_ARRAY_BUFFER, vbo_teapot_vertices);
		//glVertexAttribPointer(
		//	0, // attribute
		//	3,                 // number of elements per vertex, here (x,y,z)
		//	GL_FLOAT,          // the type of each element
		//	GL_FALSE,          // take our values as-is
		//	0,                 // no extra data between each position
		//	0                  // offset of first element
		//);
		//glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo_teapot_elements);
		glm::mat4 anim =
			glm::translate(glm::mat4(1.0f), glm::vec3(0.0, 0.0, -0.5)) *
			glm::rotate(glm::mat4(1.0f), 1 * angle, glm::vec3(1, 0, 0)) *
			glm::rotate(glm::mat4(1.0f), 2 * angle, glm::vec3(0, 1, 0)) *
			glm::rotate(glm::mat4(1.0f), 3 * angle, glm::vec3(0, 0, 1)) *
			glm::scale(glm::mat4(1.0), glm::vec3(0.5, 0.5, 0.5));

		glm::mat4 model = glm::mat4(1.0f);

		glm::mat4 projectionTeapot = glm::perspective(glm::radians(45.0f), 1.0f * 800 / 600, 0.1f, 100.0f);

		glm::mat4 mvp = projectionTeapot * view * model * anim;
		GLuint transformLoc = glGetUniformLocation(teapot_ShaderProgram, "transform");
		glUniformMatrix4fv(transformLoc, 1, GL_FALSE, glm::value_ptr(mvp));

		glDrawElements(GL_TRIANGLES, sizeof(teapot_elements) / sizeof(teapot_elements[0]), GL_UNSIGNED_SHORT, 0);
	}

	SDL_Surface * App::LoadIMG(const char *filename)
	{
		SDL_Surface *image, *temp = 0;
		//cout << "  * load image = " << filename << flush << endl;
		std::string filePath = std::string(this->data_path) + "resources" + "\\" + filename;
		temp = IMG_Load(filePath.c_str());
		if (NULL == temp)
		{
			//cout << SDL_GetError() << '\n' << flush;
			//cout << filename << " , fehlt." << '\n' << flush << endl;
			
			return 0;
		}
		image = SDL_ConvertSurfaceFormat(temp, temp->format->format, 0);//SDL_DisplayFormat(temp);
		SDL_FreeSurface(temp);
		Uint32 magenta = this->getpixel(image, 0, 0);
		SDL_SetColorKey(image, SDL_TRUE, magenta);// SDL_SRCCOLORKEY | SDL_RLEACCEL

		return image;
	}

	// initialize SDL, window, renderer, texture
	//int pitch, w, h, access;//unsigned int format = 0;//unsigned char* pixels;
	//SDL_SetTextureBlendMode(texture, SDL_BLENDMODE_BLEND);
	//SDL_QueryTexture(texture, &format, &access, &w, &h);
	//SDL_LockTexture(texture, NULL, (void **)&pixels, &pitch);
	//&SDL_GetWindowSurface(this->appWindow)->clip_rect
	//Uint32 *upixels = (Uint32 *)pixels;
	// you will need to know the color of the pixel even if it's transparent
	//Uint32 transparent = SDL_MapRGBA(SDL_GetWindowSurface(this->appWindow)->format, 0xFF, 0x00, 0xFF, 0x00);
	//Uint32 colorKey = SDL_MapRGB(SDL_GetWindowSurface(this->appWindow)->format, 255, 0x00, 255);
	// manipulate pixels
	//for (int i = 0; i < pitch * h; i++) {
		//Uint32 rgabvalue = upixels[i];
		//if (rgabvalue == colorKey){//upixels[i] = transparent;} }
	// replace the old pixels with the new ones
	//memcpy(pixels, upixels, (pitch / 4) * h);
	//SDL_UnlockTexture(texture);

	Uint32 App::getpixel(SDL_Surface *surface, int x, int y)
	{
		int bpp = surface->format->BytesPerPixel;
		Uint8 *p = (Uint8 *)surface->pixels + y * surface->pitch + x * bpp;
		switch (bpp)
		{
		case 1:
			return *p;
		case 2:
			return *(Uint16 *)p;
		case 3:
			if (SDL_BYTEORDER == SDL_BIG_ENDIAN)
				return p[0] << 16 | p[1] << 8 | p[2];
			else
				return p[0] | p[1] << 8 | p[2] << 16;
		case 4:
			return *(Uint32 *)p;
		default:
			return 0;
		}
	}

	typedef struct
	{
		const unsigned char* data;
		size_t size;
		int offset;
	} tImageSource;

	static void pngReadCallback(png_structp png_ptr, png_bytep data, png_size_t length)
	{
		tImageSource* isource = (tImageSource*)png_get_io_ptr(png_ptr);
		if ((int)(isource->offset + length) <= isource->size)
		{
			memcpy(data, isource->data + isource->offset, length);
			isource->offset += length;
		}
		else
		{
			png_error(png_ptr, "pngReaderCallback failed");
		}
		printf("123\n");
	}

	size_t App::loadpng(const char* fileNameStr, int* pnWidth, int* pnHeight, unsigned char** cbData)
	{
		std::string filePath = std::string(this->data_path) + "resources" + "\\" + fileNameStr;
		int file_size = 0;
		FILE* fp = fopen(filePath.c_str(), "rb");
		assert(fp != nullptr);
		png_infop info_ptr;
		png_structp png_ptr;
		png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);
		info_ptr = png_create_info_struct(png_ptr);
		png_init_io(png_ptr, fp);// png_ptr �� �ļ���
		png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);//	��ȡ png �ļ�
		// ������Ϣ
		*pnWidth = png_get_image_width(png_ptr, info_ptr);
		*pnHeight = png_get_image_height(png_ptr, info_ptr);
		png_uint_32 color_type = png_get_color_type(png_ptr, info_ptr);
		assert(color_type == PNG_COLOR_TYPE_RGB_ALPHA);	//��ʱֻ֧��һ�� RGBA �� png ��ʽ
		const int BLOCK_SIZE = 4;//(rgba 4 bytes)
		file_size = (*pnWidth) * (*pnHeight) * BLOCK_SIZE;
		*cbData = new unsigned char[file_size];
		//*cbData = (unsigned char*)malloc(file_size);
		// ��ʼ��ȡ���� �������
		png_bytep *row_point = row_point = png_get_rows(png_ptr, info_ptr);
		int pos = 0;
		for (int y = 0; y < *pnHeight; y++)
		{
			for (int x = 0; x < *pnWidth * BLOCK_SIZE; x += BLOCK_SIZE)
			{
				(*cbData)[pos++] = row_point[y][x + 0];		//	R
				(*cbData)[pos++] = row_point[y][x + 1];		//	G
				(*cbData)[pos++] = row_point[y][x + 2];		//	B
				(*cbData)[pos++] = row_point[y][x + 3];		//	A
			}
		}
		png_destroy_read_struct(&png_ptr, &info_ptr, 0);
		return file_size;
	}

	void App::loadPngFile(const char *filename, Image &pngData)
	{
		png_image image; /* The control structure used by libpng */
		/* Initialize the 'png_image' structure. */
		memset(&image, 0, (sizeof image));
		image.version = PNG_IMAGE_VERSION;
		/* The first argument is the file to read: */
		int pnWidth = 0;
		int pnHeight = 0;
		if (png_image_begin_read_from_file(&image, filename) != 0)
		{
			png_bytep buffer;
			/* Set the format in which to read the PNG file; this code chooses a
			 * simple sRGB format with a non-associated alpha channel, adequate to
			 * store most images.
			 */
			image.format = PNG_FORMAT_RGBA;
			const int BLOCK_SIZE = 4;//(rgba 4 bytes)
			/* Now allocate enough memory to hold the image in this format; the
			 * PNG_IMAGE_SIZE macro uses the information about the image (width,
			 * height and format) stored in 'image'.
			 */
			buffer = (png_bytep)malloc(PNG_IMAGE_SIZE(image));
			/* If enough memory was available, read the image in the desired
			 * format, then write the result out to the new file.  'background' is
			 * not necessary when reading the image, because the alpha channel is
			 * preserved; if it were to be removed, for example if we requested
			 * PNG_FORMAT_RGB, then either a solid background color would have to
			 * be supplied, or the output buffer would have to be initialized to
			 * the actual background of the image.
			 *
			 * The fourth argument to png_image_finish_read is the 'row_stride' -
			 * this is the number of components allocated for the image in each
			 * row.  It has to be at least as big as the value returned by
			 * PNG_IMAGE_ROW_STRIDE, but if you just allocate space for the
			 * default, minimum size, using PNG_IMAGE_SIZE as above, you can pass
			 * zero.
			 *
			 * The final argument is a pointer to a buffer for the colormap;
			 * colormaps have exactly the same format as a row of image pixels
			 * (so you choose what format to make the colormap by setting
			 * image.format).  A colormap is only returned if
			 * PNG_FORMAT_FLAG_COLORMAP is also set in image.format, so in this
			 * case NULL is passed as the final argument.  If you do want to force
			 * all images into an index/color-mapped format, then you can use:
			 *
			 *    PNG_IMAGE_COLORMAP_SIZE(image)
			 *
			 * to find the maximum size of the colormap in bytes.
			 */
			pnHeight = image.height;
			pnWidth = image.width;
			pngData.nHeight = pnHeight;
			pngData.nWidth = pnWidth;
			int readResult = png_image_finish_read(&image, NULL/*background*/, buffer,
				0/*row_stride*/, NULL/*colormap*/);
			if (buffer != NULL && readResult != 0)
			{
				int file_size = pnWidth * pnHeight * BLOCK_SIZE;
				unsigned char **cbData = &pngData.imgData;
				*cbData = new unsigned char[file_size];
				int pos = 0;
				png_bytep *buffer_png = &buffer;
				//int wirteResult = png_image_write_to_file(&image, (char *)*cbData, 0/*convert_to_8bit*/,buffer, 0/*row_stride*/, NULL/*colormap*/);
				for (int y = 0; y < pnHeight; y++)
				{
					for (int x = 0; x < pnWidth * BLOCK_SIZE; x += BLOCK_SIZE)
					{
						(*cbData)[pos++] = buffer_png[y][x + 0];//R
						(*cbData)[pos++] = buffer_png[y][x + 1];//G
						(*cbData)[pos++] = buffer_png[y][x + 2];//B
						(*cbData)[pos++] = buffer_png[y][x + 3];//A
					}
				}
			}
			else
			{
				if (NULL == buffer)
					png_image_free(&image);
				else
					free(buffer);
				/* Something went wrong reading or writing the image.  libpng stores a
			 * textual message in the 'png_image' structure:
			 */
				fprintf(stderr, "pngtopng: error: %s\n", image.message);
				exit(1);
			}
			/* Calling png_image_free is optional unless the simplified API was
				 * not run to completion.  In this case, if there wasn't enough
				 * memory for 'buffer', we didn't complete the read, so we must
				 * free the image:
				 */
			if (NULL == buffer)
				png_image_free(&image);
			else
				free(buffer);
		}
		//fprintf(stderr, "pngtopng: usage: pngtopng input-file output-file\n");
		//exit(2);
	}
	/*����Ѿ�ʹ��png_sig_cmp�����������png���ݣ���Ҫ����
	png_set_sig_bytes����������libpng�⣬�����⴦�����ݵ�ʱ�򽫻�����
	��Ӧ�����ݣ�������ο�http://www.libpng.org/pub/png/libpng-manual.txt
	If you had previously opened the file and read any of the signature from
    the beginning in order to see if this was a PNG file, you need to let
    libpng know that there are some bytes missing from the start of the file.
	*/
}

//����˼·
//https://blog.csdn.net/qq_40953281/article/details/80390118
//https://blog.csdn.net/qq_32190961/article/details/50939926
//https://blog.csdn.net/ycg01/article/details/18371215
//https://blog.csdn.net/theArcticOcean/article/details/54576426
//http://lazyfoo.net/tutorials/SDL/14_animated_sprites_and_vsync/index.php
//https://www.caveofprogramming.com/guest-posts/animating-using-sprite-sheets.html
//https://gamedev.stackexchange.com/questions/70876/sdl2-sprite-batching-and-texture-atlases
/*
class SpriteBatch {
public:
  // when we know the specific texture and source rectangle
  void Draw(TextureHandle tex, Rect source);

  // helper for drawing a whole texture
  void Draw(TextureHandle tex) {
	Draw(tex, Texture_SizeOf(text));
  }

  // helper if we have an atlas and a specific index of a sub-texture therein
  void Draw(TextureAtlasHandle atlas, int index) {
	Draw(TextureAtlas_TextureOf(atlas), TextureAtlas_RectOf(atlas, index));
  }

  // helper if we have an abstract sprite handle without details about a specific atlas
  void Draw(SpriteFrameHandle sprite) {
	Draw(SpriteFrame_SheetOf(sprite), SpriteFrame_IndexOf(sprite));
  }
};
*/

//https://stackoverflow.com/questions/24241974/sdl2-generate-fully-transparent-texture
//SDL�̳�3�����Ż�ͼ����غ���ʾ һ��ǰ��ͼһ������ͼ��
//https://www.cnblogs.com/MrTan/archive/2013/02/21/2920117.html
//SDL2 ����PNGƽ�̱�������ʾǰ��
//https://www.cnblogs.com/joxon/p/5560873.html
//SDL2�����̳�
//https://blog.csdn.net/cyf15238622067/column/info/37317
//Cesiumԭ��ƪ��6 Rendererģ��(2: Texture)
//https://blog.csdn.net/weixin_34345560/article/details/86227401
//SDL Game Development-��2��-5.ʵ�����������Texture Manager
//https://blog.csdn.net/zhu_h_l/article/details/19406593
//������ ���벢��ʾPNGͼƬ
//http://www.ituring.com.cn/article/132400
//pngͼƬ�뱳��ͼalpha�������֮��
//https://bbs.csdn.net/topics/390631201?page=1


//SDL ����ʵս���壩�� SDL ������Ⱦ
//https://www.cnblogs.com/renhui/p/10466773.html
//https://blog.csdn.net/a332324956/article/details/78507091


//sdl2.0ʹ��SDL_RenderCopyEx����ͼƬ��ת
		//https://blog.csdn.net/qq_29883591/article/details/52924047

//SDL_Texture �����ʹ�÷��� SDL_UpdateTextured
		//https://blog.csdn.net/korekara88730/article/details/78357293
        //https://blog.csdn.net/korekara88730/article/details/78363806
		//https://blog.csdn.net/korekara88730/article/details/70880061
		//https://blog.csdn.net/weixin_33794672/article/details/85553293

		//https://stackoverflow.com/questions/18556623/sdl2-messed-up-image-with-manipulating-pixels-and-sdl-updatetexture

		//https://stackoverflow.com/questions/19107476/get-pixel-info-from-sdl2-texture
        //SDL2ϵ�н̳�6-����
		//https://blog.csdn.net/cyf15238622067/article/details/82907529
		//https://wiki.libsdl.org/SDL_QueryTexture
		//https://blog.csdn.net/chunleixiahe/article/details/80265429
		//https://www.cnblogs.com/zl1991/p/6207584.html
		//https://blog.csdn.net/weixin_38176039/article/details/82020865
		//https://www.cnblogs.com/alps/p/5591250.html
        //https://www.cnblogs.com/1024Planet/p/5750481.html
//SDL2.0�ļ���ͼ�л滭
		//http://forums.libsdl.org/viewtopic.php?t=9785
		//https://wiki.libsdl.org/SDL_SetColorKey
		//https://blog.csdn.net/sz76211822/article/details/73603410
		//https://blog.csdn.net/a332324956/article/details/78218901
		//https://stackoverflow.com/questions/51212364/sdl-color-key-an-sdl-texture
		
		//http://bbs.ebaina.com/thread-13024-1-1.html
		//https://blog.csdn.net/victo2012/article/details/51866305
		//https://blog.csdn.net/cyf15238622067/article/details/82907529
