#include "glsl.h"
#include "../../conffile.h"
#include "shader_helpers.h"
#include "shader_platform.h"

static const GLfloat tex_coords[16] = { 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 1.0f, 0.0f, 1.0f,
                                        0.0f, 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f };
static const GLfloat mvp_ortho[16] = { 2.0f,  0.0f,  0.0f,  0.0f,
                                          0.0f,  2.0f,  0.0f,  0.0f,
                                          0.0f,  0.0f, -1.0f,  0.0f,
                                         -1.0f, -1.0f,  0.0f,  1.0f };

static void reduce_to_path(char* filename)
{
    for (int i = strlen(filename); i >= 0; i--)
    {
        if (filename[i] == '\\' || filename[i] == '/')
        {
            filename[i] = 0;
            break;
        }
    }
}

static char* read_file(const char *filename)
{
    FILE* file = NULL;
    int size;
    char* contents;

    file = fopen(filename, "rb");
    if (!file)
        return NULL;

    fseek(file, 0, SEEK_END);
    size = ftell(file);
    fseek(file, 0, SEEK_SET);

    contents = new char[size + 1];
    fread(contents, size, 1, file);
    contents[size] = '\0';
    fclose(file);

    return contents;
}

static int scale_string_to_enum(const char *string, bool last)
{
    if (!strcasecmp(string, "source"))
    {
        return GLSL_SOURCE;
    }
    else if (!strcasecmp(string, "viewport"))
    {
        return GLSL_VIEWPORT;
    }
    else if (!strcasecmp(string, "absolute"))
    {
        return GLSL_ABSOLUTE;
    }
    else if (last)
        return GLSL_VIEWPORT;
    else
        return GLSL_SOURCE;
}

static const char *scale_enum_to_string (int val)
{
    switch (val)
    {
    case GLSL_SOURCE:
        return "source";
    case GLSL_VIEWPORT:
        return "viewport";
    case GLSL_ABSOLUTE:
        return "absolute";
    default:
        return "source";
    }
}

static int wrap_mode_string_to_enum(const char *string)
{
    if (!strcasecmp(string, "repeat"))
    {
        return GL_REPEAT;
    }
    else if (!strcasecmp(string, "clamp_to_edge"))
    {
        return GL_CLAMP_TO_EDGE;
    }
    else if (!strcasecmp(string, "clamp"))
    {
        return GL_CLAMP;
    }
    else
        return GL_CLAMP_TO_BORDER;
}

static const char *wrap_mode_enum_to_string(int val)
{
    switch (val)
    {
    case GL_REPEAT:
        return "repeat";
    case GL_CLAMP_TO_EDGE:
        return "clamp_to_edge";
    case GL_CLAMP:
        return "clamp";
    default:
        return "clamp_to_border";
    }
}


