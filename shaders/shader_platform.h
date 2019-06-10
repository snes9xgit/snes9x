/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __SHADER_PLATFORM_H
#define __SHADER_PLATFORM_H

#include "port.h"

#ifdef SNES9X_GTK
#include <epoxy/gl.h>
#include <epoxy/glx.h>
#endif

#ifdef _WIN32
#include <windows.h>
#include <stdlib.h>
#include "gl_core_3_1.h"
#include <direct.h>

#ifdef UNICODE
#define chdir(dir) _wchdir(Utf8ToWide(dir))
#define realpath(src, resolved) _twfullpath(resolved, src, PATH_MAX)
#else
#define chdir(dir) _chdir(dir)
#define realpath(src, resolved) _fullpath(resolved, src, PATH_MAX)
#endif

#endif

#endif /* __SHADER_PLATFORM_H */
