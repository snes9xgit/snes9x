/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include "cgFunctions.h"

HMODULE hCgDll = NULL;
HMODULE hCgD3D9Dll = NULL;
HMODULE hCgGLDll = NULL;

//cg.dll
CGCC cgCreateContext = NULL;
CGDC cgDestroyContext = NULL;
CGGNP cgGetNamedParameter = NULL;
CGGE cgGetError = NULL;
CGGES cgGetErrorString = NULL;
CGGLL cgGetLastListing = NULL;
CGCP cgCreateProgram = NULL;
CGDP cgDestroyProgram = NULL;
CGGPRI cgGetParameterResourceIndex = NULL;
CGGFP cgGetFirstParameter = NULL;
CGGNEP cgGetNextParameter = NULL;
CGGPD cgGetParameterDirection = NULL;
CGGPS cgGetParameterSemantic = NULL;
CGGRS cgGetResourceString = NULL;
CGGPV cgGetParameterVariability = NULL;
CGGPT cgGetParameterType = NULL;
CGGFSP cgGetFirstStructParameter = NULL;
CGGPN cgGetParameterName = NULL;
//cgD3D9.dll
CGD3DSD cgD3D9SetDevice = NULL;
CGD3DBP cgD3D9BindProgram = NULL;
CGD3DGLVP cgD3D9GetLatestVertexProfile = NULL;
CGD3DGLPP cgD3D9GetLatestPixelProfile = NULL;
CGD3DGOO cgD3D9GetOptimalOptions = NULL;
CGD3DLP cgD3D9LoadProgram = NULL;
CGD3DSUM cgD3D9SetUniformMatrix = NULL;
CGD3DSU cgD3D9SetUniform = NULL;
CGD3DST cgD3D9SetTexture = NULL;
CGD3DGVD cgD3D9GetVertexDeclaration = NULL;
CGD3DSSS cgD3D9SetSamplerState = NULL;
//cggl.dll
CGGLSSMP cgGLSetStateMatrixParameter = NULL;
CGGLSP2FV cgGLSetParameter2fv = NULL;
CGGLSP1F cgGLSetParameter1f = NULL;
CGGLGLP cgGLGetLatestProfile = NULL;
CGGLEP cgGLEnableProfile = NULL;
CGGLDP cgGLDisableProfile = NULL;
CGGLSOO cgGLSetOptimalOptions = NULL;
CGGLLP cgGLLoadProgram = NULL;
CGGLBP cgGLBindProgram = NULL;
CGGLSTP cgGLSetTextureParameter = NULL;
CGGLETP cgGLEnableTextureParameter = NULL;
CGGLSPP cgGLSetParameterPointer = NULL;
CGGLECS cgGLEnableClientState = NULL;
CGGLDCS cgGLDisableClientState = NULL;