bool GLSLShader::load_shader_file (char *filename)
{
    char key[256];

    if (strlen(filename) < 6 || strcasecmp(&filename[strlen(filename) - 6], ".glslp"))
    {
        GLSLPass pass;
        this->pass.push_back (GLSLPass());

        pass.scale_type_x = pass.scale_type_y = GLSL_VIEWPORT;
        pass.filter = GLSL_UNDEFINED;
        pass.wrap_mode = GL_CLAMP_TO_BORDER;
        strcpy(pass.filename, filename);
        pass.frame_count_mod = 0;
        pass.frame_count = 0;
        pass.fp = 0;
        pass.scale_x = 1.0;
        pass.scale_y = 1.0;
        this->pass.push_back (pass);

        return true;
    }
    else
    {
        conf.LoadFile(filename);
    }

    int shader_count = conf.GetInt("::shaders", 0);

    if (shader_count < 1)
        return false;

    this->pass.push_back (GLSLPass());

    for (int i = 0; i < shader_count; i++)
    {
        GLSLPass pass;

        snprintf(key, 256, "::filter_linear%u", i);
        pass.filter = conf.Exists(key) ? (conf.GetBool(key) ? GL_LINEAR : GL_NEAREST) : GLSL_UNDEFINED;

        sprintf(key, "::scale_type%u", i);
        const char* scaleType = conf.GetString(key, "");

        if (!strcasecmp(scaleType, ""))
        {
            sprintf(key, "::scale_type_x%u", i);
            const char* scaleTypeX = conf.GetString(key, "");
            pass.scale_type_x = scale_string_to_enum(scaleTypeX, i == shader_count - 1);

            sprintf(key, "::scale_type_y%u", i);
            const char* scaleTypeY = conf.GetString(key, "");
            pass.scale_type_y = scale_string_to_enum(scaleTypeY, i == shader_count - 1);
        }
        else
        {
            int scale_type =  scale_string_to_enum(scaleType, i == shader_count - 1);
            pass.scale_type_x = scale_type;
            pass.scale_type_y = scale_type;
        }

        sprintf(key, "::scale%u", i);
        const char* scaleFloat = conf.GetString(key, "");
        if (!strcasecmp(scaleFloat, ""))
        {
            sprintf(key, "::scale_x%u", i);
            const char* scaleFloatX = conf.GetString(key, "1.0");
            pass.scale_x = atof(scaleFloatX);
            sprintf(key, "::scale_y%u", i);
            const char* scaleFloatY = conf.GetString(key, "1.0");
            pass.scale_y = atof(scaleFloatY);
        }
        else
        {
            pass.scale_x = pass.scale_y = atof(scaleFloat);
        }

        sprintf(key, "::shader%u", i);
        strcpy(pass.filename, conf.GetString(key, ""));

        sprintf(key, "::wrap_mode%u", i);
        pass.wrap_mode = wrap_mode_string_to_enum (conf.GetString (key ,""));

        sprintf(key, "::frame_count_mod%u", i);
        pass.frame_count_mod = conf.GetInt(key, 0);

        if (float_texture_available ())
        {
            sprintf(key, "::float_framebuffer%u", i);
            pass.fp = conf.GetBool(key);
        }
        else
            pass.fp = false;

        if (srgb_available ())
        {
            sprintf(key, "::srgb_framebuffer%u", i);
            pass.srgb = conf.GetBool(key);
        }
        else
            pass.srgb = false;

        sprintf(key, "::alias%u", i);
        strcpy(pass.alias, conf.GetString(key, ""));

        this->pass.push_back(pass);
    }

    char* ids = conf.GetStringDup("::textures", "");

    char* id = strtok(ids, ";");

    while (id != NULL)
    {
        GLSLLut lut;

        sprintf(key, "::%s", id);
        strcpy(lut.id, id);
        strcpy(lut.filename, conf.GetString(key, ""));

        sprintf(key, "::%s_wrap_mode", id);
        lut.wrap_mode = wrap_mode_string_to_enum (conf.GetString (key, ""));

        sprintf(key, "::%s_mipmap", id);
        lut.mipmap = conf.GetBool (key);

        sprintf(key, "::%s_linear", id);
        lut.filter = (conf.GetBool(key, false)) ? GL_LINEAR : GL_NEAREST;

        if (lut.mipmap)
        {
            lut.filter = (lut.filter == GL_LINEAR) ? GL_LINEAR_MIPMAP_LINEAR : GL_NEAREST_MIPMAP_NEAREST;
        }

        this->lut.push_back(lut);

        id = strtok(NULL, ";");
    }

    free(ids);

    return true;
}

void GLSLShader::strip_parameter_pragmas(char *buffer)
{
   // #pragma parameter lines tend to have " characters in them,
   // which is not legal GLSL.
   char *s = strstr(buffer, "#pragma parameter");

   while (s)
   {
       GLSLParam par;
       unsigned int i;

       sscanf (s, "#pragma parameter %s \"%[^\"]\" %f %f %f %f",
               par.id, par.name, &par.val, &par.min, &par.max, &par.step);

       if (par.step == 0.0f)
           par.step = 1.0f;

       for (i = 0; i < param.size(); i++)
       {
           if (!strcmp(param[i].id, par.id))
               break;
       }
       if (i >= param.size())
           param.push_back (par);

       // blank out the line
       while (*s != '\0' && *s != '\n')
          *s++ = ' ';
       s = strstr(s, "#pragma parameter");
   }
}

