/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "CD3DCG.h"
#include "wsnes9x.h"
#include "win32_display.h"
#include "snes9x.h"
#include "dxerr.h"
#include <png.h>
#include "CDirect3D.h"
#include "image_functions.h"

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

static float npot(float desired)
{
	float out=512.0;
	while(out<desired)
		out*=2;
	return out;
}

CD3DCG::CD3DCG(CGcontext cgContext,LPDIRECT3DDEVICE9 pDevice)
{
	this->cgContext = cgContext;
	this->pDevice = pDevice;
	ClearPasses();
	frameCnt=0;
}

CD3DCG::~CD3DCG(void)
{
	LoadShader(NULL);
}

void CD3DCG::ClearPasses()
{
	/* clean up cg programs, vertex buffers/declarations and textures
	   from all regular passes. pass 0 is the orignal texture, so ignore that
	*/
	if(shaderPasses.size()>1) {
		for(std::vector<shaderPass>::iterator it=(shaderPasses.begin()+1);it!=shaderPasses.end();it++) {
			if(it->cgFragmentProgram)
				cgDestroyProgram(it->cgFragmentProgram);
			if(it->cgVertexProgram)
				cgDestroyProgram(it->cgVertexProgram);
			if(it->tex)
				it->tex->Release();
			if(it->vertexBuffer)
				it->vertexBuffer->Release();
			if(it->vertexDeclaration)
				it->vertexDeclaration->Release();
		}
	}
	for(std::vector<lookupTexture>::iterator it=lookupTextures.begin();it!=lookupTextures.end();it++) {
		if(it->tex)
			it->tex->Release();
	}
	for(std::deque<prevPass>::iterator it=prevPasses.begin();it!=prevPasses.end();it++) {
		if(it->tex)
			it->tex->Release();
		if(it->vertexBuffer)
			it->vertexBuffer->Release();
	}
	prevPasses.clear();
	shaderPasses.clear();
	lookupTextures.clear();
	// prevPasses deque is always filled with PREV + PREV1-6 elements
	prevPasses.resize(7);
	shaderLoaded = false;
}

void CD3DCG::OnLostDevice()
{
	/* release all non-managed textures so the device can reset - 
	   will be recreated when required
	*/
	if(shaderPasses.size()>1) {
		for(std::vector<shaderPass>::iterator it=(shaderPasses.begin()+1);it!=shaderPasses.end();it++) {			
			if(it->tex) {
				it->tex->Release();
				it->tex = NULL;
			}
		}
	}
}

void CD3DCG::OnResetDevice()
{

}

void CD3DCG::checkForCgError(const char *situation)
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

