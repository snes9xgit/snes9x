/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef COPENGL_H
#define COPENGL_H

#include <windows.h>
#include "gl_core_3_1.h"
#include "cgFunctions.h"
#include "CGLCG.h"
#include "../shaders/glsl.h"

#include "wglext.h"
#include "IS9xDisplayOutput.h"

enum current_ogl_shader_type { OGL_SHADER_NONE, OGL_SHADER_GLSL, OGL_SHADER_CG, OGL_SHADER_GLSL_OLD};

class COpenGL : public IS9xDisplayOutput
{
private:
	HDC					hDC;
	HGLRC				hRC;
	HWND				hWnd;
	GLuint				drawTexture;
	GLuint				drawBuffer;
	GLfloat				vertices[8];
    GLfloat				texcoords[8];
	unsigned char *		noPboBuffer;
	TCHAR				currentShaderFile[MAX_PATH];

	int frameCount;

	bool initDone;
	bool fullscreen;
	unsigned int outTextureWidth;
	unsigned int outTextureHeight;
	unsigned int afterRenderWidth, afterRenderHeight;

	bool shaderFunctionsLoaded;

	bool pboFunctionsLoaded;

	CGcontext cgContext;
	CGprogram cgVertexProgram, cgFragmentProgram;
	current_ogl_shader_type shader_type;
	bool cgAvailable;

	CGLCG *cgShader;
    GLSLShader *glslShader;

	GLuint shaderProgram;
    GLuint vertexShader;
    GLuint fragmentShader;

	PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

	bool SetShaders(const TCHAR *file);
	void checkForCgError(const char *situation);
	bool SetShadersCG(const TCHAR *file);
	bool SetShadersGLSL(const TCHAR *glslFileName);
	bool SetShadersGLSL_OLD(const TCHAR *glslFileName);
	bool LoadShaderFunctions();
	bool LoadPBOFunctions();
	void CreateDrawSurface(unsigned int width, unsigned int height);
	void DestroyDrawSurface(void);
	bool ChangeDrawSurfaceSize(unsigned int width, unsigned int height);
	void SetupVertices();
    bool ShaderAvailable();
    bool NPOTAvailable();

public:
	COpenGL();
	~COpenGL();
	bool Initialize(HWND hWnd);
	void DeInitialize();
	void Render(SSurface Src);
	bool ChangeRenderSize(unsigned int newWidth, unsigned int newHeight);
	bool ApplyDisplayChanges(void);
	bool SetFullscreen(bool fullscreen);
	void SetSnes9xColorFormat(void);
	void EnumModes(std::vector<dMode> *modeVector);
    void SetSwapInterval(int frames);
	GLSLShader *GetActiveShader()
	{
		return glslShader;
	}
};


#endif