bool loadCgFunctions()
{
	if(hCgDll && hCgD3D9Dll && hCgGLDll)
		return true;

	hCgDll = LoadLibrary(TEXT("cg.dll"));
	if(hCgDll) {
		hCgD3D9Dll = LoadLibrary(TEXT("cgD3D9.dll"));
		hCgGLDll = LoadLibrary(TEXT("cgGL.dll"));
	}

	if(!hCgDll || !hCgD3D9Dll || !hCgGLDll) {
		unloadCgLibrary();
		return false;
	}

	//cg.dll
	cgCreateContext = (CGCC)GetProcAddress(hCgDll,"cgCreateContext");
	cgDestroyContext = (CGDC)GetProcAddress(hCgDll,"cgDestroyContext");
	cgGetNamedParameter = (CGGNP)GetProcAddress(hCgDll,"cgGetNamedParameter");
	cgGetError = (CGGE)GetProcAddress(hCgDll,"cgGetError");
	cgGetErrorString = (CGGES)GetProcAddress(hCgDll,"cgGetErrorString");
	cgGetLastListing = (CGGLL)GetProcAddress(hCgDll,"cgGetLastListing");
	cgCreateProgram = (CGCP)GetProcAddress(hCgDll,"cgCreateProgram");
	cgDestroyProgram = (CGDP)GetProcAddress(hCgDll,"cgDestroyProgram");
	cgGetParameterResourceIndex = (CGGPRI)GetProcAddress(hCgDll,"cgGetParameterResourceIndex");
	cgGetFirstParameter = (CGGFP)GetProcAddress(hCgDll,"cgGetFirstParameter");
	cgGetNextParameter = (CGGNEP)GetProcAddress(hCgDll,"cgGetNextParameter");
	cgGetParameterDirection = (CGGPD)GetProcAddress(hCgDll,"cgGetParameterDirection");
	cgGetParameterSemantic = (CGGPS)GetProcAddress(hCgDll,"cgGetParameterSemantic");
	cgGetResourceString = (CGGRS)GetProcAddress(hCgDll,"cgGetResourceString");
	cgGetParameterVariability = (CGGPV)GetProcAddress(hCgDll,"cgGetParameterVariability");
	cgGetParameterType = (CGGPT)GetProcAddress(hCgDll,"cgGetParameterType");
	cgGetFirstStructParameter = (CGGFSP)GetProcAddress(hCgDll,"cgGetFirstStructParameter");
	cgGetParameterName = (CGGPN)GetProcAddress(hCgDll,"cgGetParameterName");
	//cgD3D9.dll
	cgD3D9SetDevice = (CGD3DSD)GetProcAddress(hCgD3D9Dll,"cgD3D9SetDevice");
	cgD3D9BindProgram = (CGD3DBP)GetProcAddress(hCgD3D9Dll,"cgD3D9BindProgram");
	cgD3D9GetLatestVertexProfile = (CGD3DGLVP)GetProcAddress(hCgD3D9Dll,"cgD3D9GetLatestVertexProfile");
	cgD3D9GetLatestPixelProfile = (CGD3DGLPP)GetProcAddress(hCgD3D9Dll,"cgD3D9GetLatestPixelProfile");
	cgD3D9GetOptimalOptions = (CGD3DGOO)GetProcAddress(hCgD3D9Dll,"cgD3D9GetOptimalOptions");
	cgD3D9LoadProgram = (CGD3DLP)GetProcAddress(hCgD3D9Dll,"cgD3D9LoadProgram");
	cgD3D9SetUniformMatrix = (CGD3DSUM)GetProcAddress(hCgD3D9Dll,"cgD3D9SetUniformMatrix");
	cgD3D9SetUniform = (CGD3DSU)GetProcAddress(hCgD3D9Dll,"cgD3D9SetUniform");
	cgD3D9SetTexture = (CGD3DST)GetProcAddress(hCgD3D9Dll,"cgD3D9SetTexture");
	cgD3D9GetVertexDeclaration = (CGD3DGVD)GetProcAddress(hCgD3D9Dll,"cgD3D9GetVertexDeclaration");
	cgD3D9SetSamplerState = (CGD3DSSS)GetProcAddress(hCgD3D9Dll,"cgD3D9SetSamplerState");
	//cggl.dll
	cgGLSetStateMatrixParameter = (CGGLSSMP)GetProcAddress(hCgGLDll,"cgGLSetStateMatrixParameter");
	cgGLSetParameter2fv = (CGGLSP2FV)GetProcAddress(hCgGLDll,"cgGLSetParameter2fv");
	cgGLSetParameter1f = (CGGLSP1F)GetProcAddress(hCgGLDll,"cgGLSetParameter1f");
	cgGLGetLatestProfile = (CGGLGLP)GetProcAddress(hCgGLDll,"cgGLGetLatestProfile");
	cgGLEnableProfile = (CGGLEP)GetProcAddress(hCgGLDll,"cgGLEnableProfile");
	cgGLDisableProfile = (CGGLDP)GetProcAddress(hCgGLDll,"cgGLDisableProfile");
	cgGLSetOptimalOptions = (CGGLSOO)GetProcAddress(hCgGLDll,"cgGLSetOptimalOptions");
	cgGLLoadProgram = (CGGLLP)GetProcAddress(hCgGLDll,"cgGLLoadProgram");
	cgGLBindProgram = (CGGLBP)GetProcAddress(hCgGLDll,"cgGLBindProgram");
	cgGLSetTextureParameter = (CGGLSTP)GetProcAddress(hCgGLDll,"cgGLSetTextureParameter");
	cgGLEnableTextureParameter = (CGGLETP)GetProcAddress(hCgGLDll,"cgGLEnableTextureParameter");
	cgGLSetParameterPointer = (CGGLSPP)GetProcAddress(hCgGLDll,"cgGLSetParameterPointer");
	cgGLEnableClientState = (CGGLECS)GetProcAddress(hCgGLDll,"cgGLEnableClientState");
	cgGLDisableClientState = (CGGLDCS)GetProcAddress(hCgGLDll,"cgGLDisableClientState");

	if(
		//cg.dll
		!cgCreateContext ||
		!cgDestroyContext ||
		!cgGetNamedParameter ||
		!cgGetError ||
		!cgGetErrorString ||
		!cgGetLastListing ||
		!cgCreateProgram ||
		!cgDestroyProgram ||
		!cgGetParameterResourceIndex ||
		!cgGetFirstParameter ||
		!cgGetNextParameter ||
		!cgGetParameterDirection ||
		!cgGetParameterSemantic ||
		!cgGetResourceString ||
		!cgGetParameterVariability ||
		!cgGetParameterType ||
		!cgGetFirstStructParameter ||
		!cgGetParameterName ||
		//cgD3D9.dll
		!cgD3D9SetDevice ||
		!cgD3D9BindProgram ||
		!cgD3D9GetLatestVertexProfile ||
		!cgD3D9GetLatestPixelProfile ||
		!cgD3D9GetOptimalOptions ||
		!cgD3D9LoadProgram ||
		!cgD3D9SetUniformMatrix ||
		!cgD3D9SetUniform ||
		!cgD3D9SetTexture ||
		!cgD3D9GetVertexDeclaration ||
		!cgD3D9SetSamplerState ||
		//cggl.dll
		!cgGLSetStateMatrixParameter ||
		!cgGLSetParameter2fv ||
		!cgGLSetParameter1f ||
		!cgGLGetLatestProfile ||
		!cgGLEnableProfile ||
		!cgGLDisableProfile ||
		!cgGLSetOptimalOptions ||
		!cgGLLoadProgram ||
		!cgGLBindProgram ||
		!cgGLSetTextureParameter ||
		!cgGLEnableTextureParameter ||
		!cgGLSetParameterPointer ||
		!cgGLEnableClientState ||
		!cgGLDisableClientState) {
			unloadCgLibrary();
			return false;
	}
		
	return true;
}

