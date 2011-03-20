#include "COpenGL.h"
#include "win32_display.h"
#include "../snes9x.h"
#include "../gfx.h"
#include "../display.h"
#include "wsnes9x.h"
#include <msxml2.h>

#include "../filter/hq2x.h"
#include "../filter/2xsai.h"


COpenGL::COpenGL(void)
{
	hDC = NULL;
	hRC = NULL;
	hWnd = NULL;
	drawTexture = 0;
	initDone = false;
	quadTextureSize = 0;
	filterScale = 0;
	afterRenderWidth = 0;
	afterRenderHeight = 0;
	fullscreen = false;
	shaderFunctionsLoaded = false;
	shader_type = OGL_SHADER_NONE;
	pboFunctionsLoaded = false;
	shaderProgram = 0;
    vertexShader = 0;
    fragmentShader = 0;
	cgContext = NULL;
	cgVertexProgram = cgFragmentProgram = NULL;
	cgAvailable = false;
}

COpenGL::~COpenGL(void)
{
	DeInitialize();
}

bool COpenGL::Initialize(HWND hWnd)
{
	int pfdIndex;
	RECT windowRect;

	this->hWnd = hWnd;
	this->hDC = GetDC(hWnd);

	PIXELFORMATDESCRIPTOR pfd=
	{
		sizeof(PIXELFORMATDESCRIPTOR),					// Size Of This Pixel Format Descriptor
		1,												// Version Number
		PFD_DRAW_TO_WINDOW |							// Format Must Support Window
		PFD_SUPPORT_OPENGL |							// Format Must Support OpenGL
		PFD_DOUBLEBUFFER,								// Must Support Double Buffering
		PFD_TYPE_RGBA,									// Request An RGBA Format
		16,												// Select Our Color Depth
		0, 0, 0, 0, 0, 0,								// Color Bits Ignored
		0,												// No Alpha Buffer
		0,												// Shift Bit Ignored
		0,												// No Accumulation Buffer
		0, 0, 0, 0,										// Accumulation Bits Ignored
		16,												// 16Bit Z-Buffer (Depth Buffer)
		0,												// No Stencil Buffer
		0,												// No Auxiliary Buffer
		PFD_MAIN_PLANE,									// Main Drawing Layer
		0,												// Reserved
		0, 0, 0											// Layer Masks Ignored
	};
	PIXELFORMATDESCRIPTOR pfdSel;

	if(!(pfdIndex=ChoosePixelFormat(hDC,&pfd))) {
		DeInitialize();
		return false;
	}
	if(!SetPixelFormat(hDC,pfdIndex,&pfd)) {
		DeInitialize();
		return false;
	}
	if(!(hRC=wglCreateContext(hDC))) {
		DeInitialize();
		return false;
	}
	if(!wglMakeCurrent(hDC,hRC)) {
		DeInitialize();
		return false;
	}

	LoadPBOFunctions();

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)wglGetProcAddress( "wglSwapIntervalEXT" );
	
	glEnableClientState(GL_VERTEX_ARRAY);
    glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glEnable(GL_BLEND);
	glEnable(GL_TEXTURE_2D);

	glMatrixMode (GL_PROJECTION);
    glLoadIdentity ();
    glOrtho (0.0, 1.0, 0.0, 1.0, -1, 1);
	
	glVertexPointer(2, GL_FLOAT, 0, vertices);
	glTexCoordPointer(2, GL_FLOAT, 0, texcoords);

	cgAvailable = loadCgFunctions();
	if(cgAvailable) {
		cgContext = cgCreateContext();
	}

	GetClientRect(hWnd,&windowRect);
	ChangeRenderSize(windowRect.right,windowRect.bottom);

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);
	SwapBuffers(hDC);

	initDone = true;
	return true;
}