bool CD3DCG::LoadShader(const TCHAR *shaderFile)
{
	CCGShader cgShader;
	TCHAR shaderPath[MAX_PATH];
	TCHAR tempPath[MAX_PATH];
	HRESULT hr;
	GLenum error;

	ClearPasses();

	if (shaderFile == NULL || *shaderFile==TEXT('\0'))
		return true;

	lstrcpy(shaderPath, shaderFile);
    ReduceToPath(shaderPath);

	SetCurrentDirectory(shaderPath);
	if(!cgShader.LoadShader(_tToChar(shaderFile)))
		return false;

	CGprofile vertexProfile = cgD3D9GetLatestVertexProfile();
	CGprofile pixelProfile = cgD3D9GetLatestPixelProfile();

	const char** vertexOptions = cgD3D9GetOptimalOptions(vertexProfile);
	const char** pixelOptions = cgD3D9GetOptimalOptions(pixelProfile);

	shaderPasses.push_back(shaderPass());

	for(CCGShader::passVector::iterator it=cgShader.shaderPasses.begin();
			it!=cgShader.shaderPasses.end();it++) {
		shaderPass pass;

		pass.scaleParams = it->scaleParams;
		/* if this is the last pass (the only one that can have CG_SCALE_NONE)
		   and no filter has been set use the GUI setting
		*/
		if(pass.scaleParams.scaleTypeX==CG_SCALE_NONE && !it->filterSet) {
			pass.linearFilter = Settings.BilinearFilter;
		} else {
			pass.linearFilter = it->linearFilter;
		}

        pass.frameCounterMod = it->frameCounterMod;

        pass.useFloatTex = it->floatFbo;

		// paths in the meta file can be relative
		_tfullpath(tempPath,_tFromChar(it->cgShaderFile),MAX_PATH);
		char *fileContents = ReadShaderFileContents(tempPath);
		if(!fileContents)
			return false;

        // individual shader might include files, these should be relative to shader
        ReduceToPath(tempPath);
        SetCurrentDirectory(tempPath);

		pass.cgVertexProgram = cgCreateProgram( cgContext, CG_SOURCE, fileContents,
						vertexProfile, "main_vertex", vertexOptions);

		checkForCgError("Compiling vertex program");

		pass.cgFragmentProgram = cgCreateProgram( cgContext, CG_SOURCE, fileContents,
						pixelProfile, "main_fragment", pixelOptions);

		checkForCgError("Compiling fragment program");

        // set path back for next pass
        SetCurrentDirectory(shaderPath);

		delete [] fileContents;
		if(!pass.cgVertexProgram || !pass.cgFragmentProgram) {
			return false;
		}
		if(pass.cgVertexProgram) {
			hr = cgD3D9LoadProgram(pass.cgVertexProgram,false,0);
		}
		checkForCgError("Loading vertex program");
		if(pass.cgFragmentProgram) {
			hr = cgD3D9LoadProgram(pass.cgFragmentProgram,false,0);
		}
		checkForCgError("Loading fragment program");

		/* generate vertex buffer
		*/
		
		hr = pDevice->CreateVertexBuffer(sizeof(VERTEX)*4,D3DUSAGE_WRITEONLY,0,D3DPOOL_MANAGED,&pass.vertexBuffer,NULL);
		if(FAILED(hr)) {
			pass.vertexBuffer = NULL;
			DXTRACE_ERR_MSGBOX(TEXT("Error creating vertex buffer"), hr);
			return false;
		}

		/* set up vertex declarations for the pass,
		   this also creates the vertex declaration
		*/
		setupVertexDeclaration(pass);

		shaderPasses.push_back(pass);
	}

	for(std::vector<CCGShader::lookupTexture>::iterator it=cgShader.lookupTextures.begin();it!=cgShader.lookupTextures.end();it++) {		
		lookupTexture tex;
		strcpy(tex.id,it->id);
		tex.linearFilter = it->linearfilter;

		_tfullpath(tempPath,_tFromChar(it->texturePath),MAX_PATH);

		if(!d3d_create_texture_from_file(pDevice, tempPath, &tex.tex)){
			tex.tex = NULL;
		}
		lookupTextures.push_back(tex);
	}

	shaderLoaded = true;

	return true;
}

void CD3DCG::ensureTextureSize(LPDIRECT3DTEXTURE9 &tex, XMFLOAT2 &texSize,
                               XMFLOAT2 wantedSize,bool renderTarget,bool useFloat)
{
	HRESULT hr;

	if(!tex || texSize.x != wantedSize.x || texSize.y != wantedSize.y) {
		if(tex)
			tex->Release();

		hr = pDevice->CreateTexture(
			(UINT)wantedSize.x, (UINT)wantedSize.y,
			1, // 1 level, no mipmaps
			renderTarget?D3DUSAGE_RENDERTARGET:0,
            renderTarget?(useFloat?D3DFMT_A32B32G32R32F:D3DFMT_A8R8G8B8):D3DFMT_R5G6B5,
			renderTarget?D3DPOOL_DEFAULT:D3DPOOL_MANAGED, // render targets cannot be managed
			&tex,
			NULL );

		texSize = wantedSize;

		if(FAILED(hr)) {
			DXTRACE_ERR_MSGBOX(TEXT("Error while creating texture"), hr);
			return;
		}
	}
}

