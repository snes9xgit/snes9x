#include "snes9x.h"
#include <windows.h>
#include <png.h>
#include "dxerr.h"
#include "image_functions.h"

#ifdef HAVE_LIBPNG
bool loadPngImage(const TCHAR *name, int &outWidth, int &outHeight, bool &outHasAlpha, unsigned char **outData) {
	png_structp png_ptr;
	png_infop info_ptr;
	unsigned int sig_read = 0;
	int color_type, interlace_type;
	FILE *fp;

	if ((fp = _tfopen(name, TEXT("rb"))) == NULL)
		return false;

	/* Create and initialize the png_struct
	* with the desired error handler
	* functions.  If you want to use the
	* default stderr and longjump method,
	* you can supply NULL for the last
	* three parameters.  We also supply the
	* the compiler header file version, so
	* that we know if the application
	* was compiled with a compatible version
	* of the library.  REQUIRED
	*/
	png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
		NULL, NULL, NULL);

	if (png_ptr == NULL) {
		fclose(fp);
		return false;
	}

	/* Allocate/initialize the memory
	* for image information.  REQUIRED. */
	info_ptr = png_create_info_struct(png_ptr);
	if (info_ptr == NULL) {
		fclose(fp);
		png_destroy_read_struct(&png_ptr, (png_infopp)NULL, (png_infopp)NULL);
		return false;
	}

	/* Set error handling if you are
	* using the setjmp/longjmp method
	* (this is the normal method of
	* doing things with libpng).
	* REQUIRED unless you  set up
	* your own error handlers in
	* the png_create_read_struct()
	* earlier.
	*/
	if (setjmp(png_jmpbuf(png_ptr))) {
		/* Free all of the memory associated
		* with the png_ptr and info_ptr */
		png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);
		fclose(fp);
		/* If we get here, we had a
		* problem reading the file */
		return false;
	}

	/* Set up the output control if
	* you are using standard C streams */
	png_init_io(png_ptr, fp);

	/* If we have already
	* read some of the signature */
	png_set_sig_bytes(png_ptr, sig_read);

	/*
	* If you have enough memory to read
	* in the entire image at once, and
	* you need to specify only
	* transforms that can be controlled
	* with one of the PNG_TRANSFORM_*
	* bits (this presently excludes
	* dithering, filling, setting
	* background, and doing gamma
	* adjustment), then you can read the
	* entire image (including pixels)
	* into the info structure with this
	* call
	*
	* PNG_TRANSFORM_STRIP_16 |
	* PNG_TRANSFORM_PACKING  forces 8 bit
	* PNG_TRANSFORM_EXPAND forces to
	*  expand a palette into RGB
	*/
	png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, (png_voidp)NULL);

	outWidth = png_get_image_width(png_ptr, info_ptr);
	outHeight = png_get_image_height(png_ptr, info_ptr);
	switch (png_get_color_type(png_ptr, info_ptr)) {
	case PNG_COLOR_TYPE_RGBA:
		outHasAlpha = true;
		break;
	case PNG_COLOR_TYPE_RGB:
		outHasAlpha = false;
		break;
	default:
		png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
		fclose(fp);
		return false;
	}
	unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
	*outData = (unsigned char*)malloc(row_bytes * outHeight);

	png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

	for (int i = 0; i < outHeight; i++) {
		memcpy(*outData + (row_bytes * i), row_pointers[i], row_bytes);
	}

	/* Clean up after the read,
	* and free any memory allocated */
	png_destroy_read_struct(&png_ptr, &info_ptr, (png_infopp)NULL);

	/* Close the file */
	fclose(fp);

	/* That's it */
	return true;
}
#else
bool loadPngImage(const TCHAR *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData) {
	/* No PNG support */
	return false;
}
#endif

bool loadTGA(const TCHAR *filename, STGA& tgaFile)
{
	FILE *file;
	unsigned char type[4];
	unsigned char info[6];

	file = _tfopen(filename, TEXT("rb"));

	if (!file)
		return false;

	fread(&type, sizeof(char), 3, file);
	fseek(file, 12, SEEK_SET);
	fread(&info, sizeof(char), 6, file);

	//image type either 2 (color) or 3 (greyscale)
	if (type[1] != 0 || (type[2] != 2 && type[2] != 3))
	{
		fclose(file);
		return false;
	}

	tgaFile.width = info[0] + info[1] * 256;
	tgaFile.height = info[2] + info[3] * 256;
	tgaFile.byteCount = info[4] / 8;

	if (tgaFile.byteCount != 3 && tgaFile.byteCount != 4) {
		fclose(file);
		return false;
	}

	long imageSize = tgaFile.width * tgaFile.height * tgaFile.byteCount;

	//allocate memory for image data
	unsigned char *tempBuf = new unsigned char[imageSize];
	tgaFile.data = new unsigned char[tgaFile.width * tgaFile.height * 4];

	//read in image data
	fread(tempBuf, sizeof(unsigned char), imageSize, file);

	//swap line order and convert to RBGA
	for (int i = 0; i<tgaFile.height; i++) {
		unsigned char* source = tempBuf + tgaFile.width * (tgaFile.height - 1 - i) * tgaFile.byteCount;
		unsigned char* destination = tgaFile.data + tgaFile.width * i * 4;
		for (int j = 0; j<tgaFile.width; j++) {
			destination[0] = source[2];
			destination[1] = source[1];
			destination[2] = source[0];
			destination[3] = tgaFile.byteCount == 4 ? source[3] : 0xff;
			source += tgaFile.byteCount;
			destination += 4;
		}
	}
	delete[] tempBuf;
	tgaFile.byteCount = 4;

	//close file
	fclose(file);

	return true;
}

bool d3d_create_texture_from_file(LPDIRECT3DDEVICE9 pDevice, LPCTSTR pSrcFile, LPDIRECT3DTEXTURE9 *ppTexture)
{
	int strLen = lstrlen(pSrcFile);
	if (strLen<5)
		return false;

	int width, height;
	bool hasAlpha = false, needFree = false;
	STGA stga;
	unsigned char *bytes;

	if (!_tcsicmp(&pSrcFile[strLen - 4], TEXT(".png"))) {
		if (!loadPngImage(pSrcFile, width, height, hasAlpha, &bytes))
			return false;

		needFree = true;
	}
	else if (!_tcsicmp(&pSrcFile[strLen - 4], TEXT(".tga"))) {
		if (!loadTGA(pSrcFile, stga))
			return false;

		width = stga.width;
		height = stga.height;
		bytes = stga.data;
		hasAlpha = true;
	}

	bool ret = false;
	HRESULT hr = pDevice->CreateTexture(width, height, 1, 0, hasAlpha ? D3DFMT_A8R8G8B8 : D3DFMT_X8R8G8B8, D3DPOOL_MANAGED, ppTexture, NULL);
	if (SUCCEEDED(hr)) {
		LPDIRECT3DTEXTURE9 pTexture = *ppTexture;
		D3DLOCKED_RECT lr;
		HRESULT hr;

		if (FAILED(hr = pTexture->LockRect(0, &lr, NULL, 0))) {
			pTexture->Release();
			DXTRACE_ERR_MSGBOX(TEXT("Unable to lock texture"), hr);
		}
		else {
			memcpy(lr.pBits, bytes, lr.Pitch * height);
			pTexture->UnlockRect(0);
			ret = true;
		}
	}

	if (needFree)
		free(bytes);

	return ret;
}
