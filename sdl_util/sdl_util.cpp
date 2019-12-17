#include "sdl_util/sdl_util.h"

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>

namespace sdl
{
	static void swap_rows(char *a, char *b, int count)
	{
		const char *const a_end = a + count;
		while (a < a_end)
		{
			const char tmp = *a;
			*a++ = *b;
			*b++ = tmp;
		}
	}

	static void flip_surface(SDL_Surface *surface)
	{
		const int pitch = surface->pitch;
		char *row1 = static_cast<char *>(surface->pixels);
		char *row2 = row1 + ((surface->h - 1) * pitch);
		while (row1 < row2)
		{
			// Swap the row pixels.
			swap_rows(row1, row2, pitch);

			// Move the row pointers closer to each other.
			row1 += pitch;
			row2 -= pitch;
		}
	}

	void add_surface_ref(SDL_Surface *surface)
	{
		++surface->refcount;
	}

	void quit_sdl(bool unused)
	{
		SDL_Quit();
	}

	gl::TextureHandle   load_texture(const char *path)
	{
		// Try to load as an image.
		SurfacePtr surface(IMG_Load(path), false);
		if (!surface)
		{
			return gl::TextureHandle();
		}

		// Determine the most appropriate OpenGL format.
		// TODO Use a more optimised format?
		const GLenum gl_format = GL_RGBA;
		const GLenum gl_internal_format = GL_RGBA;
		const Uint32 sdl_format = SDL_PIXELFORMAT_ABGR8888;
		
		// Convert the surface to OpenGL format.
		SurfacePtr gl_surface(SDL_ConvertSurfaceFormat(surface.get(), sdl_format, SDL_SWSURFACE), false);

		// Free the original surface.
		surface.reset();

		// Flip the new surface vertically.
		flip_surface(gl_surface.get());

		// Create a texture.
		gl::TextureHandle texture = gl::gen_texture();
		if (!texture){
			return gl::TextureHandle();
		}

		// Bind the texture.
		glBindTexture(GL_TEXTURE_2D, texture.get());

		// Upload the texture image to SDL.
		glTexImage2D(
			GL_TEXTURE_2D,
			0,
			gl_internal_format,
			gl_surface->w,
			gl_surface->h,
			0,
			gl_format,
			GL_UNSIGNED_BYTE,
			gl_surface->pixels);

		// Set up the texture.
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);

		return texture;
	}
}