void COpenGL::DeInitialize()
{
	initDone = false;
	SetShaders(NULL);
	DestroyDrawSurface();
	wglMakeCurrent(NULL,NULL);
	if(hRC) {
		wglDeleteContext(hRC);
		hRC = NULL;
	}
	if(hDC) {
		ReleaseDC(hWnd,hDC);
		hDC = NULL;
	}
	hWnd = NULL;
	initDone = false;
	quadTextureSize = 0;
	filterScale = 0;
	afterRenderWidth = 0;
	afterRenderHeight = 0;
	shaderFunctionsLoaded = false;
	shader_type = OGL_SHADER_NONE;
	unloadCgLibrary();
	cgAvailable = false;
}

void COpenGL::CreateDrawSurface()
{
	unsigned int neededSize;
	HRESULT hr;

	//we need at least 512 pixels (SNES_WIDTH * 2) so we can start with that value
	quadTextureSize = 512;
	neededSize = SNES_WIDTH * filterScale;
	while(quadTextureSize < neededSize)
		quadTextureSize *=2;

	if(!drawTexture) {
		glGenTextures(1,&drawTexture);
		glBindTexture(GL_TEXTURE_2D,drawTexture);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,quadTextureSize,quadTextureSize,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,NULL);
		if(pboFunctionsLoaded) {
			glGenBuffers(1,&drawBuffer);
			glBindBuffer(GL_PIXEL_UNPACK_BUFFER,drawBuffer);
			glBufferData(GL_PIXEL_UNPACK_BUFFER,quadTextureSize*quadTextureSize*2,NULL,GL_STREAM_DRAW);
		} else {
			noPboBuffer = new BYTE[quadTextureSize*quadTextureSize*2];
		}

		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
	}
	ApplyDisplayChanges();
}

void COpenGL::DestroyDrawSurface()
{
	if(drawTexture) {
		glDeleteTextures(1,&drawTexture);
		drawTexture = NULL;
	}
	if(drawBuffer) {
		glDeleteBuffers(1,&drawBuffer);
		drawBuffer = NULL;
	}
	if(noPboBuffer) {
		delete [] noPboBuffer;
		noPboBuffer = NULL;
	}
}

bool COpenGL::ChangeDrawSurfaceSize(unsigned int scale)
{
	filterScale = scale;

	DestroyDrawSurface();
	CreateDrawSurface();
	SetupVertices();
	return true;
}

void COpenGL::SetupVertices()
{
	vertices[0] = 0.0f;
    vertices[1] = 0.0f;
	vertices[2] = 1.0f;
    vertices[3] = 0.0f;
	vertices[4] = 1.0f;
	vertices[5] = 1.0f;
    vertices[6] = 0.0f;
	vertices[7] = 1.0f;

	float tX = (float)afterRenderWidth / (float)quadTextureSize;
	float tY = (float)afterRenderHeight / (float)quadTextureSize;

	texcoords[0] = 0.0f;
    texcoords[1] = tY;
    texcoords[2] = tX;
    texcoords[3] = tY;
    texcoords[4] = tX;
    texcoords[5] = 0.0f;
    texcoords[6] = 0.0f;
    texcoords[7] = 0.0f;
}

