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

#define chdir(dir) _chdir(dir)
#define realpath(src, resolved) _fullpath(resolved, src, PATH_MAX)



#endif

#endif /* __SHADER_PLATFORM_H */
