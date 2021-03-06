/*
Copyright_License {

  XCSoar Glide Computer - http://www.xcsoar.org/
  Copyright (C) 2000-2011 The XCSoar Project
  A detailed list of copyright holders can be found in the file "AUTHORS".

  This program is free software; you can redistribute it and/or
  modify it under the terms of the GNU General Public License
  as published by the Free Software Foundation; either version 2
  of the License, or (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
}
*/

#include "Screen/OpenGL/Init.hpp"
#include "Screen/OpenGL/Debug.hpp"
#include "Screen/OpenGL/Cache.hpp"
#include "Screen/OpenGL/Globals.hpp"
#include "Screen/OpenGL/Extension.hpp"
#include "Screen/OpenGL/Features.hpp"

#ifdef ANDROID
#include "Android/Main.hpp"
#include "Android/NativeView.hpp"
#endif

void
OpenGL::Initialise()
{
#ifndef NDEBUG
  thread = pthread_self();
#endif
}

/**
 * Does the current GLES context support textures with dimensions
 * other than power-of-two?
 */
gcc_pure
static bool
SupportsNonPowerOfTwoTexturesGLES()
{
  /* the Dell Streak Mini announces this extension */
  if (OpenGL::IsExtensionSupported("GL_APPLE_texture_2D_limited_npot"))
    return true;

  /* this extension is announced by all modern Android 2.2 handsets,
     however the HTC Desire HD (Adreno 205 GPU) is unable to create
     such textures - not a reliable indicator, it seems */
  if (OpenGL::IsExtensionSupported("GL_OES_texture_npot")) {
    /* flush previous errors */
    while (glGetError() != GL_NO_ERROR) {}

    /* attempt to create an odd texture */
    GLuint id;
    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 11, 11, 0,
                 GL_RGB, GL_UNSIGNED_SHORT_5_6_5, NULL);
    glDeleteTextures(1, &id);

    /* see if there is a complaint */
    return glGetError() == GL_NO_ERROR;
  }

  return false;
}

/**
 * Does the current OpenGL context support textures with dimensions
 * other than power-of-two?
 */
gcc_pure
static bool
SupportsNonPowerOfTwoTextures()
{
  return OpenGL::IsExtensionSupported("GL_ARB_texture_non_power_of_two") ||
    (have_gles() && SupportsNonPowerOfTwoTexturesGLES());
}

void
OpenGL::SetupContext(unsigned width, unsigned height)
{
  screen_width = width;
  screen_height = height;

  texture_non_power_of_two = SupportsNonPowerOfTwoTextures();

#ifdef ANDROID
  native_view->SetTexturePowerOfTwo(texture_non_power_of_two);
#endif
}

void
OpenGL::Deinitialise()
{
  TextCache::flush();
}
