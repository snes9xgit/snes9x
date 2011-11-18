#include "CGLCG.h"
#include "wsnes9x.h"
#include "win32_display.h"
#include <png.h>

#ifndef max
#define max(a, b) (((a) > (b)) ? (a) : (b))
#endif

float npot(float desired)
{
	float out=512.0;
	while(out<desired)
		out*=2;
	return out;
}

const GLfloat CGLCG::lut_coords[8] = {0, 0, 1, 0, 1, 1, 0, 1};

CGLCG::CGLCG(CGcontext cgContext)
{
	this->cgContext = cgContext;
	fboFunctionsLoaded = FALSE;
	ClearPasses();
	LoadFBOFunctions();
	frameCnt=0;
	prevTex=0;
}

CGLCG::~CGLCG(void)
{
	LoadShader(NULL);
}

void CGLCG::ClearPasses()
{
	/* clean up cg programs, fbos and textures from all regular passes
	   pass 0 is the orignal texture, so ignore that
	*/
	if(shaderPasses.size()>1) {
		for(glPassVector::iterator it=(shaderPasses.begin()+1);it!=shaderPasses.end();it++) {
			if(it->cgFragmentProgram)
				cgDestroyProgram(it->cgFragmentProgram);
			if(it->cgVertexProgram)
				cgDestroyProgram(it->cgVertexProgram);
			if(it->fbo)
				glDeleteFramebuffers(1,&it->fbo);
			if(it->tex)
				glDeleteTextures(1,&it->tex);
		}
	}
	for(glLutVector::iterator it=lookupTextures.begin();it!=lookupTextures.end();it++) {
		if(it->tex)
			glDeleteTextures(1,&it->tex);
	}
	shaderPasses.clear();
	lookupTextures.clear();

	shaderLoaded = false;
}

bool CGLCG::LoadFBOFunctions()
{
	if(fboFunctionsLoaded)
		return true;

	const char *extensions = (const char *) glGetString(GL_EXTENSIONS);

	if(extensions && strstr(extensions, "framebuffer_object")) {
		glGenFramebuffers = (PFNGLGENFRAMEBUFFERSPROC)wglGetProcAddress("glGenFramebuffers");
		glDeleteFramebuffers = (PFNGLDELETEFRAMEBUFFERSPROC)wglGetProcAddress("glDeleteFramebuffers");
		glBindFramebuffer = (PFNGLBINDFRAMEBUFFERPROC)wglGetProcAddress("glBindFramebuffer");
		glFramebufferTexture2D = (PFNGLFRAMEBUFFERTEXTURE2DPROC)wglGetProcAddress("glFramebufferTexture2D");
		glCheckFramebufferStatus = (PFNGLCHECKFRAMEBUFFERSTATUSPROC)wglGetProcAddress("glCheckFramebufferStatus");
		glClientActiveTexture = (PFNGLACTIVETEXTUREPROC)wglGetProcAddress("glClientActiveTexture");

		if(glGenFramebuffers && glDeleteFramebuffers && glBindFramebuffer && glFramebufferTexture2D && glClientActiveTexture) {
			fboFunctionsLoaded = true;
		}
		 
	}
	return fboFunctionsLoaded;
}

void CGLCG::checkForCgError(const char *situation)
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

#define IS_SLASH(x) ((x) == TEXT('\\') || (x) == TEXT('/'))