void CD3DCG::setVertexStream(IDirect3DVertexBuffer9 *vertexBuffer,
                             XMFLOAT2 inputSize, XMFLOAT2 textureSize, XMFLOAT2 outputSize)
{
	float tX = inputSize.x / textureSize.x;
	float tY = inputSize.y / textureSize.y;
	VERTEX vertexStream[4];

	/* vertex is POSITION, TEXCOORD, LUT TEXCOORD
	*/

	vertexStream[0] = VERTEX(0.0f,0.0f,0.0f,
								0.0f,tY,
								0.0f,1.0f);
	vertexStream[1] = VERTEX(0.0f,1.0f,0.0f,
								0.0f,0.0f,
								0.0f,0.0f);
	vertexStream[2] = VERTEX(1.0f,0.0f,0.0f,
								tX,tY,
								1.0f,1.0f);
	vertexStream[3] = VERTEX(1.0f,1.0f,0.0f,
								tX,0.0f,
								1.0f,0.0f);
	// offset coordinates to correct pixel/texture alignment
	for(int i=0;i<4;i++) {
		vertexStream[i].x -= 0.5f / outputSize.x;
		vertexStream[i].y += 0.5f / outputSize.y;
	}

	void *pLockedVertexBuffer;
	HRESULT hr = vertexBuffer->Lock(0,0,&pLockedVertexBuffer,NULL);
	memcpy(pLockedVertexBuffer,vertexStream,sizeof(vertexStream));
	vertexBuffer->Unlock();

	/* set vertex to stream 0-3, for POSITION,TEXCOORD0,TEXCOORD1,COLOR
	   if stream is used in parameter, this will be set in setShaderVars
	*/
	pDevice->SetStreamSource(0,vertexBuffer,0,sizeof(VERTEX));
	pDevice->SetStreamSource(1,vertexBuffer,0,sizeof(VERTEX));
	pDevice->SetStreamSource(2,vertexBuffer,0,sizeof(VERTEX));
	pDevice->SetStreamSource(3,vertexBuffer,0,sizeof(VERTEX));
}

