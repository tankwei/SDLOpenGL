#pragma once

#ifdef SDL_UTIL_EXPORTS
#	define SDL_UTIL_DLL __declspec(dllexport)
#else
#	define SDL_UTIL_DLL __declspec(dllimport)
#endif