bool CGLCG::LoadShader(const TCHAR *shaderFile)
{
	CCGShader cgShader;
	TCHAR shaderPath[MAX_PATH];
	TCHAR tempPath[MAX_PATH];
	CGprofile vertexProfile, fragmentProfile;
	GLenum error;

	if(!fboFunctionsLoaded) {
		MessageBox(NULL, TEXT("Your OpenGL graphics driver does not support framebuffer objects.\nYou will not be able to use CG shaders in OpenGL mode."), TEXT("CG Error"),
			MB_OK|MB_ICONEXCLAMATION);
        return false;
    }

	vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

	cgGLDisableProfile(vertexProfile);
	cgGLDisableProfile(fragmentProfile);

	ClearPasses();

	if(prevTex) {
		glDeleteTextures(1,&prevTex);
		prevTex=0;
	}

	if (shaderFile == NULL || *shaderFile==TEXT('\0'))
		return true;

	lstrcpy(shaderPath,shaderFile);
	for(int i=lstrlen(shaderPath); i>=0; i--){
		if(IS_SLASH(shaderPath[i])){
			shaderPath[i]=TEXT('\0');
			break;
		}
	}

	SetCurrentDirectory(shaderPath);
	if(!cgShader.LoadShader(_tToChar(shaderFile)))
		return false;

	cgGLSetOptimalOptions(vertexProfile);
	cgGLSetOptimalOptions(fragmentProfile);

	/* insert dummy pass that will contain the original texture
	*/
	shaderPasses.push_back(shaderPass());

	for(CCGShader::passVector::iterator it=cgShader.shaderPasses.begin();
			it!=cgShader.shaderPasses.end();it++) {
		shaderPass pass;

		pass.scaleParams = it->scaleParams;
		/* if this is the last pass (the only one that can have CG_SCALE_NONE)
		   and no filter has been set use the GUI setting
		*/
		if(pass.scaleParams.scaleTypeX==CG_SCALE_NONE && !it->filterSet) {
			pass.linearFilter = GUI.BilinearFilter;
		} else {
			pass.linearFilter = it->linearFilter;
		}

		// paths in the meta file can be relative
		_tfullpath(tempPath,_tFromChar(it->cgShaderFile),MAX_PATH);
		char *fileContents = ReadShaderFileContents(tempPath);
		if(!fileContents)
			return false;

		pass.cgVertexProgram = cgCreateProgram( cgContext, CG_SOURCE, fileContents,
						vertexProfile, "main_vertex", NULL);

		checkForCgError("Compiling vertex program");

		pass.cgFragmentProgram = cgCreateProgram( cgContext, CG_SOURCE, fileContents,
							fragmentProfile, "main_fragment", NULL);

		checkForCgError("Compiling fragment program");

		delete [] fileContents;
		if(!pass.cgVertexProgram || !pass.cgFragmentProgram) {
			return false;
		}
		cgGLLoadProgram(pass.cgVertexProgram);
		cgGLLoadProgram(pass.cgFragmentProgram);

		/* generate framebuffer and texture for this pass and apply
		   default texture settings
		*/
		glGenFramebuffers(1,&pass.fbo);
		glGenTextures(1,&pass.tex);
		glBindTexture(GL_TEXTURE_2D,pass.tex);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

		shaderPasses.push_back(pass);
	}

	for(std::vector<CCGShader::lookupTexture>::iterator it=cgShader.lookupTextures.begin();it!=cgShader.lookupTextures.end();it++) {		
		lookupTexture tex;
		strcpy(tex.id,it->id);

		/* generate texture for the lut and apply specified filter setting
		*/
		glGenTextures(1,&tex.tex);
		glBindTexture(GL_TEXTURE_2D,tex.tex);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, it->linearfilter?GL_LINEAR:GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, it->linearfilter?GL_LINEAR:GL_NEAREST);

		_tfullpath(tempPath,_tFromChar(it->texturePath),MAX_PATH);

		// simple file extension png/tga decision
		int strLen = strlen(it->texturePath);
		if(strLen>4) {
			if(!strcasecmp(&it->texturePath[strLen-4],".png")) {
				int width, height;
				bool hasAlpha;
				GLubyte *texData;
				if(loadPngImage(tempPath,width,height,hasAlpha,&texData)) {
					glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
					glTexImage2D(GL_TEXTURE_2D, 0, hasAlpha ? 4 : 3, width,
						height, 0, hasAlpha ? GL_RGBA : GL_RGB, GL_UNSIGNED_BYTE, texData);
					free(texData);
				}
			} else if(!strcasecmp(&it->texturePath[strLen-4],".tga")) {
				STGA stga;
				if(loadTGA(tempPath,stga)) {
					glPixelStorei(GL_UNPACK_ROW_LENGTH, stga.width);
					glTexImage2D(GL_TEXTURE_2D, 0, 4, stga.width,
						stga.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, stga.data);
				}
			}
		}
		lookupTextures.push_back(tex);
	}

	/* enable texture unit 1 for the lookup textures
	*/
	glClientActiveTexture(GL_TEXTURE1);
	glEnableClientState(GL_TEXTURE_COORD_ARRAY);
	glTexCoordPointer(2,GL_FLOAT,0,lut_coords);
	glClientActiveTexture(GL_TEXTURE0);

	/* generate a texture that we can swap with the original texture
	   and pass back to the main opengl code
	*/
	glGenTextures(1,&prevTex);
	glBindTexture(GL_TEXTURE_2D,prevTex);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,512,512,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,NULL);
	glBindTexture(GL_TEXTURE_2D,0);
	prevTexSize.x = prevTexSize.y = prevTexImageSize.x = prevTexImageSize.y = 0;
	memset(prevTexCoords,0,sizeof(prevTexCoords));

	shaderLoaded = true;
	
	return true;
}

