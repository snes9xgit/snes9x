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
//cgD3D9.dll
CGD3DSD cgD3D9SetDevice = NULL;
CGD3DBP cgD3D9BindProgram = NULL;
CGD3DGLVP cgD3D9GetLatestVertexProfile = NULL;
CGD3DGLPP cgD3D9GetLatestPixelProfile = NULL;
CGD3DGOO cgD3D9GetOptimalOptions = NULL;
CGD3DLP cgD3D9LoadProgram = NULL;
CGD3DSUM cgD3D9SetUniformMatrix = NULL;
CGD3DSU cgD3D9SetUniform = NULL;
//cggl.dll
CGGLSSMP cgGLSetStateMatrixParameter = NULL;
CGGLSP2FV cgGLSetParameter2fv = NULL;
CGGLGLP cgGLGetLatestProfile = NULL;
CGGLEP cgGLEnableProfile = NULL;
CGGLDP cgGLDisableProfile = NULL;
CGGLSOO cgGLSetOptimalOptions = NULL;
CGGLLP cgGLLoadProgram = NULL;
CGGLBP cgGLBindProgram = NULL;

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
	//cgD3D9.dll
	cgD3D9SetDevice = (CGD3DSD)GetProcAddress(hCgD3D9Dll,"cgD3D9SetDevice");
	cgD3D9BindProgram = (CGD3DBP)GetProcAddress(hCgD3D9Dll,"cgD3D9BindProgram");
	cgD3D9GetLatestVertexProfile = (CGD3DGLVP)GetProcAddress(hCgD3D9Dll,"cgD3D9GetLatestVertexProfile");
	cgD3D9GetLatestPixelProfile = (CGD3DGLPP)GetProcAddress(hCgD3D9Dll,"cgD3D9GetLatestPixelProfile");
	cgD3D9GetOptimalOptions = (CGD3DGOO)GetProcAddress(hCgD3D9Dll,"cgD3D9GetOptimalOptions");
	cgD3D9LoadProgram = (CGD3DLP)GetProcAddress(hCgD3D9Dll,"cgD3D9LoadProgram");
	cgD3D9SetUniformMatrix = (CGD3DSUM)GetProcAddress(hCgD3D9Dll,"cgD3D9SetUniformMatrix");
	cgD3D9SetUniform = (CGD3DSU)GetProcAddress(hCgD3D9Dll,"cgD3D9SetUniform");
	//cggl.dll
	cgGLSetStateMatrixParameter = (CGGLSSMP)GetProcAddress(hCgGLDll,"cgGLSetStateMatrixParameter");
	cgGLSetParameter2fv = (CGGLSP2FV)GetProcAddress(hCgGLDll,"cgGLSetParameter2fv");
	cgGLGetLatestProfile = (CGGLGLP)GetProcAddress(hCgGLDll,"cgGLGetLatestProfile");
	cgGLEnableProfile = (CGGLEP)GetProcAddress(hCgGLDll,"cgGLEnableProfile");
	cgGLDisableProfile = (CGGLDP)GetProcAddress(hCgGLDll,"cgGLDisableProfile");
	cgGLSetOptimalOptions = (CGGLSOO)GetProcAddress(hCgGLDll,"cgGLSetOptimalOptions");
	cgGLLoadProgram = (CGGLLP)GetProcAddress(hCgGLDll,"cgGLLoadProgram");
	cgGLBindProgram = (CGGLBP)GetProcAddress(hCgGLDll,"cgGLBindProgram");

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
		//cgD3D9.dll
		!cgD3D9SetDevice ||
		!cgD3D9BindProgram ||
		!cgD3D9GetLatestVertexProfile ||
		!cgD3D9GetLatestPixelProfile ||
		!cgD3D9GetOptimalOptions ||
		!cgD3D9LoadProgram ||
		!cgD3D9SetUniformMatrix ||
		!cgD3D9SetUniform ||
		//cggl.dll
		!cgGLSetStateMatrixParameter ||
		!cgGLSetParameter2fv ||
		!cgGLGetLatestProfile ||
		!cgGLEnableProfile ||
		!cgGLDisableProfile ||
		!cgGLSetOptimalOptions ||
		!cgGLLoadProgram ||
		!cgGLBindProgram) {
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
	//cgD3D9.dll
	cgD3D9SetDevice = NULL;
	cgD3D9BindProgram = NULL;
	cgD3D9GetLatestVertexProfile = NULL;
	cgD3D9GetLatestPixelProfile = NULL;
	cgD3D9GetOptimalOptions = NULL;
	cgD3D9LoadProgram = NULL;
	cgD3D9SetUniformMatrix = NULL;
	cgD3D9SetUniform = NULL;
	//cggl.dll
	cgGLSetStateMatrixParameter = NULL;
	cgGLSetParameter2fv = NULL;
	cgGLGetLatestProfile = NULL;
	cgGLEnableProfile = NULL;
	cgGLDisableProfile = NULL;
	cgGLSetOptimalOptions = NULL;
	cgGLLoadProgram = NULL;
	cgGLBindProgram = NULL;
}
