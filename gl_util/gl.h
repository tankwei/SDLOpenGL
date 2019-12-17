#pragma once

#include "dll.h"

#include "util/unique_handle.h"

#include <SDL2/SDL_opengl.h>

namespace gl
{
	//GL_UTIL_DLL
    void delete_texture(GLuint texture);
	
	typedef util::UniqueHandle<GLuint, 0, GLuint, &delete_texture> TextureHandle;

	//GL_UTIL_DLL
	TextureHandle gen_texture();
}
