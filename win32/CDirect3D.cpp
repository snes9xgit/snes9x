/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#pragma comment( lib, "d3d9" )

#include "cdirect3d.h"
#include "win32_display.h"
#include "../snes9x.h"
#include "../gfx.h"
#include "../display.h"
#include "wsnes9x.h"
#include "dxerr.h"
#include <commctrl.h>

#include "../filter/hq2x.h"
#include "../filter/2xsai.h"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif
#ifndef min
#define min(a, b) (((a) < (b)) ? (a) : (b))
#endif

const D3DVERTEXELEMENT9 CDirect3D::vertexElems[4] = {
		{0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0},
		{0, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0},
		{0, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		D3DDECL_END()
	};

/* CDirect3D::CDirect3D()
sets default values for the variables
*/
CDirect3D::CDirect3D()
{
	init_done = false;
	pD3D = NULL;
	pDevice = NULL;
	drawSurface = NULL;
	vertexBuffer = NULL;
	afterRenderWidth = 0;
	afterRenderHeight = 0;
	quadTextureSize = 0;
	fullscreen = false;
	filterScale = 1;
	for(int i = 0; i < MAX_SHADER_TEXTURES; i++) {
		rubyLUT[i] = NULL;
	}
	shader_type = D3D_SHADER_NONE;
	shaderTimer = 1.0f;
	shaderTimeStart = 0;
	shaderTimeElapsed = 0;
	frameCount = 0;
	cgContext = NULL;
	cgAvailable = false;
	cgShader = NULL;
	vertexDeclaration = NULL;
}

/* CDirect3D::~CDirect3D()
releases allocated objects
*/
CDirect3D::~CDirect3D()
{
	DeInitialize();
}


/*  CDirect3D::Initialize
Initializes Direct3D (always in window mode)
IN:
hWnd	-	the HWND of the window in which we render/the focus window for fullscreen
-----
returns true if successful, false otherwise
*/
bool CDirect3D::Initialize(HWND hWnd)
{
	if(init_done)
		return true;

	pD3D = Direct3DCreate9(D3D_SDK_VERSION);
	if(pD3D == NULL) {
		DXTRACE_ERR_MSGBOX(TEXT("Error creating initial D3D9 object"), 0);
		return false;
	}

	memset(&dPresentParams, 0, sizeof(dPresentParams));
	dPresentParams.hDeviceWindow = hWnd;
    dPresentParams.Windowed = true;
	dPresentParams.BackBufferCount = GUI.DoubleBuffered?2:1;
    dPresentParams.SwapEffect = D3DSWAPEFFECT_DISCARD;
	dPresentParams.BackBufferFormat = D3DFMT_UNKNOWN;

	HRESULT hr = pD3D->CreateDevice(D3DADAPTER_DEFAULT,
                      D3DDEVTYPE_HAL,
                      hWnd,
                      D3DCREATE_MIXED_VERTEXPROCESSING,
					  &dPresentParams,
                      &pDevice);
	if(FAILED(hr)) {
		DXTRACE_ERR_MSGBOX(TEXT("Error creating D3D9 device"), hr);
		return false;
	}

	hr = pDevice->CreateVertexBuffer(sizeof(vertexStream),D3DUSAGE_WRITEONLY,0,D3DPOOL_MANAGED,&vertexBuffer,NULL);
	if(FAILED(hr)) {
		DXTRACE_ERR_MSGBOX(TEXT("Error creating vertex buffer"), hr);
		return false;
	}

	hr = pDevice->CreateVertexDeclaration(vertexElems,&vertexDeclaration);
	if(FAILED(hr)) {
		DXTRACE_ERR_MSGBOX(TEXT("Error creating vertex declaration"), hr);
		return false;
	}

	cgAvailable = loadCgFunctions();

	if(cgAvailable) {
		cgContext = cgCreateContext();
		hr = cgD3D9SetDevice(pDevice);
		if(FAILED(hr)) {
			DXTRACE_ERR_MSGBOX(TEXT("Error setting cg device"), hr);
		}
		cgShader = new CD3DCG(cgContext,pDevice);
	}

	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);
	pDevice->SetRenderState( D3DRS_ZENABLE, FALSE);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
	pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);

	pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	init_done = true;

	ApplyDisplayChanges();

	return true;

}