void CD3DCG::Render(LPDIRECT3DTEXTURE9 &origTex, XMFLOAT2 textureSize,
                    XMFLOAT2 inputSize, XMFLOAT2 viewportSize, XMFLOAT2 windowSize)
{
	LPDIRECT3DSURFACE9 pRenderSurface = NULL,pBackBuffer = NULL;
	frameCnt++;

	if(!shaderLoaded)
		return;

	/* save back buffer render target
	*/
	pDevice->GetRenderTarget(0,&pBackBuffer);	

	/* pass 0 represents the original texture
	*/
	shaderPasses[0].tex = origTex;
	shaderPasses[0].outputSize = inputSize;
	shaderPasses[0].textureSize = textureSize;

	calculateMatrix();	

	for(int i=1;i<shaderPasses.size();i++) {
		switch(shaderPasses[i].scaleParams.scaleTypeX) {
			case CG_SCALE_ABSOLUTE:
				shaderPasses[i].outputSize.x = (double)shaderPasses[i].scaleParams.absX;
				break;
			case CG_SCALE_SOURCE:
				shaderPasses[i].outputSize.x = shaderPasses[i-1].outputSize.x * shaderPasses[i].scaleParams.scaleX;
				break;
			case CG_SCALE_VIEWPORT:
				shaderPasses[i].outputSize.x = viewportSize.x * shaderPasses[i].scaleParams.scaleX;
				break;
			default:
				shaderPasses[i].outputSize.x = viewportSize.x;
		}
		switch(shaderPasses[i].scaleParams.scaleTypeY) {
			case CG_SCALE_ABSOLUTE:
				shaderPasses[i].outputSize.y = (double)shaderPasses[i].scaleParams.absY;
				break;
			case CG_SCALE_SOURCE:
				shaderPasses[i].outputSize.y = shaderPasses[i-1].outputSize.y * shaderPasses[i].scaleParams.scaleY;
				break;
			case CG_SCALE_VIEWPORT:
				shaderPasses[i].outputSize.y = viewportSize.y * shaderPasses[i].scaleParams.scaleY;
				break;
			default:
				shaderPasses[i].outputSize.y = viewportSize.y;
		}
		float texSize = npot(max(shaderPasses[i].outputSize.x,shaderPasses[i].outputSize.y));
		
		/* make sure the render target exists and has an appropriate size,
		   then set as current render target with last pass as source
		*/
		ensureTextureSize(shaderPasses[i].tex,shaderPasses[i].textureSize, XMFLOAT2(texSize,texSize),true,shaderPasses[i].useFloatTex);
		shaderPasses[i].tex->GetSurfaceLevel(0,&pRenderSurface);
		pDevice->SetTexture(0, shaderPasses[i-1].tex);
		pDevice->SetRenderTarget(0,pRenderSurface);
		pRenderSurface->Release();

		/* set vertex declaration of current pass, update vertex
		   buffer and set base streams
		*/
		pDevice->SetVertexDeclaration(shaderPasses[i].vertexDeclaration);
		setVertexStream(shaderPasses[i].vertexBuffer,
			shaderPasses[i-1].outputSize,shaderPasses[i-1].textureSize,shaderPasses[i].outputSize);
		pDevice->SetSamplerState(0, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);
		pDevice->SetSamplerState(0, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);
		pDevice->SetSamplerState(0, D3DSAMP_MAGFILTER, shaderPasses[i].linearFilter?D3DTEXF_LINEAR:D3DTEXF_POINT);
		pDevice->SetSamplerState(0, D3DSAMP_MINFILTER, shaderPasses[i].linearFilter?D3DTEXF_LINEAR:D3DTEXF_POINT);

		/* shader vars need to be set after the base vertex streams
		   have been set so they can override them
		*/
		setShaderVars(i);

		cgD3D9BindProgram(shaderPasses[i].cgVertexProgram);
		checkForCgError("Binding vertex program");
		cgD3D9BindProgram(shaderPasses[i].cgFragmentProgram);
		checkForCgError("Binding fragment program");
		
		/* viewport defines output size
		*/
		setViewport(0,0,(DWORD)shaderPasses[i].outputSize.x, (DWORD)shaderPasses[i].outputSize.y);

        pDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

		pDevice->BeginScene();
		pDevice->DrawPrimitive(D3DPT_TRIANGLESTRIP,0,2);
		pDevice->EndScene();
	}

	/* take oldes PREV out of deque, make sure it has
	   the same size as the current texture and
	   then set it up as new original texture
	*/
	prevPass &oldestPrev = prevPasses.back();
	ensureTextureSize(oldestPrev.tex,oldestPrev.textureSize,textureSize,false);
	if(oldestPrev.vertexBuffer)
		oldestPrev.vertexBuffer->Release();
	origTex = oldestPrev.tex;
	prevPasses.pop_back();
	
	/* push current original with corresponding vertex
	   buffer to front of PREV deque
	*/
	shaderPasses[0].vertexBuffer = shaderPasses[1].vertexBuffer;
	prevPasses.push_front(prevPass(shaderPasses[0]));
	
	/* create new vertex buffer so that next render call
	   will not overwrite the PREV
	*/
	pDevice->CreateVertexBuffer(sizeof(VERTEX)*4,D3DUSAGE_WRITEONLY,0,D3DPOOL_MANAGED,&shaderPasses[1].vertexBuffer,NULL);
	
	/* set up last pass texture, backbuffer and viewport
	   for final display pass without shaders
	*/
	pDevice->SetTexture(0, shaderPasses.back().tex);
	pDevice->SetRenderTarget(0,pBackBuffer);
	pBackBuffer->Release();
	RECT displayRect=CalculateDisplayRect((unsigned int)shaderPasses.back().outputSize.x, (unsigned int)shaderPasses.back().outputSize.y, (unsigned int)windowSize.x, (unsigned int)windowSize.y);
	setViewport(displayRect.left,displayRect.top,displayRect.right - displayRect.left,displayRect.bottom - displayRect.top);
	setVertexStream(shaderPasses.back().vertexBuffer,
		shaderPasses.back().outputSize,shaderPasses.back().textureSize,
		XMFLOAT2((float)(displayRect.right - displayRect.left),(float)(displayRect.bottom - displayRect.top)));
	pDevice->SetVertexShader(NULL);
	pDevice->SetPixelShader(NULL);
}

