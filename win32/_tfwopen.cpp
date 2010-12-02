#ifdef UNICODE
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

extern "C" FILE *_tfwopen(const char *filename, const char *mode ) {
	wchar_t mode_w[30];
	lstrcpyn(mode_w,Utf8ToWide(mode),29);
	mode_w[29]=L'\0';
	return _wfopen(Utf8ToWide(filename),mode_w);
}

extern "C" int _twremove(const char *filename ) {
	return _wremove(Utf8ToWide(filename));
}

#endif // UNICODE