void CDirect3D::DeInitialize()
{
	DestroyDrawSurface();
	SetShader(NULL);

	if(cgShader) {
		delete cgShader;
		cgShader = NULL;
	}
	if(cgContext) {
		cgDestroyContext(cgContext);
		cgContext = NULL;
	}
	if(cgAvailable)
		cgD3D9SetDevice(NULL);

	if(vertexBuffer) {
		vertexBuffer->Release();
		vertexBuffer = NULL;
	}

	if(vertexDeclaration) {
		vertexDeclaration->Release();
		vertexDeclaration = NULL;
	}

	if( pDevice ) {
		pDevice->Release();
		pDevice = NULL;
	}

	if( pD3D ) {
		pD3D->Release();
		pD3D = NULL;
	}

	init_done = false;
	afterRenderWidth = 0;
	afterRenderHeight = 0;
	quadTextureSize = 0;
	fullscreen = false;
	filterScale = 0;
	if(cgAvailable)
		unloadCgLibrary();
	cgAvailable = false;
}

bool CDirect3D::SetShader(const TCHAR *file)
{
	SetShaderCG(NULL);
	shader_type = D3D_SHADER_NONE;
	if(file!=NULL &&
		(lstrlen(file)>3 && _tcsncicmp(&file[lstrlen(file)-3],TEXT(".cg"),3)==0) ||
		(lstrlen(file)>4 && _tcsncicmp(&file[lstrlen(file)-4],TEXT(".cgp"),4)==0)){
		return SetShaderCG(file);
	}

	return true;
}

void CDirect3D::checkForCgError(const char *situation)
{
	char buffer[4096];
	CGerror error = cgGetError();
	const char *string = cgGetErrorString(error);

	if (error != CG_NO_ERROR) {
		sprintf(buffer,
			  "Situation: %s\n"
			  "Error: %s\n\n"
			  "Cg compiler output...\n", situation, string);
		MessageBoxA(0, buffer,
				  "Cg error", MB_OK|MB_ICONEXCLAMATION);
		if (error == CG_COMPILER_ERROR) {
			MessageBoxA(0, cgGetLastListing(cgContext),
					  "Cg compilation error", MB_OK|MB_ICONEXCLAMATION);
		}
	}
}

