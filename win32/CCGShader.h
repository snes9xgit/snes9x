/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef CCGSHADER_H
#define CCGSHADER_H

#include "port.h"
#include "cgFunctions.h"
#include <vector>

enum cgScaleType { CG_SCALE_NONE, CG_SCALE_SOURCE, CG_SCALE_VIEWPORT, CG_SCALE_ABSOLUTE };
typedef struct _cgScaleParams {
	cgScaleType scaleTypeX;
	cgScaleType scaleTypeY;
	float scaleX;
	float scaleY;
	unsigned absX;
	unsigned absY;
} cgScaleParams;

class CCGShader
{
private:	
	cgScaleType scaleStringToEnum(const char* scale);
public:
	typedef struct _shaderPass {
		cgScaleParams scaleParams;
		bool linearFilter;
		bool filterSet;
        bool floatFbo;
        unsigned frameCounterMod;
		char cgShaderFile[PATH_MAX];
	} shaderPass;
	typedef struct _lookupTexture {
		char id[PATH_MAX];
		char texturePath[PATH_MAX];
		bool linearfilter;
	} lookupTexture;

	CCGShader(void);
	~CCGShader(void);
	bool LoadShader(const char *path);

	typedef std::vector<shaderPass> passVector;
	typedef std::vector<lookupTexture> lutVector;
	passVector shaderPasses;
	lutVector lookupTextures;
};

#endif