GLuint GLSLShader::compile_shader(char *program,
                                  const char *aliases,
                                  const char *defines,
                                  GLuint type,
                                  GLuint *out)
{
    char info_log[1024];
    char *ptr = program;
    std::string complete_program = "";
    char version[32];
    const char *existing_version = strstr(ptr, "#version");

    if (existing_version)
    {
        unsigned version_no = (unsigned) strtoul (existing_version + 8, &ptr, 10);
        snprintf (version, 32, "#version %u\n", version_no);
    }
    else
    {
        snprintf (version, 32, "#version 150\n");
    }

    complete_program += version;
    complete_program += aliases;
    complete_program += defines;
    complete_program += ptr;

    GLuint shader = glCreateShader (type);
    GLint status;
    GLint length = complete_program.length();
    GLchar *prog = (GLchar *) complete_program.c_str();

    glShaderSource (shader, 1, &prog, &length);
    glCompileShader (shader);
    glGetShaderiv (shader, GL_COMPILE_STATUS, &status);

    info_log[0] = '\0';
    glGetShaderInfoLog (shader, 1024, NULL, info_log);
    if (*info_log)
        printf ("%s\n", info_log);
    *out = shader;

    return status;
}

bool GLSLShader::load_shader (char *filename)
{
    char shader_path[PATH_MAX];
    char temp[PATH_MAX];
    std::string aliases = "";
    GLint status;
    char log[1024];

    if (this->pass.size())
        return false;

    if (!filename || *filename == '\0')
        return false;

    strcpy(shader_path, filename);
    reduce_to_path(shader_path);

    chdir(shader_path);
    if (!load_shader_file(filename))
        return false;

    /*
    for (unsigned int i = 1; i < pass.size(); i++)
    {
        if (pass[i].alias && *pass[i].alias)
        {
            aliases += "#define ";
            aliases += pass[i].alias;
            aliases += " Pass";
            aliases += std::to_string(i - 1);
            aliases += "Texture\n";
            printf ("%s\n", aliases.c_str());
        }
    }*/

    for (unsigned int i = 1; i < pass.size(); i++)
    {
        GLSLPass *p = &pass[i];
        GLuint vertex_shader = 0, fragment_shader = 0;

        realpath(p->filename, temp);
        strcpy (p->filename, temp);

        char *contents = read_file (p->filename);
        if (!contents)
        {
            printf("Couldn't read shader file %s\n", temp);
            return false;
        }

        strip_parameter_pragmas(contents);

        if (!compile_shader (contents,
                             "#define VERTEX\n#define PARAMETER_UNIFORM\n",
                             aliases.c_str(),
                             GL_VERTEX_SHADER,
                             &vertex_shader) || !vertex_shader)
        {
            printf("Couldn't compile vertex shader in %s.\n", p->filename);
            return false;
        }

        if (!compile_shader (contents,
                             "#define FRAGMENT\n#define PARAMETER_UNIFORM\n",
                             aliases.c_str(),
                             GL_FRAGMENT_SHADER,
                             &fragment_shader) || !fragment_shader)
        {
            printf("Couldn't compile fragment shader in %s.\n", p->filename);
            return false;
        }

        delete[] contents;

        p->program = glCreateProgram ();

        glAttachShader (p->program, vertex_shader);
        glAttachShader (p->program, fragment_shader);

        glLinkProgram (p->program);
        glGetProgramiv (p->program, GL_LINK_STATUS, &status);
        log[0] = '\0';
        glGetProgramInfoLog(p->program, 1024, NULL, log);
        if (*log)
            printf ("%s\n", log);

        glDeleteShader (vertex_shader);
        glDeleteShader (fragment_shader);

        if (status != GL_TRUE)
        {
            printf ("Failed to link program\n");
            glDeleteProgram (p->program);

            return false;
        }

        glGenFramebuffers(1, &p->fbo);
        glGenTextures(1, &p->texture);
        glBindTexture(GL_TEXTURE_2D, p->texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

        p->frame_count = 0;
    }

    for (unsigned int i = 0; i < lut.size(); i++)
    {
        GLSLLut *l = &lut[i];
        // generate texture for the lut and apply specified filter setting
        glGenTextures(1, &l->texture);
        glBindTexture(GL_TEXTURE_2D, l->texture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, l->wrap_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, l->wrap_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, l->filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, l->filter);

        realpath(l->filename, temp);
        strcpy (l->filename, temp);

        // simple file extension png/tga decision
        int length = strlen(temp);
        if (length > 4)
        {
            if (!strcasecmp(&temp[length - 4], ".png"))
            {
                int width, height;
                bool hasAlpha;
                GLubyte* texData;

                if (loadPngImage(temp, width, height, hasAlpha, &texData))
                {
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
                    glTexImage2D(GL_TEXTURE_2D,
                                 0,
                                 GL_RGBA,
                                 width,
                                 height,
                                 0,
                                 hasAlpha ? GL_RGBA : GL_RGB,
                                 GL_UNSIGNED_BYTE,
                                 texData);
                    free(texData);
                }
                else
                {
                    printf ("Failed to load PNG LUT: %s\n", temp);
                }
            }
            else if (!strcasecmp(&temp[length - 4], ".tga"))
            {
                STGA stga;

                if (loadTGA(temp, stga))
                {
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, stga.width);
                    glTexImage2D(GL_TEXTURE_2D,
                                 0,
                                 GL_RGBA,
                                 stga.width,
                                 stga.height,
                                 0,
                                 GL_RGBA,
                                 GL_UNSIGNED_BYTE,
                                 stga.data);
                }
                else
                {
                    printf ("Failed to load TGA LUT: %s\n", temp);
                }
            }
        }

        if (l->mipmap)
            glGenerateMipmap (GL_TEXTURE_2D);
    }

    // Check for parameters specified in file
    for (unsigned int i = 0; i < param.size(); i++)
    {
        char key[266];
        const char *value;
        snprintf (key, 266, "::%s", param[i].id);
        value = conf.GetString (key, NULL);
        if (value)
        {
            param[i].val = atof (value);
            if (param[i].val < param[i].min)
                param[i].val = param[i].min;
            if (param[i].val > param[i].max)
                param[i].val = param[i].max;
        }
    }

    glActiveTexture(GL_TEXTURE1);
    glEnable(GL_TEXTURE_COORD_ARRAY);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords);
    glActiveTexture(GL_TEXTURE0);

    register_uniforms();

    prev_frame.resize (max_prev_frame);

    for (unsigned int i = 0; i < prev_frame.size(); i++)
    {
        glGenTextures(1, &(prev_frame[i].texture));
        prev_frame[i].width = prev_frame[i].height = 0;
    }

    glGenBuffers(1, &vbo);

    frame_count = 0;

    return true;
}