bool CDirect3D::SetShaderCG(const TCHAR *file)
{
	if(!cgAvailable) {
		if(file)
			MessageBox(NULL, TEXT("The CG runtime is unavailable, CG shaders will not run.\nConsult the snes9x readme for information on how to obtain the runtime."), TEXT("CG Error"),
				MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

	if(!cgShader->LoadShader(file))
		return false;

	shader_type = D3D_SHADER_CG;

	return true;
}

/*  CDirect3D::Render
does the actual rendering, changes the draw surface if necessary and recalculates
the vertex information if filter output size changes
IN:
Src		-	the input surface
*/
void CDirect3D::Render(SSurface Src)
{
	SSurface Dst;
	RECT dstRect;
	unsigned int newFilterScale;
	D3DLOCKED_RECT lr;
	D3DLOCKED_RECT lrConv;
	HRESULT hr;

	if(!init_done) return;

	//create a new draw surface if the filter scale changes
	//at least factor 2 so we can display unscaled hi-res images
	newFilterScale = max(2,max(GetFilterScale(GUI.ScaleHiRes),GetFilterScale(GUI.Scale)));
	if(newFilterScale!=filterScale) {
		ChangeDrawSurfaceSize(newFilterScale);
	}

	if(FAILED(hr = pDevice->TestCooperativeLevel())) {
		switch(hr) {
			case D3DERR_DEVICELOST:		//do no rendering until device is restored
				return;
			case D3DERR_DEVICENOTRESET: //we can reset now
                if(!IsIconic(dPresentParams.hDeviceWindow))
				    ResetDevice();
				return;
			default:
				DXTRACE_ERR_MSGBOX(TEXT("Internal driver error"), hr);
				return;
		}
	}

	//BlankTexture(drawSurface);
	if(FAILED(hr = drawSurface->LockRect(0, &lr, NULL, 0))) {
		DXTRACE_ERR_MSGBOX(TEXT("Unable to lock texture"), hr);
		return;
	} else {
		Dst.Surface = (unsigned char *)lr.pBits;
		Dst.Height = quadTextureSize;
		Dst.Width = quadTextureSize;
		Dst.Pitch = lr.Pitch;

		RenderMethod (Src, Dst, &dstRect);
		if(!Settings.AutoDisplayMessages) {
			WinSetCustomDisplaySurface((void *)Dst.Surface, Dst.Pitch/2, dstRect.right-dstRect.left, dstRect.bottom-dstRect.top, GetFilterScale(CurrentScale));
			S9xDisplayMessages ((uint16*)Dst.Surface, Dst.Pitch/2, dstRect.right-dstRect.left, dstRect.bottom-dstRect.top, GetFilterScale(CurrentScale));
		}

		drawSurface->UnlockRect(0);
	}

	if(!GUI.Stretch||GUI.AspectRatio)
		pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

	//if the output size of the render method changes we need to update the viewport
	if(afterRenderHeight != dstRect.bottom || afterRenderWidth != dstRect.right) {
		afterRenderHeight = dstRect.bottom;
		afterRenderWidth = dstRect.right;
		SetViewport();
	}

	pDevice->SetTexture(0, drawSurface);
	pDevice->SetVertexDeclaration(vertexDeclaration);
	pDevice->SetStreamSource(0,vertexBuffer,0,sizeof(VERTEX));

	if(shader_type == D3D_SHADER_CG) {
		RECT displayRect;
		//Get maximum rect respecting AR setting
		displayRect=CalculateDisplayRect(dPresentParams.BackBufferWidth,dPresentParams.BackBufferHeight,
										dPresentParams.BackBufferWidth,dPresentParams.BackBufferHeight);
		cgShader->Render(drawSurface,
			XMFLOAT2((float)quadTextureSize, (float)quadTextureSize),
			XMFLOAT2((float)afterRenderWidth, (float)afterRenderHeight),
			XMFLOAT2((float)(displayRect.right - displayRect.left),
								(float)(displayRect.bottom - displayRect.top)),
			XMFLOAT2((float)dPresentParams.BackBufferWidth, (float)dPresentParams.BackBufferHeight));
	}

	SetFiltering();

	pDevice->SetVertexDeclaration(vertexDeclaration);

	pDevice->BeginScene();
	pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2);
	pDevice->EndScene();

	pDevice->Present(NULL, NULL, NULL, NULL);

	if (GUI.ReduceInputLag)
	{
		IDirect3DSurface9 *surface;
		RECT r = { 0, 0, 2, 2 };

		if (pDevice->GetBackBuffer(0, 0, D3DBACKBUFFER_TYPE_MONO, &surface) == D3D_OK)
		{
			if (surface->LockRect(&lr, &r, D3DLOCK_READONLY) == D3D_OK)
			{
				surface->UnlockRect();
			}
			surface->Release();
		}
	}

    return;
}

/*  CDirect3D::CreateDrawSurface
calculates the necessary texture size (multiples of 2)
and creates a new texture
*/
void CDirect3D::CreateDrawSurface()
{
	unsigned int neededSize;
	HRESULT hr;

	//we need at least 512 pixels (SNES_WIDTH * 2) so we can start with that value
	quadTextureSize = 512;
	neededSize = SNES_WIDTH * filterScale;
	while(quadTextureSize < neededSize)
		quadTextureSize *=2;

	if(!drawSurface) {
		hr = pDevice->CreateTexture(
			quadTextureSize, quadTextureSize,
			1, // 1 level, no mipmaps
			0, // dynamic textures can be locked
			D3DFMT_R5G6B5,
			D3DPOOL_MANAGED,
			&drawSurface,
			NULL );

		if(FAILED(hr)) {
			DXTRACE_ERR_MSGBOX(TEXT("Error while creating texture"), hr);
			return;
		}
	}
}

/*  CDirect3D::DestroyDrawSurface
releases the old textures (if allocated)
*/
void CDirect3D::DestroyDrawSurface()
{
	if(drawSurface) {
		drawSurface->Release();
		drawSurface = NULL;
	}
}

/*  CDirect3D::BlankTexture
clears a texture (fills it with zeroes)
IN:
texture		-	the texture to be blanked
-----
returns true if successful, false otherwise
*/
bool CDirect3D::BlankTexture(LPDIRECT3DTEXTURE9 texture)
{
	D3DLOCKED_RECT lr;
	HRESULT hr;

	if(FAILED(hr = texture->LockRect(0, &lr, NULL, 0))) {
		DXTRACE_ERR_MSGBOX(TEXT("Unable to lock texture"), hr);
		return false;
	} else {
		memset(lr.pBits, 0, lr.Pitch * quadTextureSize);
		texture->UnlockRect(0);
		return true;
	}
}

/*  CDirect3D::ChangeDrawSurfaceSize
changes the draw surface size: deletes the old textures, creates a new texture
and calculate new vertices
IN:
scale	-	the scale that has to fit into the textures
-----
returns true if successful, false otherwise
*/
bool CDirect3D::ChangeDrawSurfaceSize(unsigned int scale)
{
	filterScale = scale;

	if(pDevice) {
		DestroyDrawSurface();
		CreateDrawSurface();
		SetupVertices();
		return true;
	}
	return false;
}

/*  CDirect3D::SetupVertices
calculates the vertex coordinates
(respecting the stretch and aspect ratio settings)
*/
void CDirect3D::SetupVertices()
{
	void *pLockedVertexBuffer;

	float tX = (float)afterRenderWidth / (float)quadTextureSize;
	float tY = (float)afterRenderHeight / (float)quadTextureSize;

	vertexStream[0] = VERTEX(0.0f,0.0f,0.0f,0.0f,tY,0.0f,0.0f);
	vertexStream[1] = VERTEX(0.0f,1.0f,0.0f,0.0f,0.0f,0.0f,0.0f);
	vertexStream[2] = VERTEX(1.0f,0.0f,0.0f,tX,tY,0.0f,0.0f);
	vertexStream[3] = VERTEX(1.0f,1.0f,0.0f,tX,0.0f,0.0f,0.0f);
	for(int i=0;i<4;i++) {
		vertexStream[i].x -= 0.5f / (float)dPresentParams.BackBufferWidth;
		vertexStream[i].y += 0.5f / (float)dPresentParams.BackBufferHeight;
	}


	HRESULT hr = vertexBuffer->Lock(0,0,&pLockedVertexBuffer,NULL);
	memcpy(pLockedVertexBuffer,vertexStream,sizeof(vertexStream));
	vertexBuffer->Unlock();
}

void CDirect3D::SetViewport()
{
	XMMATRIX matIdentity;
	XMMATRIX matProjection;

	matProjection = XMMatrixOrthographicOffCenterLH(0.0f,1.0f,0.0f,1.0f,0.0f,1.0f);
	matIdentity = XMMatrixIdentity();
	pDevice->SetTransform(D3DTS_WORLD,(D3DMATRIX*)&matIdentity);
	pDevice->SetTransform(D3DTS_VIEW, (D3DMATRIX*)&matIdentity);
	pDevice->SetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&matProjection);

	RECT drawRect = CalculateDisplayRect(afterRenderWidth,afterRenderHeight,dPresentParams.BackBufferWidth,dPresentParams.BackBufferHeight);
	D3DVIEWPORT9 viewport;
	viewport.X = drawRect.left;
	viewport.Y = drawRect.top;
	viewport.Height = drawRect.bottom - drawRect.top;
	viewport.Width = drawRect.right - drawRect.left;
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;
	HRESULT hr = pDevice->SetViewport(&viewport);

	SetupVertices();
}