void CD3DCG::calculateMatrix()
{
	XMMATRIX matWorld;
	XMMATRIX matView;
	XMMATRIX matProj;

	pDevice->GetTransform(D3DTS_WORLD, (D3DMATRIX*)&matWorld);
	pDevice->GetTransform(D3DTS_VIEW, (D3DMATRIX*)&matView);
	pDevice->GetTransform(D3DTS_PROJECTION, (D3DMATRIX*)&matProj);

	XMStoreFloat4x4(&mvp, XMMatrixTranspose(XMMatrixMultiply(matProj, XMMatrixMultiply(matWorld, matView))));
}

void CD3DCG::setViewport(DWORD x, DWORD y, DWORD width, DWORD height)
{
	D3DVIEWPORT9 viewport;
	viewport.X = x;
	viewport.Y = y;
	viewport.Height = height;
	viewport.Width = width;
	viewport.MinZ = 0.0f;
	viewport.MaxZ = 1.0f;
	HRESULT hr = pDevice->SetViewport(&viewport);
}

void CD3DCG::setShaderVars(int pass)
{
	XMFLOAT2 inputSize = shaderPasses[pass-1].outputSize;
	XMFLOAT2 textureSize = shaderPasses[pass-1].textureSize;
	XMFLOAT2 outputSize = shaderPasses[pass].outputSize;

	/* mvp paramater
	*/
	CGparameter cgpModelViewProj = cgGetNamedParameter(shaderPasses[pass].cgVertexProgram, "modelViewProj");

	if(cgpModelViewProj)
		cgD3D9SetUniformMatrix(cgpModelViewProj,(D3DMATRIX*)&mvp);

#define setProgramUniform(pass,varname,floats)\
{\
	CGparameter cgpf = cgGetNamedParameter(shaderPasses[pass].cgFragmentProgram, varname);\
	CGparameter cgpv = cgGetNamedParameter(shaderPasses[pass].cgVertexProgram, varname);\
	if(cgpf)\
		cgD3D9SetUniform(cgpf,floats);\
	if(cgpv)\
		cgD3D9SetUniform(cgpv,floats);\
}\

#define setTextureParameter(pass,varname,val,linear)\
{\
	CGparameter cgpf = cgGetNamedParameter(shaderPasses[pass].cgFragmentProgram, varname);\
    CGparameter cgpv = cgGetNamedParameter(shaderPasses[pass].cgVertexProgram, varname);\
	if(cgpf) {\
		cgD3D9SetTexture(cgpf,val);\
		cgD3D9SetSamplerState(cgpf, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);\
		cgD3D9SetSamplerState(cgpf, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);\
		cgD3D9SetSamplerState(cgpf, D3DSAMP_MINFILTER, linear?D3DTEXF_LINEAR:D3DTEXF_POINT);\
		cgD3D9SetSamplerState(cgpf, D3DSAMP_MAGFILTER, linear?D3DTEXF_LINEAR:D3DTEXF_POINT);\
	}\
    if(cgpv) {\
		cgD3D9SetTexture(cgpv,val);\
		/*cgD3D9SetSamplerState(cgpv, D3DSAMP_ADDRESSU, D3DTADDRESS_BORDER);\
		cgD3D9SetSamplerState(cgpv, D3DSAMP_ADDRESSV, D3DTADDRESS_BORDER);\
		cgD3D9SetSamplerState(cgpv, D3DSAMP_MINFILTER, linear?D3DTEXF_LINEAR:D3DTEXF_POINT);\
		cgD3D9SetSamplerState(cgpv, D3DSAMP_MAGFILTER, linear?D3DTEXF_LINEAR:D3DTEXF_POINT);*/\
	}\
}\

#define setTexCoordsParameter(pass,varname,val)\
{\
	CGparameter cgpv = cgGetNamedParameter(shaderPasses[pass].cgVertexProgram, varname);\
	if(cgpv) {\
		unsigned long index = cgGetParameterResourceIndex(cgpv);\
		pDevice->SetStreamSource(shaderPasses[pass].parameterMap[index].streamNumber,val,0,sizeof(VERTEX));\
	}\
}\

	/* IN paramater
	*/
	setProgramUniform(pass,"IN.video_size",&inputSize);
	setProgramUniform(pass,"IN.texture_size",&textureSize);
	setProgramUniform(pass,"IN.output_size",&outputSize);
    float shaderFrameCnt = (float)frameCnt;
    if(shaderPasses[pass].frameCounterMod)
        shaderFrameCnt = (float)(frameCnt % shaderPasses[pass].frameCounterMod);
	setProgramUniform(pass,"IN.frame_count",&shaderFrameCnt);
    float frameDirection = Settings.Rewinding?-1.0f:1.0f;
    setProgramUniform(pass,"IN.frame_direction",&frameDirection);

	/* ORIG parameter
	*/
	setProgramUniform(pass,"ORIG.video_size",&shaderPasses[0].outputSize);
	setProgramUniform(pass,"ORIG.texture_size",&shaderPasses[0].textureSize);
	setTextureParameter(pass,"ORIG.texture",shaderPasses[0].tex,shaderPasses[1].linearFilter);
	setTexCoordsParameter(pass,"ORIG.tex_coord",shaderPasses[1].vertexBuffer);

	/* PREV parameter
	*/
	if(prevPasses[0].tex) {
		setProgramUniform(pass,"PREV.video_size",&prevPasses[0].imageSize);
		setProgramUniform(pass,"PREV.texture_size",&prevPasses[0].textureSize);
		setTextureParameter(pass,"PREV.texture",prevPasses[0].tex,shaderPasses[1].linearFilter);
		setTexCoordsParameter(pass,"PREV.tex_coord",prevPasses[0].vertexBuffer);
	}

	/* PREV1-6 parameters
	*/
	for(int i=1;i<prevPasses.size();i++) {
		if(!prevPasses[i].tex)
			break;
		char varname[100];
		sprintf(varname,"PREV%d.video_size",i);
		setProgramUniform(pass,varname,&prevPasses[i].imageSize);
		sprintf(varname,"PREV%d.texture_size",i);
		setProgramUniform(pass,varname,&prevPasses[i].textureSize);
		sprintf(varname,"PREV%d.texture",i);
		setTextureParameter(pass,varname,prevPasses[i].tex,shaderPasses[1].linearFilter);
		sprintf(varname,"PREV%d.tex_coord",i);
		setTexCoordsParameter(pass,varname,prevPasses[i].vertexBuffer);
	}

	/* LUT parameters
	*/
	for(int i=0;i<lookupTextures.size();i++) {
		setTextureParameter(pass,lookupTextures[i].id,lookupTextures[i].tex,lookupTextures[i].linearFilter);
	}

	/* PASSX parameters, only for third pass and up
	*/
	if(pass>2) {
		for(int i=1;i<pass-1;i++) {
			char varname[100];
			sprintf(varname,"PASS%d.video_size",i);
			setProgramUniform(pass,varname,&shaderPasses[i].outputSize);
			sprintf(varname,"PASS%d.texture_size",i);
			setProgramUniform(pass,varname,&shaderPasses[i].textureSize);
			sprintf(varname,"PASS%d.texture",i);
			setTextureParameter(pass,varname,shaderPasses[i].tex,shaderPasses[i+1].linearFilter);
			sprintf(varname,"PASS%d.tex_coord",i);
			setTexCoordsParameter(pass,varname,shaderPasses[i+1].vertexBuffer);
		}
	}
}

