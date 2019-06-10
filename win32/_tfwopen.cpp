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

extern "C" int _twaccess(const char *_Filename, int _AccessMode) {
	return _waccess(Utf8ToWide(_Filename), _AccessMode);
}

extern "C" int _twrename(const char *_OldFilename, const char *_NewFilename) {
	return _wrename(Utf8ToWide(_OldFilename), Utf8ToWide(_NewFilename));
}

extern "C" int _twunlink(const char *_Filename) {
	return _wunlink(Utf8ToWide(_Filename));
}

extern "C" int _twchdir(const char *_Path) {
	return _wchdir(Utf8ToWide(_Path));
}

extern "C" int _twmkdir(const char *_Path) {
	return _wmkdir(Utf8ToWide(_Path));
}

extern "C" int _twrmdir(const char *_Path) {
	return _wrmdir(Utf8ToWide(_Path));
}

extern "C" void _twsplitpath(const char *_FullPath, char *_Drive, char *_Dir, char *_Filename, char *_Ext) {
	wchar_t _wDrive[_MAX_PATH];
	wchar_t _wDir[_MAX_PATH];
	wchar_t _wFilename[_MAX_PATH];
	wchar_t _wExt[_MAX_PATH];
	_wsplitpath(Utf8ToWide(_FullPath), _wDrive, _wDir, _wFilename, _wExt);
	strcpy(_Drive, WideToUtf8(_wDrive));
	strcpy(_Dir, WideToUtf8(_wDir));
	strcpy(_Filename, WideToUtf8(_wFilename));
	strcpy(_Ext, WideToUtf8(_wExt));
}

extern "C" void _twmakepath(char *_Path, const char *_Drive, const char *_Dir, const char *_Filename, const char *_Ext) {
	wchar_t wResultPath[_MAX_PATH];
	wcscpy(wResultPath, L"");
	_wmakepath(wResultPath, Utf8ToWide(_Drive), Utf8ToWide(_Dir), Utf8ToWide(_Filename), Utf8ToWide(_Ext));
	strcpy(_Path, WideToUtf8(wResultPath));
}

extern "C" char *_twcsrchr(const char *_Str, int _Ch) {
	wchar_t *wStr = Utf8ToWide(_Str);
	wchar_t *wResult = wcsrchr(wStr, (wchar_t) _Ch);
	if (wResult != NULL)
	{
		wResult[0] = L'\0';
		return (char*) &_Str[strlen(WideToUtf8(wResult))];
	}
	else
	{
		return NULL;
	}
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