void unloadCgLibrary()
{
	if(hCgDll)
		FreeLibrary(hCgDll);
	if(hCgD3D9Dll)
		FreeLibrary(hCgD3D9Dll);
	if(hCgGLDll)
		FreeLibrary(hCgGLDll);

	hCgDll = hCgD3D9Dll = hCgGLDll = NULL;

	//cg.dll
	cgCreateContext = NULL;
	cgDestroyContext = NULL;
	cgGetNamedParameter = NULL;
	cgGetError = NULL;
	cgGetErrorString = NULL;
	cgGetLastListing = NULL;
	cgCreateProgram = NULL;
	cgDestroyProgram = NULL;
	cgGetParameterResourceIndex = NULL;
	cgGetFirstParameter = NULL;
	cgGetNextParameter = NULL;
	cgGetParameterDirection = NULL;
	cgGetParameterSemantic = NULL;
	cgGetResourceString = NULL;
	cgGetParameterVariability = NULL;
	cgGetParameterType = NULL;
	cgGetFirstStructParameter = NULL;
	cgGetParameterName = NULL;
	//cgD3D9.dll
	cgD3D9SetDevice = NULL;
	cgD3D9BindProgram = NULL;
	cgD3D9GetLatestVertexProfile = NULL;
	cgD3D9GetLatestPixelProfile = NULL;
	cgD3D9GetOptimalOptions = NULL;
	cgD3D9LoadProgram = NULL;
	cgD3D9SetUniformMatrix = NULL;
	cgD3D9SetUniform = NULL;
	cgD3D9SetTexture = NULL;
	cgD3D9GetVertexDeclaration = NULL;
	cgD3D9SetSamplerState = NULL;
	//cggl.dll
	cgGLSetStateMatrixParameter = NULL;
	cgGLSetParameter2fv = NULL;
	cgGLGetLatestProfile = NULL;
	cgGLEnableProfile = NULL;
	cgGLDisableProfile = NULL;
	cgGLSetOptimalOptions = NULL;
	cgGLLoadProgram = NULL;
	cgGLBindProgram = NULL;
	cgGLEnableClientState = NULL;
	cgGLDisableClientState = NULL;
}