/* checks for known cg parameters, to make sure they
   get stream offset 12
*/
static bool isKnownParameter(const char *paramName)
{
	int len = strlen(paramName);
	if(len<3)
		return false;
	if(!strncmp(paramName,"IN.",3))
		return true;
	if(len<4)
		return false;
	if(!strncmp(paramName,"PREV",4) ||
		!strncmp(paramName,"PASS",4) ||
		!strncmp(paramName,"ORIG",4))
		return true;
	return false;
}

/* iterate all parameters and record input variyings
   with their semantic and resource index
*/
void CD3DCG::fillParameterMap(std::vector<parameterEntry> &map, CGparameter param)
{
	parameterEntry mapEntry;
	while (param) {
		if(cgGetParameterType(param)==CG_STRUCT)
			fillParameterMap(map,cgGetFirstStructParameter(param));
		else
			if (cgGetParameterDirection(param) == CG_IN && cgGetParameterVariability(param) == CG_VARYING) {
				mapEntry.rIndex = cgGetParameterResourceIndex(param);
				mapEntry.semantic = cgGetParameterSemantic(param);
				mapEntry.isKnownParam = isKnownParameter(cgGetParameterName(param));
				if(map.size()<mapEntry.rIndex+1)
					map.resize(mapEntry.rIndex+1);
				map[mapEntry.rIndex] = mapEntry;
			}
		param = cgGetNextParameter(param);
	}
}