void COpenGL::Render(SSurface Src)
{
	SSurface Dst;
	RECT dstRect;
	unsigned int newFilterScale;
	GLenum error;

	if(!initDone) return;

	//create a new draw surface if the filter scale changes
	//at least factor 2 so we can display unscaled hi-res images
	newFilterScale = max(2,max(GetFilterScale(GUI.ScaleHiRes),GetFilterScale(GUI.Scale)));
	if(newFilterScale!=filterScale) {
		ChangeDrawSurfaceSize(newFilterScale);
	}

	if(pboFunctionsLoaded) {
		Dst.Surface = (unsigned char *)glMapBuffer(GL_PIXEL_UNPACK_BUFFER,GL_WRITE_ONLY);
	} else {
		Dst.Surface = noPboBuffer;
	}
	Dst.Height = quadTextureSize;
	Dst.Width = quadTextureSize;
	Dst.Pitch = quadTextureSize * 2;

	RenderMethod (Src, Dst, &dstRect);
	if(!Settings.AutoDisplayMessages) {
		WinSetCustomDisplaySurface((void *)Dst.Surface, Dst.Pitch/2, dstRect.right-dstRect.left, dstRect.bottom-dstRect.top, GetFilterScale(CurrentScale));
		S9xDisplayMessages ((uint16*)Dst.Surface, Dst.Pitch/2, dstRect.right-dstRect.left, dstRect.bottom-dstRect.top, GetFilterScale(CurrentScale));
	}

	if(pboFunctionsLoaded)
		glUnmapBuffer(GL_PIXEL_UNPACK_BUFFER);

	if(afterRenderHeight != dstRect.bottom || afterRenderWidth != dstRect.right) {
		afterRenderHeight = dstRect.bottom;
		afterRenderWidth = dstRect.right;
		ApplyDisplayChanges();
	}

	if (shader_type != OGL_SHADER_NONE) {
		GLint location;

		float inputSize[2] = { (float)afterRenderWidth, (float)afterRenderHeight };
		RECT windowSize;
		GetClientRect(hWnd,&windowSize);
		float outputSize[2] = {(float)(GUI.Stretch?windowSize.right:afterRenderWidth),
							(float)(GUI.Stretch?windowSize.bottom:afterRenderHeight) };
		float textureSize[2] = { (float)quadTextureSize, (float)quadTextureSize };

		if(shader_type == OGL_SHADER_GLSL) {
			location = glGetUniformLocation (shaderProgram, "rubyInputSize");
			glUniform2fv (location, 1, inputSize);

			location = glGetUniformLocation (shaderProgram, "rubyOutputSize");
			glUniform2fv (location, 1, outputSize);

			location = glGetUniformLocation (shaderProgram, "rubyTextureSize");
			glUniform2fv (location, 1, textureSize);
		} else if(shader_type == OGL_SHADER_CG) {
			CGparameter cgpModelViewProj = cgGetNamedParameter(cgVertexProgram, "modelViewProj");

			cgGLSetStateMatrixParameter(cgpModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

#define setProgram2fv(program,varname,floats)\
{\
	CGparameter cgp = cgGetNamedParameter(program, varname);\
	if(cgp)\
		cgGLSetParameter2fv(cgp,floats);\
}\

			setProgram2fv(cgFragmentProgram,"IN.video_size",inputSize);
			setProgram2fv(cgFragmentProgram,"IN.texture_size",textureSize);
			setProgram2fv(cgFragmentProgram,"IN.output_size",outputSize);
			setProgram2fv(cgVertexProgram,"IN.video_size",inputSize);
			setProgram2fv(cgVertexProgram,"IN.texture_size",textureSize);
			setProgram2fv(cgVertexProgram,"IN.output_size",outputSize);
		}
    }

	glPixelStorei(GL_UNPACK_ROW_LENGTH, quadTextureSize);
	glTexSubImage2D (GL_TEXTURE_2D,0,0,0,dstRect.right-dstRect.left,dstRect.bottom-dstRect.top,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,pboFunctionsLoaded?0:noPboBuffer);

	glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
	glClear(GL_COLOR_BUFFER_BIT);
	glDrawArrays (GL_QUADS, 0, 4);

	glFlush();
	SwapBuffers(hDC);
}

bool COpenGL::ChangeRenderSize(unsigned int newWidth, unsigned int newHeight)
{
	RECT displayRect=CalculateDisplayRect(afterRenderWidth,afterRenderHeight,newWidth,newHeight);
	glViewport(displayRect.left,newHeight-displayRect.bottom,displayRect.right-displayRect.left,displayRect.bottom-displayRect.top);
	SetupVertices();
	return true;
}

bool COpenGL::ApplyDisplayChanges(void)
{
	if(GUI.BilinearFilter) {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	} else {
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	}
	if(wglSwapIntervalEXT) {
		wglSwapIntervalEXT(GUI.Vsync?1:0);
	}
	if(GUI.shaderEnabled && GUI.OGLshaderFileName)
		SetShaders(GUI.OGLshaderFileName);
	else
		SetShaders(NULL);

	RECT windowSize;
	GetClientRect(hWnd,&windowSize);
	ChangeRenderSize(windowSize.right,windowSize.bottom);
	return true;
}

bool COpenGL::SetFullscreen(bool fullscreen)
{
	if(!initDone)
		return false;

	if(this->fullscreen==fullscreen)
		return true;

	this->fullscreen = fullscreen;

	if(fullscreen) {
		DEVMODE dmScreenSettings={0};
		dmScreenSettings.dmSize=sizeof(dmScreenSettings);
		dmScreenSettings.dmPelsWidth	= GUI.FullscreenMode.width;
		dmScreenSettings.dmPelsHeight	= GUI.FullscreenMode.height;
		dmScreenSettings.dmBitsPerPel	= GUI.FullscreenMode.depth;
		dmScreenSettings.dmDisplayFrequency = GUI.FullscreenMode.rate;
		dmScreenSettings.dmFields=DM_BITSPERPEL|DM_PELSWIDTH|DM_PELSHEIGHT|DM_DISPLAYFREQUENCY;
		if(ChangeDisplaySettings(&dmScreenSettings,CDS_FULLSCREEN)!=DISP_CHANGE_SUCCESSFUL) {
			this->fullscreen = false;
			return false;
		}
		ChangeRenderSize(GUI.FullscreenMode.width,GUI.FullscreenMode.height);
	} else {
		ChangeDisplaySettings(NULL,0);
	}

	

	return true;
}

void COpenGL::SetSnes9xColorFormat()
{
	GUI.ScreenDepth = 16;
	GUI.BlueShift = 0;
	GUI.GreenShift = 6;
	GUI.RedShift = 11;
	S9xSetRenderPixelFormat (RGB565);
	S9xBlit2xSaIFilterInit();
	S9xBlitHQ2xFilterInit();
	GUI.NeedDepthConvert = FALSE;
	GUI.DepthConverted = TRUE;
	return;
}

void COpenGL::EnumModes(std::vector<dMode> *modeVector)
{
	DISPLAY_DEVICE dd;
	dd.cb = sizeof(dd);
	DWORD dev = 0;
	int iMode = 0;
	dMode mode;

	while (EnumDisplayDevices(0, dev, &dd, 0))
	{
		if (!(dd.StateFlags & DISPLAY_DEVICE_MIRRORING_DRIVER) && (dd.StateFlags & DISPLAY_DEVICE_PRIMARY_DEVICE))
		{
			DEVMODE dm;
			ZeroMemory(&dm, sizeof(dm));
			dm.dmSize = sizeof(dm);
			iMode = 0;
			while(EnumDisplaySettings(dd.DeviceName,iMode,&dm)) {
				if(dm.dmBitsPerPel>=16) {
					mode.width = dm.dmPelsWidth;
					mode.height = dm.dmPelsHeight;
					mode.rate = dm.dmDisplayFrequency;
					mode.depth = dm.dmBitsPerPel;
					modeVector->push_back(mode);
				}
				iMode++;
			}
		}
		dev++;
	}
}

bool COpenGL::LoadPBOFunctions()
{
	if(pboFunctionsLoaded)
		return true;

	const char *extensions = (const char *) glGetString(GL_EXTENSIONS);

	if(extensions && strstr(extensions, "pixel_buffer_object")) {
		glGenBuffers = (PFNGLGENBUFFERSPROC)wglGetProcAddress("glGenBuffers");
		glBindBuffer = (PFNGLBINDBUFFERPROC)wglGetProcAddress("glBindBuffer");
		glBufferData = (PFNGLBUFFERDATAPROC)wglGetProcAddress("glBufferData");
		glDeleteBuffers = (PFNGLDELETEBUFFERSPROC)wglGetProcAddress("glDeleteBuffers");
		glMapBuffer = (PFNGLMAPBUFFERPROC)wglGetProcAddress("glMapBuffer");
		glUnmapBuffer = (PFNGLUNMAPBUFFERPROC)wglGetProcAddress("glUnmapBuffer");

		if(glGenBuffers && glBindBuffer && glBufferData && glDeleteBuffers && glMapBuffer) {
			pboFunctionsLoaded = true;
		}
		 
	}
	return pboFunctionsLoaded;
}

bool COpenGL::LoadShaderFunctions()
{
	if(shaderFunctionsLoaded)
		return true;

	const char *extensions = (const char *) glGetString(GL_EXTENSIONS);

    if(extensions && strstr(extensions, "fragment_program")) {
		glCreateProgram = (PFNGLCREATEPROGRAMPROC) wglGetProcAddress ("glCreateProgram");
		glCreateShader = (PFNGLCREATESHADERPROC) wglGetProcAddress ("glCreateShader");
		glCompileShader = (PFNGLCOMPILESHADERPROC) wglGetProcAddress ("glCompileShader");
		glDeleteShader = (PFNGLDELETESHADERPROC) wglGetProcAddress ("glDeleteShader");
		glDeleteProgram = (PFNGLDELETEPROGRAMPROC) wglGetProcAddress ("glDeleteProgram");
		glAttachShader = (PFNGLATTACHSHADERPROC) wglGetProcAddress ("glAttachShader");
		glDetachShader = (PFNGLDETACHSHADERPROC) wglGetProcAddress ("glDetachShader");
		glLinkProgram = (PFNGLLINKPROGRAMPROC) wglGetProcAddress ("glLinkProgram");
		glUseProgram = (PFNGLUSEPROGRAMPROC) wglGetProcAddress ("glUseProgram");
		glShaderSource = (PFNGLSHADERSOURCEPROC) wglGetProcAddress ("glShaderSource");
		glGetUniformLocation = (PFNGLGETUNIFORMLOCATIONPROC) wglGetProcAddress ("glGetUniformLocation");
		glUniform2fv = (PFNGLUNIFORM2FVPROC) wglGetProcAddress ("glUniform2fv");

		if(glCreateProgram      &&
		   glCreateShader       &&
		   glCompileShader      &&
		   glDeleteShader       &&
		   glDeleteProgram      &&
		   glAttachShader       &&
		   glDetachShader       &&
		   glLinkProgram        &&
		   glUseProgram         &&
		   glShaderSource       &&
		   glGetUniformLocation &&
		   glUniform2fv) {
			   shaderFunctionsLoaded = true;
		}
	}
	return shaderFunctionsLoaded;
}

bool COpenGL::SetShaders(const TCHAR *file)
{
	SetShadersCG(NULL);
	SetShadersGLSL(NULL);
	shader_type = OGL_SHADER_NONE;
	if(file!=NULL && lstrlen(file)>3 && _tcsncicmp(&file[lstrlen(file)-3],TEXT(".cg"),3)==0) {
		return SetShadersCG(file);
	} else {
		return SetShadersGLSL(file);
	}
}

void COpenGL::checkForCgError(const char *situation)
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

bool COpenGL::SetShadersCG(const TCHAR *file)
{
	TCHAR errorMsg[MAX_PATH + 50];
	HRESULT hr;
	CGprofile vertexProfile, fragmentProfile;

	if(cgFragmentProgram) {
		cgDestroyProgram(cgFragmentProgram);
		cgFragmentProgram = NULL;
	}
	if(cgVertexProgram) {
		cgDestroyProgram(cgVertexProgram);
		cgVertexProgram = NULL;
	}

	if(cgAvailable) {
		vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
		fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

		cgGLDisableProfile(vertexProfile);
		cgGLDisableProfile(fragmentProfile);
	}

	if (file == NULL || *file==TEXT('\0'))
		return true;

	if(!cgAvailable) {
		MessageBox(NULL, TEXT("The CG runtime is unavailable, CG shaders will not run.\nConsult the snes9x readme for information on how to obtain the runtime."), TEXT("CG Error"),
			MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

	cgGLSetOptimalOptions(vertexProfile);
	cgGLSetOptimalOptions(fragmentProfile);

	char *fileContents = ReadShaderFileContents(file);
	if(!fileContents)
		return false;

	cgVertexProgram = cgCreateProgram( cgContext, CG_SOURCE, fileContents,
						vertexProfile, "main_vertex", NULL);

	checkForCgError("Compiling vertex program");

	cgFragmentProgram = cgCreateProgram( cgContext, CG_SOURCE, fileContents,
						fragmentProfile, "main_fragment", NULL);

	checkForCgError("Compiling fragment program");

	delete [] fileContents;

	if(!cgVertexProgram || !cgFragmentProgram) {
		return false;
	}

	if(cgVertexProgram) {
		cgGLEnableProfile(vertexProfile);
		cgGLLoadProgram(cgVertexProgram);
		cgGLBindProgram(cgVertexProgram);
	}
	if(cgFragmentProgram) {
		cgGLEnableProfile(fragmentProfile);
		cgGLLoadProgram(cgFragmentProgram);
		cgGLBindProgram(cgFragmentProgram);
	}

	shader_type = OGL_SHADER_CG;

	return true;
}

bool COpenGL::SetShadersGLSL(const TCHAR *glslFileName)
{
	char *fragment=NULL, *vertex=NULL;
	IXMLDOMDocument * pXMLDoc = NULL;
	IXMLDOMElement * pXDE = NULL;
	IXMLDOMNode * pXDN = NULL;
	HRESULT hr;
	BSTR queryString, nodeContent;

	TCHAR errorMsg[MAX_PATH + 50];

	if(fragmentShader) {
		glDetachShader(shaderProgram,fragmentShader);
		glDeleteShader(fragmentShader);
		fragmentShader = 0;
	}
	if(vertexShader) {
		glDetachShader(shaderProgram,vertexShader);
		glDeleteShader(vertexShader);
		vertexShader = 0;
	}
	if(shaderProgram) {
		glUseProgram(0);
		glDeleteProgram(shaderProgram);
		shaderProgram = 0;
	}

	if(glslFileName==NULL || *glslFileName==TEXT('\0'))
		return true;

	if(!LoadShaderFunctions()) {
        MessageBox(NULL, TEXT("Unable to load OpenGL shader functions"), TEXT("Shader Loading Error"),
			MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

	hr = CoCreateInstance(CLSID_DOMDocument,NULL,CLSCTX_INPROC_SERVER,IID_PPV_ARGS(&pXMLDoc));

	if(FAILED(hr)) {
		MessageBox(NULL, TEXT("Error creating XML Parser"), TEXT("Shader Loading Error"),
			MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

	VARIANT fileName;
	VARIANT_BOOL ret;
	fileName.vt = VT_BSTR;
#ifdef UNICODE
	fileName.bstrVal = SysAllocString(glslFileName);
#else
	wchar_t tempfilename[MAX_PATH];
	MultiByteToWideChar(CP_UTF8,0,glslFileName,-1,tempfilename,MAX_PATH);
	fileName.bstrVal = SysAllocString(tempfilename);
#endif
	hr = pXMLDoc->load(fileName,&ret);
	SysFreeString(fileName.bstrVal);

	if(FAILED(hr) || hr==S_FALSE) {
		_stprintf(errorMsg,TEXT("Error loading GLSL shader file:\n%s"),glslFileName);
		MessageBox(NULL, errorMsg, TEXT("Shader Loading Error"), MB_OK|MB_ICONEXCLAMATION);
		pXMLDoc->Release();
		return false;
	}

	VARIANT attributeValue;
	BSTR attributeName;

	hr = pXMLDoc->get_documentElement(&pXDE);
	if(FAILED(hr) || hr==S_FALSE) {
		_stprintf(errorMsg,TEXT("Error loading root element from file:\n%s"),glslFileName);
		MessageBox(NULL, errorMsg, TEXT("Shader Loading Error"), MB_OK|MB_ICONEXCLAMATION);
		pXMLDoc->Release();
		return false;
	}

	attributeName=SysAllocString(L"language");
	pXDE->getAttribute(attributeName,&attributeValue);
	SysFreeString(attributeName);
	pXDE->Release();

	if(attributeValue.vt!=VT_BSTR || lstrcmpiW(attributeValue.bstrVal,L"glsl")) {
		_stprintf(errorMsg,TEXT("Shader language is <%s>, expected <GLSL> in file:\n%s"),attributeValue.bstrVal,glslFileName);
		MessageBox(NULL, errorMsg, TEXT("Shader Loading Error"), MB_OK|MB_ICONEXCLAMATION);
		if(attributeValue.vt==VT_BSTR) SysFreeString(attributeValue.bstrVal);
		pXMLDoc->Release();
		return false;
	}
	if(attributeValue.vt==VT_BSTR) SysFreeString(attributeValue.bstrVal);

	queryString=SysAllocString(L"/shader/fragment");
	hr = pXMLDoc->selectSingleNode(queryString,&pXDN);
	SysFreeString(queryString);

	if(hr == S_OK) {
		hr = pXDN->get_text(&nodeContent);
		if(hr == S_OK) {
			int requiredChars = WideCharToMultiByte(CP_ACP,0,nodeContent,-1,fragment,0,NULL,NULL);
			fragment = new char[requiredChars];
			WideCharToMultiByte(CP_UTF8,0,nodeContent,-1,fragment,requiredChars,NULL,NULL);
		}
		SysFreeString(nodeContent);
		pXDN->Release();
		pXDN = NULL;
	}

	queryString=SysAllocString(L"/shader/vertex");
	hr = pXMLDoc->selectSingleNode(queryString,&pXDN);
	SysFreeString(queryString);

	if(hr == S_OK) {
		hr = pXDN->get_text(&nodeContent);
		if(hr == S_OK) {
			int requiredChars = WideCharToMultiByte(CP_ACP,0,nodeContent,-1,vertex,0,NULL,NULL);
			vertex = new char[requiredChars];
			WideCharToMultiByte(CP_UTF8,0,nodeContent,-1,vertex,requiredChars,NULL,NULL);
		}
		SysFreeString(nodeContent);
		pXDN->Release();
		pXDN = NULL;
	}

	pXMLDoc->Release();

	if(!fragment && !vertex) {
		_stprintf(errorMsg,TEXT("No vertex or fragment program in file:\n%s"),glslFileName);
		MessageBox(NULL, errorMsg, TEXT("Shader Loading Error"), MB_OK|MB_ICONEXCLAMATION);
		return false;
	}

    shaderProgram = glCreateProgram ();
	if(vertex) {
		vertexShader = glCreateShader (GL_VERTEX_SHADER);
		glShaderSource(vertexShader, 1, (const GLchar **)&vertex, NULL);
		glCompileShader(vertexShader);
		glAttachShader(shaderProgram, vertexShader);
		delete[] vertex;
	}
	if(fragment) {
		fragmentShader = glCreateShader (GL_FRAGMENT_SHADER);
		glShaderSource(fragmentShader, 1, (const GLchar **)&fragment, NULL);    
		glCompileShader(fragmentShader);    
		glAttachShader(shaderProgram, fragmentShader);
		delete[] fragment;
	}

    glLinkProgram(shaderProgram);
    glUseProgram(shaderProgram);

	shader_type = OGL_SHADER_GLSL;

    return true;
}
