/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma once
#include "port.h"

#if defined(SNES9X_QT)
    #include <glad/gl.h>
    #if defined(_WIN32)
        #define realpath(src, resolved) _fullpath(resolved, src, PATH_MAX)
    #endif

#elif defined(SNES9X_GTK)
    #include <glad/gl.h>
#elif defined(_WIN32)
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