/*  CDirect3D::ChangeRenderSize
determines if we need to reset the device (if the size changed)
called with (0,0) whenever we want new settings to take effect
IN:
newWidth,newHeight	-	the new window size
-----
returns true if successful, false otherwise
*/
bool CDirect3D::ChangeRenderSize(unsigned int newWidth, unsigned int newHeight)
{
	if(!init_done)
		return false;

	//if we already have the desired size no change is necessary
	//during fullscreen no changes are allowed
	if(dPresentParams.BackBufferWidth == newWidth && dPresentParams.BackBufferHeight == newHeight)
		return true;

	if(!ResetDevice())
		return false;

	return true;
}

/*  CDirect3D::ResetDevice
resets the device
called if surface was lost or the settings/display size require a device reset
-----
returns true if successful, false otherwise
*/
bool CDirect3D::ResetDevice()
{
	if(!init_done) return false;

	HRESULT hr;

	//release prior to reset
	DestroyDrawSurface();

	if(cgAvailable) {
		cgShader->OnLostDevice();
		cgD3D9SetDevice(NULL);
	}

	//zero or unknown values result in the current window size/display settings
	dPresentParams.BackBufferWidth = 0;
	dPresentParams.BackBufferHeight = 0;
	dPresentParams.BackBufferCount = GUI.DoubleBuffered?2:1;
	dPresentParams.BackBufferFormat = D3DFMT_UNKNOWN;
	dPresentParams.FullScreen_RefreshRateInHz = 0;
	dPresentParams.Windowed = true;
	dPresentParams.PresentationInterval = GUI.Vsync?D3DPRESENT_INTERVAL_ONE:D3DPRESENT_INTERVAL_IMMEDIATE;
	dPresentParams.Flags = D3DPRESENTFLAG_LOCKABLE_BACKBUFFER;

	if(fullscreen) {
		dPresentParams.BackBufferWidth = GUI.FullscreenMode.width;
		dPresentParams.BackBufferHeight = GUI.FullscreenMode.height;
		dPresentParams.BackBufferCount = GUI.DoubleBuffered?2:1;
		dPresentParams.Windowed = false;
		if(GUI.FullscreenMode.depth == 32)
			dPresentParams.BackBufferFormat = D3DFMT_X8R8G8B8;
		else
			dPresentParams.BackBufferFormat = D3DFMT_R5G6B5;
		dPresentParams.FullScreen_RefreshRateInHz = GUI.FullscreenMode.rate;
	}

	if(FAILED(hr = pDevice->Reset(&dPresentParams))) {
		DXTRACE_ERR(TEXT("Unable to reset device"), hr);
		return false;
	}

	if(cgAvailable) {
		cgD3D9SetDevice(pDevice);
		cgShader->OnResetDevice();
	}

	pDevice->SetRenderState(D3DRS_LIGHTING, FALSE);

	pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);
	
	//recreate the surface
	CreateDrawSurface();

	SetViewport();

	return true;
}

