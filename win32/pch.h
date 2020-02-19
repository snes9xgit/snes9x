/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

// pch.h
// Header for standard system include files.

#ifndef _PCH_H_
#define _PCH_H_

// Standard C headers
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <math.h>
#include <memory.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

// POSIX headers
#include <sys/types.h>

// MSVC headers
#ifdef __WIN32__
#include <tchar.h>
#endif

// Standard C++ headers
#ifdef __cplusplus
#include <algorithm>
#include <deque>
#include <fstream>
#include <iostream>
#include <limits>
#include <map>
#include <new>
#include <numeric>
#include <set>
#include <sstream>
#include <stack>
#include <string>
#include <utility>
#include <vector>
#endif

#ifdef ZLIB
#include <zlib.h>
#endif

#ifdef HAVE_LIBPNG
#include <png.h>
#endif

#endif
