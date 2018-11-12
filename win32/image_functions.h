#pragma once
#include <tchar.h>
#include <d3d9.h>

typedef struct _STGA {
	_STGA() {
		data = (unsigned char*)0;
		width = 0;
		height = 0;
		byteCount = 0;
	}

	~_STGA() { destroy(); }

	void destroy() { if (data) { delete[] data; data = 0; } }

	int width;
	int height;
	unsigned char byteCount;
	unsigned char* data;
} STGA;

bool loadPngImage(const TCHAR *name, int &outWidth, int &outHeight, bool &outHasAlpha, unsigned char **outData);
bool loadTGA(const TCHAR *filename, STGA& tgaFile);
bool d3d_create_texture_from_file(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DTEXTURE9 *ppTexture);