void GLSLShader::render(GLuint &orig,
                        int width, int height,
                        int viewport_x, int viewport_y,
                        int viewport_width, int viewport_height,
                        GLSLViewportCallback vpcallback)
{
    frame_count++;

    // set up our dummy pass for easier loop code
    pass[0].texture = orig;
    pass[0].width = width;
    pass[0].height = height;

    // loop through all real passes
    for (unsigned int i = 1; i < pass.size(); i++)
    {
        bool lastpass = (i == pass.size() - 1);

        switch (pass[i].scale_type_x)
        {
        case GLSL_ABSOLUTE:
            pass[i].width = pass[i].scale_x;
            break;
        case GLSL_SOURCE:
            pass[i].width = pass[i-1].width * pass[i].scale_x;
            break;
        case GLSL_VIEWPORT:
            pass[i].width = viewport_width * pass[i].scale_x;
            break;
        default:
            pass[i].width = viewport_width;
        }

        switch (pass[i].scale_type_y)
        {
        case GLSL_ABSOLUTE:
            pass[i].height = pass[i].scale_y;
            break;
        case GLSL_SOURCE:
            pass[i].height = pass[i - 1].height * pass[i].scale_y;
            break;
        case GLSL_VIEWPORT:
            pass[i].height = viewport_height * pass[i].scale_y;
            break;
        default:
            pass[i].height = viewport_height;
        }

        if (!lastpass)
        {
            // Output to a framebuffer texture
            glBindTexture(GL_TEXTURE_2D, pass[i].texture);

            if (pass[i].srgb)
            {
                glEnable(GL_FRAMEBUFFER_SRGB);

                glTexImage2D(GL_TEXTURE_2D,
                             0,
                             GL_SRGB8_ALPHA8,
                             (unsigned int) pass[i].width,
                             (unsigned int) pass[i].height,
                             0,
                             GL_RGBA,
                             GL_UNSIGNED_INT_8_8_8_8,
                             NULL);
            }
            else
            {
                glTexImage2D(GL_TEXTURE_2D,
                             0,
                             (pass[i].fp ? GL_RGBA32F : GL_RGBA),
                             (unsigned int) pass[i].width,
                             (unsigned int) pass[i].height,
                             0,
                             GL_RGBA,
                             (pass[i].fp ? GL_FLOAT : GL_UNSIGNED_INT_8_8_8_8),
                             NULL);
            }

            // viewport determines the area we render into the output texture
            glViewport(0, 0, pass[i].width, pass[i].height);

            // set up framebuffer and attach output texture
            glBindFramebuffer(GL_FRAMEBUFFER, pass[i].fbo);
            glFramebufferTexture2D(GL_FRAMEBUFFER,
                                   GL_COLOR_ATTACHMENT0,
                                   GL_TEXTURE_2D,
                                   pass[i].texture,
                                   0);
        }
        else
        {
            int out_x = 0;
            int out_y = 0;
            int out_width = 0;
            int out_height = 0;

            // output to the screen
            glBindFramebuffer(GL_FRAMEBUFFER, 0);
            vpcallback (pass[i].width, pass[i].height,
                        viewport_x, viewport_y,
                        viewport_width, viewport_height,
                        &out_x, &out_y,
                        &out_width, &out_height);
            glViewport(out_x, out_y, out_width, out_height);
        }

        // set up input texture (output of previous pass) and apply filter settings
        GLuint filter = (pass[i].filter == GLSL_UNDEFINED) ?
                         (lastpass ?
                          (Settings.BilinearFilter ? GL_LINEAR : GL_NEAREST) : GL_NEAREST
                              ) : pass[i].filter;

        glBindTexture(GL_TEXTURE_2D, pass[i - 1].texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) pass[i - 1].width);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MAG_FILTER,
                        filter);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_MIN_FILTER,
                        filter);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_S,
                        pass[i].wrap_mode);
        glTexParameteri(GL_TEXTURE_2D,
                        GL_TEXTURE_WRAP_T,
                        pass[i].wrap_mode);

        glTexCoordPointer(2, GL_FLOAT, 0, tex_coords + (lastpass ? 8 : 0));

        glUseProgram (pass[i].program);
        set_shader_vars(i);

        glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_QUADS, 0, 4);

        // reset vertex attribs set in set_shader_vars
        clear_shader_vars();

        if (pass[i].srgb)
        {
            glDisable (GL_FRAMEBUFFER_SRGB);
        }
    }

    // Disable framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram (0);

    // Pop back of previous frame stack and use as upload buffer
    if (prev_frame.size() > 0)
    {
        GLint internal_format;
        orig = prev_frame.back().texture;
        prev_frame.pop_back();

        GLSLPass newprevframe;
        newprevframe.width = width;
        newprevframe.height = height;
        newprevframe.texture = pass[0].texture;
        prev_frame.push_front(newprevframe);
        glBindTexture(GL_TEXTURE_2D, newprevframe.texture);
        glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_INTERNAL_FORMAT, &internal_format);

        glBindTexture(GL_TEXTURE_2D, orig);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
        glTexImage2D(GL_TEXTURE_2D,
                     0,
                     internal_format,
                     width,
                     height,
                     0,
                     GL_RGB,
                     GL_UNSIGNED_BYTE,
                     NULL);
    }

    glBindTexture(GL_TEXTURE_2D, orig);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint)pass.back().width);
    glTexCoordPointer(2, GL_FLOAT, 0, tex_coords + 8);
}