void CGLCG::setTexCoords(int pass,xySize inputSize,xySize textureSize,bool topdown)
{
	float tX = inputSize.x / textureSize.x;
	float tY = inputSize.y / textureSize.y;

	// last pass uses top-down coordinates, all others bottom-up
	if(topdown) {
		shaderPasses[pass].texcoords[0] = 0.0f;
		shaderPasses[pass].texcoords[1] = tY;
		shaderPasses[pass].texcoords[2] = tX;
		shaderPasses[pass].texcoords[3] = tY;
		shaderPasses[pass].texcoords[4] = tX;
		shaderPasses[pass].texcoords[5] = 0.0f;
		shaderPasses[pass].texcoords[6] = 0.0f;
		shaderPasses[pass].texcoords[7] = 0.0f;
	} else {
		shaderPasses[pass].texcoords[0] = 0.0f;
		shaderPasses[pass].texcoords[1] = 0.0f;
		shaderPasses[pass].texcoords[2] = tX;
		shaderPasses[pass].texcoords[3] = 0.0f;
		shaderPasses[pass].texcoords[4] = tX;
		shaderPasses[pass].texcoords[5] = tY;
		shaderPasses[pass].texcoords[6] = 0.0f;
		shaderPasses[pass].texcoords[7] = tY;
	}

	glTexCoordPointer(2, GL_FLOAT, 0, shaderPasses[pass].texcoords);
}

