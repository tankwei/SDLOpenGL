#pragma once

#include "dll.h"

#include "gl_util/gl.h"

#include "util/intrusive_ptr.h"
#include "util/unique_handle.h"

#include <SDL2/SDL_video.h>

#include <memory>

namespace sdl
{
	void add_surface_ref(SDL_Surface *surface);
	//SDL_UTIL_DLL void add_surface_ref(SDL_Surface *surface);
	void quit_sdl(bool unused);
	
	//typedef util::UniqueHandle<char *, NULL, void *, &SDL_free> BufferPtr;
	//typedef util::UniqueHandle<SDL_GLContext, NULL, SDL_GLContext, &SDL_GL_DeleteContext> GLContext;
	//typedef util::UniqueHandle<bool, false, bool, &quit_sdl> SdlContext;
	typedef util::IntrusivePtr<SDL_Surface, &add_surface_ref, &SDL_FreeSurface> SurfacePtr;
	typedef std::shared_ptr<SDL_Window> WindowPtr;
	
	gl::TextureHandle  load_texture(const char *path);
	//SDL_UTIL_DLL gl::TextureHandle load_texture(const char *path);
}