void CD3DCG::setupVertexDeclaration(shaderPass &pass)
{
	static const D3DVERTEXELEMENT9 end = D3DDECL_END();
	CGparameter param = cgGetFirstParameter(pass.cgVertexProgram, CG_PROGRAM);
	fillParameterMap(pass.parameterMap,param);

	/* set up vertex elements according to parameter map.
	   all streams use the same vertex buffer layout, with
	   different offsets.
	   default (unknown name) parameters with semantics
	   POSITION0, TEXCOORD0, TEXCOORD1 and COLOR0 get stream
	   0,1,2 and 3 respectively.
	   the rest (all texcoords) get stream 4 and up
	*/
	D3DVERTEXELEMENT9 *vElems = new D3DVERTEXELEMENT9[pass.parameterMap.size()+1];
	int streamNum = 4;
	for(int i=0;i<pass.parameterMap.size();i++) {
		const char* sem = pass.parameterMap[i].semantic;
		pass.parameterMap[i].streamNumber = 0;
		if(pass.parameterMap[i].isKnownParam==false) {
			if(!strcmp(sem,"TEXCOORD0")) {
				D3DVERTEXELEMENT9 elem = {1, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0};
				vElems[i] = elem;
			} else if(!strcmp(sem,"TEXCOORD1")) {
				D3DVERTEXELEMENT9 elem = {2, 20, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1};
				vElems[i] = elem;
			} else if(!strcmp(sem,"COLOR0")) {
				D3DVERTEXELEMENT9 elem = {3, 12, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0};
				vElems[i] = elem;
			} else if(!strcmp(sem,"POSITION0")) {
				D3DVERTEXELEMENT9 elem = {0, 0,  D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0};
				vElems[i] = elem;
			}
		} else {
			int resIndex = atoi(sem + strlen(sem) - 1);
			D3DVERTEXELEMENT9 elem = {(WORD)streamNum, 12, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, (BYTE)resIndex};
			vElems[i] = elem;
			pass.parameterMap[i].streamNumber = streamNum;
			streamNum++;
		}
	}	
	vElems[pass.parameterMap.size()] = end;

	/* create vertex declaration for vertex elements, then
	   get rid of the elements
	*/
	LPDIRECT3DVERTEXDECLARATION9 vertexDeclaration;
	HRESULT hr = pDevice->CreateVertexDeclaration(vElems,&vertexDeclaration);
	if(FAILED(hr)) {
		DXTRACE_ERR_MSGBOX(TEXT("Error creating vertex declaration"), hr);
	}
	if(pass.vertexDeclaration)
		pass.vertexDeclaration->Release();
	pass.vertexDeclaration = vertexDeclaration;
	delete [] vElems;
}
