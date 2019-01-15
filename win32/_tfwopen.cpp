/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <windows.h>
#include "_tfwopen.h"

Utf8ToWide::Utf8ToWide(const char *utf8Chars) {
	int requiredChars = MultiByteToWideChar(CP_UTF8,0,utf8Chars,-1,wideChars,0);
	wideChars = new wchar_t[requiredChars];
	MultiByteToWideChar(CP_UTF8,0,utf8Chars,-1,wideChars,requiredChars);
}

WideToUtf8::WideToUtf8(const wchar_t *wideChars) {
	int requiredChars = WideCharToMultiByte(CP_UTF8,0,wideChars,-1,utf8Chars,0,NULL,NULL);
	utf8Chars = new char[requiredChars];
	WideCharToMultiByte(CP_UTF8,0,wideChars,-1,utf8Chars,requiredChars,NULL,NULL);
}

CPToWide::CPToWide(const char *chars, unsigned int cp) {
   int requiredChars = MultiByteToWideChar(cp,0,chars,-1,wideChars,0);
   wideChars = new wchar_t[requiredChars];
   MultiByteToWideChar(cp,0,chars,-1,wideChars,requiredChars);
}

WideToCP::WideToCP(const wchar_t *wideChars, unsigned int cp) {
	int requiredChars = WideCharToMultiByte(cp,0,wideChars,-1,cpchars,0,NULL,NULL);
	cpchars = new char[requiredChars];
	WideCharToMultiByte(cp,0,wideChars,-1,cpchars,requiredChars,NULL,NULL);
}

#ifdef UNICODE

extern "C" FILE *_tfwopen(const char *filename, const char *mode ) {
	wchar_t mode_w[30];
	lstrcpyn(mode_w,Utf8ToWide(mode),29);
	mode_w[29]=L'\0';
	return _wfopen(Utf8ToWide(filename),mode_w);
}

extern "C" int _twremove(const char *filename ) {
	return _wremove(Utf8ToWide(filename));
}

extern "C" int _twopen(const char *filename, int oflag, int pmode) {
  return _wopen(Utf8ToWide(filename), oflag, pmode);
}

extern "C" char *_twfullpath(char* dst, const char* src, int len) {
    wchar_t *resolved = _wfullpath(NULL, Utf8ToWide(src), MAX_PATH);
	WideToUtf8 utf8resolved = WideToUtf8(resolved);
	free(resolved);
	if (dst == NULL)
	{
		len = strlen(utf8resolved) + 1;
		dst = (char *)malloc(len);
	}
    strncpy(dst, utf8resolved, len);
    dst[len - 1] = '\0';
    return dst;
}

#endif // UNICODE
