/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef W9XDIRECT3D_H
#define W9XDIRECT3D_H

#define MAX_SHADER_TEXTURES 8

#include <d3d9.h>
#include <windows.h>

#include "cgFunctions.h"
#include "CD3DCG.h"

#include "render.h"
#include "wsnes9x.h"
#include "IS9xDisplayOutput.h"

#define FVF_COORDS_TEX D3DFVF_XYZ | D3DFVF_TEX1

typedef struct _VERTEX {
		float x, y, z;
		float tx, ty;
		float lutx, luty;
		_VERTEX() {}
		_VERTEX(float x,float y,float z,float tx,float ty,float lutx, float luty) {
			this->x=x;this->y=y;this->z=z;this->tx=tx;this->ty=ty;this->lutx=lutx;this->luty=luty;
		}
} VERTEX; //our custom vertex with a constuctor for easier assignment

enum current_d3d_shader_type { D3D_SHADER_NONE, D3D_SHADER_CG };

class CDirect3D: public IS9xDisplayOutput
{
private:
	bool                  init_done;					//has initialize been called?
	LPDIRECT3D9           pD3D;
	LPDIRECT3DDEVICE9     pDevice;
	LPDIRECT3DTEXTURE9    drawSurface;					//the texture used for all drawing operations

	LPDIRECT3DVERTEXBUFFER9 vertexBuffer;
	D3DPRESENT_PARAMETERS dPresentParams;
	unsigned int filterScale;							//the current maximum filter scale (at least 2)
	unsigned int afterRenderWidth, afterRenderHeight;	//dimensions after filter has been applied
	unsigned int quadTextureSize;						//size of the texture (only multiples of 2)
	bool fullscreen;									//are we currently displaying in fullscreen mode
	
	VERTEX vertexStream[4];								//the 4 vertices that make up our display rectangle

	static const D3DVERTEXELEMENT9 vertexElems[4];
	LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;

	LPDIRECT3DTEXTURE9      rubyLUT[MAX_SHADER_TEXTURES];
	CGcontext cgContext;
	current_d3d_shader_type shader_type;
	bool cgAvailable;

	CD3DCG *cgShader;

	float shaderTimer;
	int shaderTimeStart;
	int shaderTimeElapsed;
	int frameCount;

	bool BlankTexture(LPDIRECT3DTEXTURE9 texture);
	void CreateDrawSurface();
	void DestroyDrawSurface();
	bool ChangeDrawSurfaceSize(unsigned int scale);
	void SetViewport();
	void SetupVertices();
	bool ResetDevice();
	void SetFiltering();
	bool SetShader(const TCHAR *file);
	void checkForCgError(const char *situation);
	bool SetShaderCG(const TCHAR *file);

public:
	CDirect3D();
	~CDirect3D();
	bool Initialize(HWND hWnd);
	void DeInitialize();
	void Render(SSurface Src);
	bool ChangeRenderSize(unsigned int newWidth, unsigned int newHeight);
	bool ApplyDisplayChanges(void);
	bool SetFullscreen(bool fullscreen);
	void SetSnes9xColorFormat();
	void EnumModes(std::vector<dMode> *modeVector);
};

#endif