void CGLCG::Render(GLuint &origTex, xySize textureSize, xySize inputSize, xySize viewportSize, xySize windowSize)
{
	GLenum error;
	frameCnt++;
	CGprofile vertexProfile, fragmentProfile;

	if(!shaderLoaded)
		return;

	vertexProfile = cgGLGetLatestProfile(CG_GL_VERTEX);
	fragmentProfile = cgGLGetLatestProfile(CG_GL_FRAGMENT);

	cgGLEnableProfile(vertexProfile);
	cgGLEnableProfile(fragmentProfile);	

	/* set up our dummy pass for easier loop code
	*/
	shaderPasses[0].tex = origTex;
	shaderPasses[0].outputSize = inputSize;
	shaderPasses[0].textureSize = textureSize;

	/* loop through all real passes
	*/
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
				shaderPasses[i].outputSize.y = (double)shaderPasses[i].scaleParams.absX;
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
		/* use next power of two in both directions
		*/
		float texSize = npot(max(shaderPasses[i].outputSize.x,shaderPasses[i].outputSize.y));
		shaderPasses[i].textureSize.x = shaderPasses[i].textureSize.y = texSize;

		/* set size of output texture
		*/
		glBindTexture(GL_TEXTURE_2D,shaderPasses[i].tex);
		glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,(unsigned int)shaderPasses[i].textureSize.x,
			(unsigned int)shaderPasses[i].textureSize.y,0,GL_RGBA,GL_UNSIGNED_INT_8_8_8_8,NULL);

		/* viewport determines the area we render into the output texture
		*/
		glViewport(0,0,shaderPasses[i].outputSize.x,shaderPasses[i].outputSize.y);

		/* set up framebuffer and attach output texture
		*/
		glBindFramebuffer(GL_FRAMEBUFFER,shaderPasses[i].fbo);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, shaderPasses[i].tex, 0);

		/* set up input texture (output of previous pass) and apply filter settings
		*/
		glBindTexture(GL_TEXTURE_2D,shaderPasses[i-1].tex);
		glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)shaderPasses[i-1].textureSize.x);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
			shaderPasses[i].linearFilter?GL_LINEAR:GL_NEAREST);
		glTexParameteri (GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
			shaderPasses[i].linearFilter?GL_LINEAR:GL_NEAREST);

		/* calculate tex coords first since we pass them to the shader
		*/
		setTexCoords(i,shaderPasses[i-1].outputSize,shaderPasses[i-1].textureSize);

		setShaderVars(i);

		cgGLBindProgram(shaderPasses[i].cgVertexProgram);
		cgGLBindProgram(shaderPasses[i].cgFragmentProgram);

		glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
		glClear(GL_COLOR_BUFFER_BIT);
		glDrawArrays (GL_QUADS, 0, 4);

	}

	/* disable framebuffer
	*/
	glBindFramebuffer(GL_FRAMEBUFFER,0);

	/* switch original and prev texture and make sure the
	   new original texture has the same size as the old one
	*/
	origTex = prevTex;
	prevTex = shaderPasses[0].tex;
	prevTexSize.x = textureSize.x;
	prevTexSize.y = textureSize.y;
	prevTexImageSize.x = inputSize.x;
	prevTexImageSize.y = inputSize.y;
	memcpy(prevTexCoords,shaderPasses[1].texcoords,sizeof(prevTexCoords));
	glBindTexture(GL_TEXTURE_2D,origTex);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGB,textureSize.x,textureSize.y,0,GL_RGB,GL_UNSIGNED_SHORT_5_6_5,NULL);

	/* bind output of last pass to be rendered on the backbuffer
	*/
	glBindTexture(GL_TEXTURE_2D,shaderPasses.back().tex);
	glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)shaderPasses.back().textureSize.x);

	/* calculate and apply viewport and texture coordinates to
	   that will be used in the main ogl code
	*/
	RECT displayRect=CalculateDisplayRect(shaderPasses.back().outputSize.x,shaderPasses.back().outputSize.y,windowSize.x,windowSize.y);
	glViewport(displayRect.left,windowSize.y-displayRect.bottom,displayRect.right-displayRect.left,displayRect.bottom-displayRect.top);	
	setTexCoords(shaderPasses.size()-1,shaderPasses.back().outputSize,shaderPasses.back().textureSize,true);

	/* render to backbuffer without shaders
	*/
	cgGLDisableProfile(vertexProfile);
	cgGLDisableProfile(fragmentProfile);
}

