#ifdef UNICODE
#ifndef _TFWOPEN_H
#define _TFWOPEN_H

#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif

FILE *_tfwopen(const char *filename, const char *mode );
int _twremove(const char *filename );

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus
#include <fstream>

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

	explicit __CLR_OR_THIS_CALL u8nifstream(_Filet *_File)
		: std::ifstream(_File) {}
};
}
#define ifstream u8nifstream
#endif // __cplusplus

#define fopen _tfwopen
#define remove _twremove
#endif

#endif // _TFWOPEN_H