void GLSLShader::register_uniforms ()
{
    max_prev_frame = 0;
    char varname[100];

    for (unsigned int i = 1; i < pass.size(); i++)
    {
        GLSLUniforms *u = &pass[i].unif;
        GLuint program = pass[i].program;

        glUseProgram (program);

        GLint mvp = glGetUniformLocation (program, "MVPMatrix");
        if (mvp > -1)
            glUniformMatrix4fv(mvp, 1, GL_FALSE, mvp_ortho);

        u->Texture = glGetUniformLocation (program, "Texture");
        u->InputSize = glGetUniformLocation (program, "InputSize");
        u->OutputSize = glGetUniformLocation (program, "OutputSize");
        u->TextureSize = glGetUniformLocation (program, "TextureSize");

        u->TexCoord = glGetAttribLocation (program, "TexCoord");
        u->LUTTexCoord = glGetAttribLocation (program, "LUTTexCoord");
        u->VertexCoord = glGetAttribLocation (program, "VertexCoord");

        u->FrameCount = glGetUniformLocation (program, "FrameCount");
        u->FrameDirection = glGetUniformLocation (program, "FrameDirection");

        u->OrigTexture = glGetUniformLocation (program, "OrigTexture");
        u->OrigInputSize = glGetUniformLocation (program, "OrigInputSize");
        u->OrigTextureSize = glGetUniformLocation (program, "OrigTextureSize");
        u->OrigTexCoord = glGetAttribLocation (program, "OrigTexCoord");

        u->Prev[0].Texture = glGetUniformLocation (program, "PrevTexture");
        u->Prev[0].InputSize = glGetUniformLocation (program, "PrevInputSize");
        u->Prev[0].TextureSize = glGetUniformLocation (program, "PrevTextureSize");
        u->Prev[0].TexCoord = glGetAttribLocation (program, "PrevTexCoord");

        if (u->Prev[0].Texture > -1)
            max_prev_frame = 1;

        for (unsigned int j = 1; j < 7; j++)
        {
            sprintf(varname, "Prev%dTexture", j);
            u->Prev[j].Texture = glGetUniformLocation (program, varname);
            sprintf(varname, "Prev%dInputSize", j);
            u->Prev[j].InputSize = glGetUniformLocation (program, varname);
            sprintf(varname, "Prev%dTextureSize", j);
            u->Prev[j].TextureSize = glGetUniformLocation (program, varname);
            sprintf(varname, "Prev%dTexCoord", j);
            u->Prev[j].TexCoord = glGetAttribLocation (program, varname);

            if (u->Prev[j].Texture > -1)
                max_prev_frame = j + 1;
        }
        for (unsigned int j = 0; j < pass.size(); j++)
        {
            sprintf(varname, "Pass%dTexture", j);
            u->Pass[j].Texture = glGetUniformLocation (program, varname);
            sprintf(varname, "Pass%dInputSize", j);
            u->Pass[j].InputSize = glGetUniformLocation (program, varname);
            sprintf(varname, "Pass%dTextureSize", j);
            u->Pass[j].TextureSize = glGetUniformLocation (program, varname);
            sprintf(varname, "Pass%dTexCoord", j);
            u->Pass[j].TexCoord = glGetAttribLocation (program, varname);
            if (u->Pass[j].Texture)
                u->max_pass = j;

            sprintf(varname, "PassPrev%dTexture", j);
            u->PassPrev[j].Texture = glGetUniformLocation (program, varname);
            sprintf(varname, "PassPrev%dInputSize", j);
            u->PassPrev[j].InputSize = glGetUniformLocation (program, varname);
            sprintf(varname, "PassPrev%dTextureSize", j);
            u->PassPrev[j].TextureSize = glGetUniformLocation (program, varname);
            sprintf(varname, "PassPrev%dTexCoord", j);
            u->PassPrev[j].TexCoord = glGetAttribLocation (program, varname);
            if (u->PassPrev[j].Texture > -1)
                u->max_prevpass = j;
        }

        for (unsigned int j = 0; j < lut.size(); j++)
        {
            u->Lut[j] = glGetUniformLocation (program, lut[j].id);
        }

        for (unsigned int j = 0; j < param.size(); j++)
        {
            param[j].unif[i] = glGetUniformLocation (program, param[j].id);
        }
    }

    glUseProgram (0);
}