/*  CDirect3D::SetSnes9xColorFormat
sets the color format to 16bit (since the texture is always 16bit)
no depth conversion is necessary (done by D3D)
*/
void CDirect3D::SetSnes9xColorFormat()
{
	GUI.ScreenDepth = 16;
	GUI.BlueShift = 0;
	GUI.GreenShift = 6;
	GUI.RedShift = 11;
	S9xBlit2xSaIFilterInit();
	S9xBlitHQ2xFilterInit();
	GUI.NeedDepthConvert = FALSE;
	GUI.DepthConverted = TRUE;
	return;
}

/*  CDirect3D::SetFullscreen
enables/disables fullscreen mode
IN:
fullscreen	-	determines if fullscreen is enabled/disabled
-----
returns true if successful, false otherwise
*/
bool CDirect3D::SetFullscreen(bool fullscreen)
{
	if(!init_done)
		return false;

	if(this->fullscreen==fullscreen)
		return true;

	this->fullscreen = fullscreen;
	if(!ResetDevice())
		return false;

	//present here to get a fullscreen blank even if no rendering is done
	pDevice->Present(NULL,NULL,NULL,NULL);

	return true;
}

/*  CDirect3D::EnumModes
enumerates possible display modes (only 16 and 32 bit) and fills the vector
IN:
modeVector	-	pointer to the mode vector
*/
void CDirect3D::EnumModes(std::vector<dMode> *modeVector)
{
	D3DDISPLAYMODE d3dMode;
	int modeCount,index;
	dMode mode;

	if(!init_done)
		return;

	//enumerate 32bit modes
	modeCount = pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT,D3DFMT_X8R8G8B8);
	for(int i=0;i<modeCount;i++) {
		if(pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT,D3DFMT_X8R8G8B8,i,&d3dMode)==D3D_OK) {
			mode.width = d3dMode.Width;
			mode.height = d3dMode.Height;
			mode.rate = d3dMode.RefreshRate;
			mode.depth = 32;

			modeVector->push_back(mode);
		}

	}

	//enumerate 16bit modes
	modeCount = pD3D->GetAdapterModeCount(D3DADAPTER_DEFAULT,D3DFMT_R5G6B5);
	for(int i=0;i<modeCount;i++) {
		if(pD3D->EnumAdapterModes(D3DADAPTER_DEFAULT,D3DFMT_R5G6B5,i,&d3dMode)==D3D_OK) {
			mode.width = d3dMode.Width;
			mode.height = d3dMode.Height;
			mode.rate = d3dMode.RefreshRate;
			mode.depth = 16;

			modeVector->push_back(mode);
		}

	}
}

/*  CDirect3D::ApplyDisplayChanges
calls changerendersize to apply new display settings
-----
returns true if successful, false otherwise
*/
bool CDirect3D::ApplyDisplayChanges(void)
{
	if(GUI.shaderEnabled && GUI.D3DshaderFileName)
		SetShader(GUI.D3DshaderFileName);
	else
		SetShader(NULL);

	return ChangeRenderSize(0,0);
}

void CDirect3D::SetFiltering()
{
	if(Settings.BilinearFilter) {
		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_LINEAR);
		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_LINEAR);
	} else {
		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
	}
}
