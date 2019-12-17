#include "gl_util/gl.h"

namespace gl
{
	void delete_texture(GLuint texture)
	{
		glDeleteTextures(1, &texture);
	}

	//TextureHandle 
	TextureHandle gen_texture()
	{
		// Generate a texture name.
		GLuint texture = 0;
		glGenTextures(1, &texture);
		return gl::TextureHandle(texture);
	}
}