void CGLCG::setShaderVars(int pass)
{
	/* mvp paramater
	*/
	CGparameter cgpModelViewProj = cgGetNamedParameter(shaderPasses[pass].cgVertexProgram, "modelViewProj");
	cgGLSetStateMatrixParameter(cgpModelViewProj, CG_GL_MODELVIEW_PROJECTION_MATRIX, CG_GL_MATRIX_IDENTITY);

#define setProgram2fv(pass,varname,floats)\
{\
	CGparameter cgpf = cgGetNamedParameter(shaderPasses[pass].cgFragmentProgram, varname);\
	CGparameter cgpv = cgGetNamedParameter(shaderPasses[pass].cgVertexProgram, varname);\
	if(cgpf)\
		cgGLSetParameter2fv(cgpf,floats);\
	if(cgpv)\
		cgGLSetParameter2fv(cgpv,floats);\
}\

#define setProgram1f(pass,varname,val)\
{\
	CGparameter cgpf = cgGetNamedParameter(shaderPasses[pass].cgFragmentProgram, varname);\
	CGparameter cgpv = cgGetNamedParameter(shaderPasses[pass].cgVertexProgram, varname);\
	if(cgpf)\
		cgGLSetParameter1f(cgpf,val);\
	if(cgpv)\
		cgGLSetParameter1f(cgpv,val);\
}\

#define setTextureParameter(pass,varname,val)\
{\
	CGparameter cgpf = cgGetNamedParameter(shaderPasses[pass].cgFragmentProgram, varname);\
	if(cgpf) {\
		cgGLSetTextureParameter(cgpf,val);\
		cgGLEnableTextureParameter(cgpf);\
	}\
}\

#define setTexCoordsParameter(pass,varname,val)\
{\
	CGparameter cgpv = cgGetNamedParameter(shaderPasses[pass].cgVertexProgram, varname);\
	if(cgpv) {\
		cgGLSetParameterPointer(cgpv, 2, GL_FLOAT, 0, val);\
        cgGLEnableClientState(cgpv);\
	}\
}\

	/* IN paramater
	*/
	float inputSize[2] = {shaderPasses[pass-1].outputSize.x,shaderPasses[pass-1].outputSize.y};
	float textureSize[2] = {shaderPasses[pass-1].textureSize.x,shaderPasses[pass-1].textureSize.y};
	float outputSize[2] = {shaderPasses[pass].outputSize.x,shaderPasses[pass].outputSize.y};

	setProgram2fv(pass,"IN.video_size",inputSize);
	setProgram2fv(pass,"IN.texture_size",textureSize);
	setProgram2fv(pass,"IN.output_size",outputSize);
	setProgram1f(pass,"IN.frame_count",(float)frameCnt);

	/* ORIG parameter
	*/
	float orig_videoSize[2] = {shaderPasses[0].outputSize.x,shaderPasses[0].outputSize.y};
	float orig_textureSize[2] = {shaderPasses[0].textureSize.x,shaderPasses[0].textureSize.y};
	
	setProgram2fv(pass,"ORIG.video_size",orig_videoSize);
	setProgram2fv(pass,"ORIG.texture_size",orig_textureSize);
	setTextureParameter(pass,"ORIG.texture",shaderPasses[0].tex);
	setTexCoordsParameter(pass,"ORIG.tex_coord",shaderPasses[1].texcoords);

	/* PREV parameter
	*/
	float prev_videoSize[2] = {prevTexImageSize.x,prevTexImageSize.y};
	float prev_textureSize[2] = {prevTexSize.x,prevTexSize.y};

	setProgram2fv(pass,"PREV.video_size",prev_videoSize);
	setProgram2fv(pass,"PREV.texture_size",prev_textureSize);
	setTextureParameter(pass,"PREV.texture",prevTex);
	setTexCoordsParameter(pass,"PREV.tex_coord",prevTexCoords);

	/* LUT parameters
	*/
	for(int i=0;i<lookupTextures.size();i++) {
		setTextureParameter(pass,lookupTextures[i].id,lookupTextures[i].tex);
	}

	/* PASSX parameters, only for third pass and up
	*/
	if(pass>2) {
		for(int i=1;i<pass-1;i++) {
			char varname[100];
			float pass_videoSize[2] = {shaderPasses[i].outputSize.x,shaderPasses[i].outputSize.y};
			float pass_textureSize[2] = {shaderPasses[i].textureSize.x,shaderPasses[i].textureSize.y};
			sprintf(varname,"PASS%d.video_size",i);
			setProgram2fv(pass,varname,pass_videoSize);
			sprintf(varname,"PASS%d.texture_size",i);
			setProgram2fv(pass,varname,pass_textureSize);
			sprintf(varname,"PASS%d.texture",i);
			setTextureParameter(pass,varname,shaderPasses[i].tex);
			sprintf(varname,"PASS%d.tex_coord",i);
			setTexCoordsParameter(pass,varname,shaderPasses[i+1].texcoords);
		}
	}
}

