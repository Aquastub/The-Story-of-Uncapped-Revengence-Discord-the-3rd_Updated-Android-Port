// Emacs style mode select   -*- C++ -*-
//-----------------------------------------------------------------------------
//
// Copyright (C) 1998-2000 by DooM Legacy Team.
// Copyright (C) 2014-2022 by Sonic Team Junior.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
//-----------------------------------------------------------------------------
/// \file
/// \brief SDL specific part of the OpenGL API for SRB2

#ifdef _MSC_VER
#pragma warning(disable : 4214 4244)
#endif

#ifdef HAVE_SDL
#define _MATH_DEFINES_DEFINED

#include "SDL.h"

#include "sdlmain.h"

#ifdef _MSC_VER
#pragma warning(default : 4214 4244)
#endif

#include "../doomdef.h"

#ifdef HWRENDER
#include "../hardware/r_opengl/r_opengl.h"
#include "../hardware/hw_main.h"
#include "ogl_sdl.h"
#include "../i_system.h"
#include "hwsym_sdl.h"
#include "../m_argv.h"

#ifdef DEBUG_TO_FILE
#include <stdarg.h>
#if defined (_WIN32) && !defined (__CYGWIN__)
#include <direct.h>
#else
#include <unistd.h>
#endif
#include <sys/types.h>
#include <sys/stat.h>
#endif

#ifdef USE_WGL_SWAP
PFNWGLEXTSWAPCONTROLPROC wglSwapIntervalEXT = NULL;
#else
typedef int (*PFNGLXSWAPINTERVALPROC) (int);
PFNGLXSWAPINTERVALPROC glXSwapIntervalSGIEXT = NULL;
#endif

/**	\brief SDL video display surface
*/
void *GLUhandle = NULL;
SDL_GLContext sdlglcontext = 0;

void *GLBackend_GetFunction(const char *proc)
{
	if (strncmp(proc, "glu", 3) == 0)
	{
		if (GLUhandle)
			return hwSym(proc, GLUhandle);
		else
			return NULL;
	}
	return SDL_GL_GetProcAddress(proc);
}

boolean GLBackend_Init(void)
{
#ifndef STATIC_OPENGL
	const char *OGLLibname = NULL;
	const char *GLULibname = NULL;

	if (M_CheckParm("-OGLlib") && M_IsNextParm())
		OGLLibname = M_GetNextParm();

	if (SDL_GL_LoadLibrary(OGLLibname) != 0)
	{
		CONS_Alert(CONS_ERROR, "Could not load OpenGL Library: %s\n"
					"Falling back to Software mode.\n", SDL_GetError());
		if (!M_CheckParm("-OGLlib"))
			CONS_Printf("If you know what is the OpenGL library's name, use -OGLlib\n");
		return 0;
	}

#if 0
	GLULibname = "/proc/self/exe";
#elif defined (_WIN32)
	GLULibname = "GLU32.DLL";
#elif defined (__MACH__)
	GLULibname = "/System/Library/Frameworks/OpenGL.framework/Libraries/libGLU.dylib";
#elif defined (macintos)
	GLULibname = "OpenGLLibrary";
#elif defined (__unix__)
	GLULibname = "libGLU.so.1";
#elif defined (__HAIKU__)
	GLULibname = "libGLU.so";
#else
	GLULibname = NULL;
#endif

	if (M_CheckParm("-GLUlib") && M_IsNextParm())
		GLULibname = M_GetNextParm();

	if (GLULibname)
	{
		GLUhandle = hwOpen(GLULibname);
		if (GLUhandle)
			return GLBackend_LoadFunctions();
		else
		{
			CONS_Alert(CONS_ERROR, "Could not load GLU Library: %s\n", GLULibname);
			if (!M_CheckParm ("-GLUlib"))
				CONS_Alert(CONS_ERROR, "If you know what is the GLU library's name, use -GLUlib\n");
		}
	}
	else
	{
		CONS_Alert(CONS_ERROR, "Could not load GLU Library\n");
		CONS_Alert(CONS_ERROR, "If you know what is the GLU library's name, use -GLUlib\n");
	}
#endif
	return GLBackend_LoadFunctions();
}

/**	\brief	The OglSdlSurface function

	\param	w	width
	\param	h	height

	\return	if true, changed video mode
*/
boolean OglSdlSurface(INT32 w, INT32 h)
{
	INT32 cbpp = cv_scr_depth.value < 16 ? 16 : cv_scr_depth.value;

	if (!GLBackend_InitContext())
		return false;

	if (!GLBackend_LoadExtraFunctions())
		return false;

	GLBackend_SetSurface(w, h);

	textureformatGL = cbpp > 16 ? GL_RGBA : GL_RGB5_A1;

	return true;
}

/**	\brief	The OglSdlFinishUpdate function

	\param	vidwait	wait for video sync

	\return	void
*/
void OglSdlFinishUpdate(boolean waitvbl)
{
	int sdlw, sdlh;

	static boolean oldwaitvbl = false;
	if (oldwaitvbl != waitvbl)
		SDL_GL_SetSwapInterval(waitvbl ? 1 : 0);

	oldwaitvbl = waitvbl;

	SDL_GetWindowSize(window, &sdlw, &sdlh);
	GPU->MakeFinalScreenTexture();

#ifdef HAVE_GL_FRAMEBUFFER
	GLFramebuffer_Disable();
	RenderToFramebuffer = FramebufferEnabled;
#endif

	GPU->DrawFinalScreenTexture(sdlw, sdlh);

#ifdef HAVE_GL_FRAMEBUFFER
	if (RenderToFramebuffer)
		GLFramebuffer_Enable();
#endif

	SDL_GL_SwapWindow(window);
	GPU->GClipRect(0, 0, realwidth, realheight, NZCLIP_PLANE);

	// Sryder:	We need to draw the final screen texture again into the other buffer in the original position so that
	//			effects that want to take the old screen can do so after this
	GPU->DrawFinalScreenTexture(realwidth, realheight);
}

#endif //HWRENDER
#endif //SDL
