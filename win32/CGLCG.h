#ifndef CGGLCG_H
#define CGGLCG_H

#include <windows.h>
#include <gl/gl.h>
#include "glext.h"
#include "cgFunctions.h"
#include "CCGShader.h"
#include <vector>

typedef struct _xySize {
	double x;
	double y;
} xySize;

class CGLCG
{
private:
	typedef struct _STGA {
		_STGA() {data = (unsigned char*)0;
				 width = 0;
				 height = 0;
				 byteCount = 0;}

		~_STGA() { delete[] data; data = 0; }

		void destroy() { delete[] data; data = 0; }

		int width;
		int height;
		unsigned char byteCount;
		unsigned char* data;
	} STGA;
	typedef struct _shaderPass {
		cgScaleParams scaleParams;
		bool linearFilter;
		CGprogram cgVertexProgram, cgFragmentProgram;
		GLuint tex;
		GLuint fbo;
		xySize outputSize;
		xySize textureSize;
		GLfloat	texcoords[8];
		_shaderPass()  {cgVertexProgram=NULL;
					    cgFragmentProgram=NULL;
						fbo=NULL;
						tex=NULL;}
	} shaderPass;
	typedef struct _lookupTexture {
		char id[PATH_MAX];
		GLuint tex;
		_lookupTexture() {tex=NULL;}
	} lookupTexture;

	typedef std::vector<shaderPass> glPassVector;
	typedef std::vector<lookupTexture> glLutVector;
	glPassVector shaderPasses;
	glLutVector lookupTextures;

	bool fboFunctionsLoaded;
	bool shaderLoaded;
	bool LoadFBOFunctions();
	void checkForCgError(const char *situation);
	void setTexCoords(int pass,xySize inputSize,xySize textureSize,bool topdown=false);
	void setShaderVars(int pass);
	bool loadPngImage(const TCHAR *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData);
	bool loadTGA(const TCHAR *filename, STGA& tgaFile);

	CGcontext cgContext;
	int frameCnt;
	static const GLfloat lut_coords[8];
	GLuint prevTex;
	xySize prevTexSize;
	xySize prevTexImageSize;
	GLfloat	prevTexCoords[8];


	PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers;
	PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers;
	PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer;
	PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D;
	PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus;
	PFNGLACTIVETEXTUREPROC glClientActiveTexture;

public:
	CGLCG(CGcontext cgContext);
	~CGLCG(void);

	bool LoadShader(const TCHAR *shaderFile);
	void Render(GLuint &origTex, xySize textureSize, xySize inputSize, xySize viewportSize, xySize windowSize);
	void ClearPasses();	
};

#endif