#ifdef HAVE_LIBPNG
bool CGLCG::loadPngImage(const TCHAR *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData) {
    png_structp png_ptr;
    png_infop info_ptr;
    unsigned int sig_read = 0;
    int color_type, interlace_type;
    FILE *fp;

	if ((fp = _tfopen(name, TEXT("rb"))) == NULL)
        return false;

    /* Create and initialize the png_struct
     * with the desired error handler
     * functions.  If you want to use the
     * default stderr and longjump method,
     * you can supply NULL for the last
     * three parameters.  We also supply the
     * the compiler header file version, so
     * that we know if the application
     * was compiled with a compatible version
     * of the library.  REQUIRED
     */
    png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING,
            NULL, NULL, NULL);

    if (png_ptr == NULL) {
        fclose(fp);
        return false;
    }

    /* Allocate/initialize the memory
     * for image information.  REQUIRED. */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fclose(fp);
        png_destroy_read_struct(&png_ptr, png_infopp_NULL, png_infopp_NULL);
        return false;
    }

    /* Set error handling if you are
     * using the setjmp/longjmp method
     * (this is the normal method of
     * doing things with libpng).
     * REQUIRED unless you  set up
     * your own error handlers in
     * the png_create_read_struct()
     * earlier.
     */
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* Free all of the memory associated
         * with the png_ptr and info_ptr */
        png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);
        fclose(fp);
        /* If we get here, we had a
         * problem reading the file */
        return false;
    }

    /* Set up the output control if
     * you are using standard C streams */
    png_init_io(png_ptr, fp);

    /* If we have already
     * read some of the signature */
    png_set_sig_bytes(png_ptr, sig_read);

    /*
     * If you have enough memory to read
     * in the entire image at once, and
     * you need to specify only
     * transforms that can be controlled
     * with one of the PNG_TRANSFORM_*
     * bits (this presently excludes
     * dithering, filling, setting
     * background, and doing gamma
     * adjustment), then you can read the
     * entire image (including pixels)
     * into the info structure with this
     * call
     *
     * PNG_TRANSFORM_STRIP_16 |
     * PNG_TRANSFORM_PACKING  forces 8 bit
     * PNG_TRANSFORM_EXPAND forces to
     *  expand a palette into RGB
     */
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_STRIP_16 | PNG_TRANSFORM_PACKING | PNG_TRANSFORM_EXPAND, png_voidp_NULL);

    outWidth = info_ptr->width;
    outHeight = info_ptr->height;
    switch (info_ptr->color_type) {
        case PNG_COLOR_TYPE_RGBA:
            outHasAlpha = true;
            break;
        case PNG_COLOR_TYPE_RGB:
            outHasAlpha = false;
            break;
        default:
            png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
            fclose(fp);
            return false;
    }
    unsigned int row_bytes = png_get_rowbytes(png_ptr, info_ptr);
    *outData = (unsigned char*) malloc(row_bytes * outHeight);

    png_bytepp row_pointers = png_get_rows(png_ptr, info_ptr);

    for (int i = 0; i < outHeight; i++) {
        memcpy(*outData+(row_bytes * i), row_pointers[i], row_bytes);
    }

    /* Clean up after the read,
     * and free any memory allocated */
    png_destroy_read_struct(&png_ptr, &info_ptr, png_infopp_NULL);

    /* Close the file */
    fclose(fp);

    /* That's it */
    return true;
}
#else
bool CGLCG::loadPngImage(const TCHAR *name, int &outWidth, int &outHeight, bool &outHasAlpha, GLubyte **outData) {
	/* No PNG support */
	return false;
}
#endif

bool CGLCG::loadTGA(const TCHAR *filename, STGA& tgaFile)
{
	FILE *file;
	unsigned char type[4];
	unsigned char info[6];

        file = _tfopen(filename, TEXT("rb"));

        if (!file)
		return false;

	fread (&type, sizeof (char), 3, file);
	fseek (file, 12, SEEK_SET);
	fread (&info, sizeof (char), 6, file);

	//image type either 2 (color) or 3 (greyscale)
	if (type[1] != 0 || (type[2] != 2 && type[2] != 3))
	{
		fclose(file);
		return false;
	}

	tgaFile.width = info[0] + info[1] * 256;
	tgaFile.height = info[2] + info[3] * 256;
	tgaFile.byteCount = info[4] / 8;

	if (tgaFile.byteCount != 3 && tgaFile.byteCount != 4) {
		fclose(file);
		return false;
	}

	long imageSize = tgaFile.width * tgaFile.height * tgaFile.byteCount;

	//allocate memory for image data
	unsigned char *tempBuf = new unsigned char[imageSize];
	tgaFile.data = new unsigned char[tgaFile.width * tgaFile.height * 4];

	//read in image data
	fread(tempBuf, sizeof(unsigned char), imageSize, file);

	//swap line order and convert to RBGA
	for(int i=0;i<tgaFile.height;i++) {
		unsigned char* source = tempBuf + tgaFile.width * (tgaFile.height - 1 - i) * tgaFile.byteCount;
		unsigned char* destination = tgaFile.data + tgaFile.width * i * 4;
		for(int j=0;j<tgaFile.width;j++) {
			destination[0]=source[2];
			destination[1]=source[1];
			destination[2]=source[0];
			destination[3]=tgaFile.byteCount==4?source[3]:0xff;
			source+=tgaFile.byteCount;
			destination+=4;
		}
	}
	delete [] tempBuf;
	tgaFile.byteCount = 4;

	//close file
	fclose(file);

	return true;
}