void GLSLShader::set_shader_vars (unsigned int p)
{
    unsigned int texunit = 0;
    unsigned int offset = 0;

    if (p == pass.size() - 1)
        offset = 8;
    GLSLUniforms *u = &pass[p].unif;

    GLint mvp = glGetUniformLocation (pass[p].program, "MVPMatrix");
    if (mvp > -1)
        glUniformMatrix4fv(mvp, 1, GL_FALSE, mvp_ortho);

#define setUniform2fv(uni, val) if (uni > -1) glUniform2fv (uni, 1, val);
#define setUniform1f(uni, val) if (uni > -1) glUniform1f (uni, val);
#define setUniform1i(uni, val) if (uni > -1) glUniform1i (uni, val);
#define setTexture1i(uni, val) \
    if (uni > -1) \
    { \
        glActiveTexture (GL_TEXTURE0 + texunit); \
        glBindTexture (GL_TEXTURE_2D, val); \
        glUniform1i (uni, texunit); \
        texunit++; \
    }
    // We use non-power-of-two textures,
    // so no need to mess with input size/texture size
#define setTexCoords(attr) \
    if (attr > -1) \
    { \
        glEnableVertexAttribArray(attr); \
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof (float) * offset)); \
        vaos.push_back (attr); \
    }
#define setTexCoordsNoOffset(attr) \
    if (attr > -1) \
    { \
        glEnableVertexAttribArray(attr); \
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, 0, NULL); \
        vaos.push_back (attr); \
    }

    glBindBuffer (GL_ARRAY_BUFFER, vbo);
    glBufferData (GL_ARRAY_BUFFER, sizeof (GLfloat) * 16, tex_coords, GL_STATIC_DRAW);

    float inputSize[2] = { (float) pass[p - 1].width, (float) pass[p - 1].height };
    float outputSize[2] = { (float) pass[p].width, (float) pass[p].height };

    setTexture1i (u->Texture, pass[p - 1].texture);
    setUniform2fv(u->InputSize, inputSize);
    setUniform2fv(u->OutputSize, outputSize);
    setUniform2fv(u->TextureSize, inputSize);

    unsigned int shaderFrameCnt = frame_count;
    if (pass[p].frame_count_mod)
        shaderFrameCnt %= pass[p].frame_count_mod;
    setUniform1i(u->FrameCount, (float) shaderFrameCnt);
    setUniform1i(u->FrameDirection, Settings.Rewinding ? -1.0f : 1.0f);

    setTexCoords (u->TexCoord);
    setTexCoords (u->LUTTexCoord);
    setTexCoordsNoOffset (u->VertexCoord);

    // Orig parameter
    float orig_videoSize[2] = { (float) pass[0].width, (float) pass[0].height };

    setUniform2fv(u->OrigInputSize,  orig_videoSize);
    setUniform2fv(u->OrigTextureSize, orig_videoSize);
    setTexture1i (u->OrigTexture, pass[0].texture);
    setTexCoords (u->OrigTexCoord);

    // Prev parameter
    if (max_prev_frame >= 1 && prev_frame[0].width > 0) {
        float prevSize[2] = { (float) prev_frame[0].width, (float) prev_frame[0].height };

        setUniform2fv(u->Prev[0].InputSize, prevSize);
        setUniform2fv(u->Prev[0].TextureSize, prevSize);
        setTexture1i (u->Prev[0].Texture, prev_frame[0].texture);
        setTexCoords (u->Prev[0].TexCoord);
    }

    // Prev[1-6] parameters
    for (unsigned int i = 1; i < prev_frame.size(); i++)
    {
        if (prev_frame[i].width <= 0)
            break;

        float prevSize[2] = { (float) prev_frame[i].width, (float) prev_frame[i].height };

        setUniform2fv(u->Prev[i].InputSize, prevSize);
        setUniform2fv(u->Prev[i].TextureSize, prevSize);
        setTexture1i (u->Prev[i].Texture, prev_frame[i].texture);
        setTexCoords (u->Prev[i].TexCoord);
    }

    // LUT parameters
    for (unsigned int i = 0; i < lut.size(); i++)
    {
        setTexture1i (u->Lut[i], lut[i].texture);
    }

    // PassX parameters, only for third pass and up
    if (p > 2) {
        for (unsigned int i = 1; i < p - 1; i++) {
            float passSize[2] = { (float) pass[i].width, (float) pass[i].height };
            setUniform2fv(u->Pass[i].InputSize, passSize);
            setUniform2fv(u->Pass[i].TextureSize, passSize);
            setTexture1i (u->Pass[i].Texture, pass[i].texture);
            setTexCoords (u->Pass[i].TexCoord);
        }
    }

    // PassPrev parameter
    for (unsigned int i = 0; i < p; i++)
    {
        float passSize[2] = { (float) pass[i].width, (float) pass[i].height };
        setUniform2fv(u->PassPrev[p - i].InputSize, passSize);
        setUniform2fv(u->PassPrev[p - i].TextureSize, passSize);
        setTexture1i (u->PassPrev[p - i].Texture, pass[i].texture);
        setTexCoords (u->PassPrev[p - i].TexCoord);
    }

    // User and Preset Parameters
    for (unsigned int i = 0; i < param.size(); i++)
    {
        setUniform1f (param[i].unif[p], param[i].val);
    }

    glActiveTexture (GL_TEXTURE0);
    glBindBuffer (GL_ARRAY_BUFFER, 0);
}

