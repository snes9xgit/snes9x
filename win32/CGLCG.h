/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef CGGLCG_H
#define CGGLCG_H

#include <windows.h>
#include "gl_core_3_1.h"
#include "cgFunctions.h"
#include "CCGShader.h"
#include <vector>
#include <deque>
#include "image_functions.h"

typedef struct _xySize {
	double x;
	double y;
} xySize;

class CGLCG
{
private:
	typedef struct _shaderPass {
		cgScaleParams scaleParams;
		bool linearFilter;
        unsigned frameCounterMod;
        bool floatFbo;
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
	typedef struct _prevPass {
		GLuint tex;
		xySize videoSize;
		xySize textureSize;
		GLfloat	texCoords[8];
		_prevPass() {tex=0;}
	} prevPass;
	typedef struct _lookupTexture {
		char id[PATH_MAX];
		GLuint tex;
		_lookupTexture() {tex=NULL;}
	} lookupTexture;

	typedef std::vector<shaderPass> glPassVector;
	typedef std::vector<lookupTexture> glLutVector;
	typedef std::deque<prevPass> glPrevDeque;
	typedef std::vector<CGparameter> glAttribParams;
	glPassVector shaderPasses;
	glLutVector lookupTextures;
	glPrevDeque prevPasses;
	glAttribParams cgAttribParams;

	bool fboFunctionsLoaded;
	bool shaderLoaded;
	bool LoadFBOFunctions();
	void checkForCgError(const char *situation);
	void setTexCoords(int pass,xySize inputSize,xySize textureSize,bool topdown=false);
	void setShaderVars(int pass);
	void resetAttribParams();

	CGcontext cgContext;
	unsigned int frameCnt;
	static const GLfloat lut_coords[8];


public:
	CGLCG(CGcontext cgContext);
	~CGLCG(void);

	bool LoadShader(const TCHAR *shaderFile);
	void Render(GLuint &origTex, xySize textureSize, xySize inputSize, xySize viewportSize, xySize windowSize);
	void ClearPasses();	
};

#endif
