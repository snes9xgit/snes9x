/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef _TFWOPEN_H
#define _TFWOPEN_H

#ifdef UNICODE

#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <string.h>
#include <direct.h>
#include <tchar.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE *_tfwopen(const char *filename, const char *mode );
int _twremove(const char *filename );
int _twopen(const char *filename, int oflag, int pmode);
char *_twfullpath(char* dst, const char* src, int len);

int _twaccess(const char *_Filename, int _AccessMode);
int _twrename(const char *_OldFilename, const char *_NewFilename);
int _twunlink(const char *_Filename);
int _twchdir(const char *_Path);
int _twmkdir(const char *_Path);
int _twrmdir(const char *_Path);
void _twsplitpath(const char *_FullPath, char *_Drive, char *_Dir, char *_Filename, char *_Ext);
void _twmakepath(char *_Path, const char *_Drive, const char *_Dir, const char *_Filename, const char *_Ext);
char *_twcsrchr(const char *_Str, int _Ch);

#ifdef __cplusplus
}
#endif

#endif // UNICODE

#ifdef __cplusplus

class Utf8ToWide {
private:
	wchar_t *wideChars;
public:
	Utf8ToWide(const char *utf8Chars);
	~Utf8ToWide() { delete [] wideChars; }
	operator wchar_t *() { return wideChars; }
};

class WideToUtf8 {
private:
	char *utf8Chars;
public:
	WideToUtf8(const wchar_t *wideChars);
	~WideToUtf8() { delete [] utf8Chars; }
	operator char *() { return utf8Chars; }
};

class CPToWide {
private:
   wchar_t *wideChars;
public:
   CPToWide(const char *chars, unsigned int cp);
   ~CPToWide() { delete [] wideChars; }
   operator wchar_t *() { return wideChars; }
};

class WideToCP {
private:
	char *cpchars;
public:
	WideToCP(const wchar_t *wideChars, unsigned int cp);
	~WideToCP() { delete [] cpchars; }
	operator char *() { return cpchars; }
};

#endif // __cplusplus

#ifdef UNICODE
#ifdef __cplusplus
#include <fstream>

namespace std {
class u8nifstream: public std::ifstream
{
public:
	void __CLR_OR_THIS_CALL open(const char *_Filename, ios_base::open_mode _Mode)
		{
			std::ifstream::open(Utf8ToWide(_Filename), (ios_base::openmode)_Mode);
		}

	void __CLR_OR_THIS_CALL open(const wchar_t *_Filename, ios_base::open_mode _Mode)
		{
			std::ifstream::open(_Filename, (ios_base::openmode)_Mode);
		}

	__CLR_OR_THIS_CALL u8nifstream()
		: std::ifstream() {}

	explicit __CLR_OR_THIS_CALL u8nifstream(const char *_Filename,
		ios_base::openmode _Mode = ios_base::in,
		int _Prot = (int)ios_base::_Openprot)
		: std::ifstream(Utf8ToWide(_Filename),_Mode) {}

	explicit __CLR_OR_THIS_CALL u8nifstream(const wchar_t *_Filename,
		ios_base::openmode _Mode = ios_base::in,
		int _Prot = (int)ios_base::_Openprot)
		: std::ifstream(_Filename,_Mode,_Prot) {}

 #ifdef _NATIVE_WCHAR_T_DEFINED
	explicit __CLR_OR_THIS_CALL u8nifstream(const unsigned short *_Filename,
		ios_base::openmode _Mode = ios_base::in,
		int _Prot = (int)ios_base::_Openprot)
		: std::ifstream(_Filename,_Mode,_Prot) {}
 #endif /* _NATIVE_WCHAR_T_DEFINED */

	explicit __CLR_OR_THIS_CALL u8nifstream(FILE *_File)
		: std::ifstream(_File) {}
};
}
#define ifstream u8nifstream
#endif // __cplusplus

__forceinline static FILE *fopenA(const char *filename, const char *mode) {
	return fopen(filename, mode);
}
__forceinline static int removeA(const char *filename) {
	return remove(filename);
}
__forceinline static int openA(const char *filename, int oflag, int pmode) {
	return open(filename, oflag, pmode);
}
__forceinline static int _accessA(const char *path, int mode) {
	return _access(path, mode);
}
__forceinline static int renameA(const char *oldname, const char *newname) {
	return rename(oldname, newname);
}
__forceinline static int _unlinkA(const char *filename) {
	return _unlink(filename);
}
__forceinline static int _chdirA(const char *dirname) {
	return _chdir(dirname);
}
__forceinline static int _mkdirA(const char *dirname) {
	return _mkdir(dirname);
}
__forceinline static int _rmdirA(const char *dirname) {
	return _rmdir(dirname);
}
__forceinline static void _splitpathA(const char *path, char *drive, char *dir, char *fname, char *ext) {
	_splitpath(path, drive, dir, fname, ext);
}
__forceinline static void _makepathA(char *path, const char *drive, const char *dir, const char *fname, const char *ext) {
	_makepath(path, drive, dir, fname, ext);
}
//__forceinline static char *strrchrA(const char *str, int c) {
//	return strrchr(str, c);
//}

#define fopen _tfwopen
#undef remove
__forceinline static int remove(const char *filename) {
  return _twremove(filename);
}
#undef open
__forceinline static int open(const char *filename, int oflag, int pmode) {
  return _twopen(filename, oflag, pmode);
}

#define _access _twaccess
#define rename _twrename
#define _unlink _twunlink
#define _chdir _twchdir
#define _mkdir _twmkdir
#define _rmdir _twrmdir
#define _splitpath _twsplitpath
#define _makepath _twmakepath
//#define strrchr _twcsrchr

#endif // UNICODE

#endif // _TFWOPEN_H