void GLSLShader::clear_shader_vars (void)
{
    for (unsigned int i = 0; i < vaos.size(); i++)
        glDisableVertexAttribArray (vaos[i]);

    vaos.clear();
}

#define outs(s, v) fprintf (file, "%s%d = \"%s\"\n", s, i, v)
#define outf(s, v) fprintf (file, "%s%d = \"%f\"\n", s, i, v)
#define outd(s, v) fprintf (file, "%s%d = \"%d\"\n", s, i, v)
void GLSLShader::save (const char *filename)
{
    FILE *file = fopen (filename, "wb");
    if (!file)
    {
        printf ("Couldn't save shader config to %s\n", filename);
    }

    fprintf (file, "shaders = \"%d\"\n", (unsigned int) pass.size() - 1);

    for (unsigned int pn = 1; pn < pass.size(); pn++)
    {
        GLSLPass *p = &pass[pn];
        unsigned int i = pn - 1;

        outs("shader", p->filename);
        if (p->filter != GLSL_UNDEFINED)
        {
            outs("filter_linear", p->filter == GL_LINEAR ? "true" : "false");
        }
        outs ("wrap_mode", wrap_mode_enum_to_string(p->wrap_mode));
        outs ("alias", p->alias);
        outs ("float_framebuffer", p->fp ? "true" : "false");
        outs ("srgb_framebuffer", p->srgb ? "true" : "false");
        outs ("scale_type_x", scale_enum_to_string(p->scale_type_x));
        if (p->scale_type_x == GLSL_ABSOLUTE)
            outd ("scale_x", (int) p->scale_x);
        else
            outf ("scale_x", p->scale_x);

        outs ("scale_type_y", scale_enum_to_string(p->scale_type_y));
        if (p->scale_type_y == GLSL_ABSOLUTE)
            outd ("scale_y", (int) p->scale_y);
        else
            outf ("scale_y", p->scale_y);

        if (p->frame_count_mod)
            outd ("frame_count_mod", p->frame_count_mod);
    }

    if (param.size() > 0)
    {
        fprintf (file, "parameters = \"");
        for (unsigned int i = 0; i < param.size(); i++)
        {
            fprintf (file, "%s%c", param[i].id, (i == param.size() - 1) ? '\"' : ';');
        }
        fprintf (file, "\n");
    }

    for (unsigned int i = 0; i < param.size(); i++)
    {
        fprintf (file, "%s = \"%f\"\n", param[i].id, param[i].val);
    }

    if (lut.size() > 0)
    {
        fprintf (file, "textures = \"");
        for (unsigned int i = 0; i < lut.size(); i++)
        {
            fprintf (file, "%s%c", lut[i].id, (i == lut.size() - 1) ? '\"' : ';');
        }
        fprintf (file, "\n");
    }

    for (unsigned int i = 0; i < lut.size(); i++)
    {
        fprintf (file, "%s = \"%s\"\n", lut[i].id, lut[i].filename);
        fprintf (file, "%s_linear = \"%s\"\n", lut[i].id, lut[i].filter == GL_LINEAR || lut[i].filter == GL_LINEAR_MIPMAP_LINEAR ? "true" : "false");
        fprintf (file, "%s_wrap_mode = \"%s\"\n", lut[i].id, wrap_mode_enum_to_string(lut[i].wrap_mode));
        fprintf (file, "%s_mipmap = \"%s\"\n", lut[i].id, lut[i].mipmap ? "true" : "false");
    }

    fclose (file);
}
#undef outf
#undef outs
#undef outd


void GLSLShader::destroy (void)
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);

    for (unsigned int i = 1; i < pass.size(); i++)
    {
        glDeleteProgram(pass[i].program);
        glDeleteTextures(1, &pass[i].texture);
        glDeleteFramebuffers(1, &pass[i].fbo);
    }

    for (unsigned int i = 0; i < lut.size(); i++)
    {
        glDeleteTextures (1, &lut[i].texture);
    }

    for (unsigned int i = 0; i < prev_frame.size (); i++)
    {
        glDeleteTextures (1, &prev_frame[i].texture);
    }

    param.clear();
    pass.clear();
    lut.clear();
    prev_frame.clear();
    conf.Clear();
}

