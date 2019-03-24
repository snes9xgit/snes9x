/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include "glsl.h"
#include "../../conffile.h"
#include "shader_helpers.h"
#include "shader_platform.h"

static const GLfloat tex_coords[16] = { 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f, 1.0f,
                                        0.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f, 0.0f };
static const GLfloat mvp_ortho[16] = { 2.0f,  0.0f,  0.0f,  0.0f,
                                       0.0f,  2.0f,  0.0f,  0.0f,
                                       0.0f,  0.0f, -1.0f,  0.0f,
                                      -1.0f, -1.0f,  0.0f,  1.0f };

static int scale_string_to_enum(const char *string)
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
    else
        return GLSL_NONE;
}

static const char *scale_enum_to_string(int val)
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
        return "undefined";
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


bool GLSLShader::load_shader_preset_file(char *filename)
{
    char key[256];
    int length = strlen(filename);
    bool singlepass = false;

    if (length > 6 && (!strcasecmp(&filename[length - 5], ".glsl") ||
                       !strcasecmp(&filename[length - 6], ".slang")))
        singlepass = true;

    this->using_slang = false;
    if (length > 7 && (!strcasecmp(&filename[length - 6], ".slang") ||
                       !strcasecmp(&filename[length - 7], ".slangp")))
    {
#ifdef USE_SLANG
        this->using_slang = true;
#else
        return false;
#endif
    }

    if (singlepass)
    {
        GLSLPass pass;
        this->pass.push_back(GLSLPass());

        pass.scale_type_x = pass.scale_type_y = GLSL_VIEWPORT;
        pass.filter = GLSL_UNDEFINED;
        pass.wrap_mode = GL_CLAMP_TO_BORDER;
        strcpy(pass.filename, filename);
        pass.frame_count_mod = 0;
        pass.frame_count = 0;
        pass.fp = 0;
        pass.scale_x = 1.0;
        pass.scale_y = 1.0;
        this->pass.push_back(pass);

        return true;
    }
    else
    {
        conf.LoadFile(filename);
    }

    int shader_count = conf.GetInt("::shaders", 0);

    if (shader_count < 1)
        return false;

    this->pass.push_back(GLSLPass());

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
            pass.scale_type_x = scale_string_to_enum(scaleTypeX);

            sprintf(key, "::scale_type_y%u", i);
            const char* scaleTypeY = conf.GetString(key, "");
            pass.scale_type_y = scale_string_to_enum(scaleTypeY);
        }
        else
        {
            int scale_type =  scale_string_to_enum(scaleType);
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

        sprintf(key, "::mipmap_input%u", i);
        pass.mipmap_input = conf.GetBool (key);

        sprintf(key, "::frame_count_mod%u", i);
        pass.frame_count_mod = conf.GetInt(key, 0);

        if (gl_float_texture_available())
        {
            sprintf(key, "::float_framebuffer%u", i);
            pass.fp = conf.GetBool(key);
        }
        else
            pass.fp = false;

        if (gl_srgb_available())
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

static std::string folder_from_path(std::string filename)
{
    for (int i = filename.length() - 1; i >= 0; i--)
        if (filename[i] == '\\' || filename[i] == '/')
            return filename.substr(0, i);

    return std::string(".");
}

static std::string canonicalize(const std::string &noncanonical)
{
    char *temp = realpath(noncanonical.c_str(), NULL);
    std::string filename_string(temp);
    free(temp);
    return filename_string;
}

static GLuint string_to_format(char *format)
{
#define MATCH(s, f)                                                            \
    if (!strcmp(format, s))                                                    \
        return f;
    MATCH("R8_UNORM", GL_R8);
    MATCH("R8_UINT", GL_R8UI);
    MATCH("R8_SINT", GL_R8I);
    MATCH("R8G8_UNORM", GL_RG8);
    MATCH("R8G8_UINT", GL_RG8UI);
    MATCH("R8G8_SINT", GL_RG8I);
    MATCH("R8G8B8A8_UNORM", GL_RGBA8);
    MATCH("R8G8B8A8_UINT", GL_RGBA8UI);
    MATCH("R8G8B8A8_SINT", GL_RGBA8I);
    MATCH("R8G8B8A8_SRGB", GL_SRGB8_ALPHA8);

    MATCH("R16_UINT", GL_R16UI);
    MATCH("R16_SINT", GL_R16I);
    MATCH("R16_SFLOAT", GL_R16F);
    MATCH("R16G16_UINT", GL_RG16UI);
    MATCH("R16G16_SINT", GL_RG16I);
    MATCH("R16G16_SFLOAT", GL_RG16F);
    MATCH("R16G16B16A16_UINT", GL_RGBA16UI);
    MATCH("R16G16B16A16_SINT", GL_RGBA16I);
    MATCH("R16G16B16A16_SFLOAT", GL_RGBA16F);

    MATCH("R32_UINT", GL_R32UI);
    MATCH("R32_SINT", GL_R32I);
    MATCH("R32_SFLOAT", GL_R32F);
    MATCH("R32G32_UINT", GL_RG32UI);
    MATCH("R32G32_SINT", GL_RG32I);
    MATCH("R32G32_SFLOAT", GL_RG32F);
    MATCH("R32G32B32A32_UINT", GL_RGBA32UI);
    MATCH("R32G32B32A32_SINT", GL_RGBA32I);
    MATCH("R32G32B32A32_SFLOAT", GL_RGBA32F);

    return GL_RGBA;
}

// filename must be canonical
void GLSLShader::read_shader_file_with_includes(std::string filename,
                                                std::vector<std::string> &lines,
                                                int p)
{
    std::ifstream ss(filename.c_str());

    if (ss.fail())
    {
        printf ("Couldn't open file \"%s\"\n", filename.c_str());
        return;
    }

    std::string line;

    while (std::getline(ss, line, '\n'))
    {
        if (line.empty())
            continue;

        if (line.compare(0, 8, "#include") == 0)
        {
            char tmp[PATH_MAX];
            sscanf(line.c_str(), "#include \"%[^\"]\"", tmp);

            std::string fullpath = canonicalize(folder_from_path(filename) + "/" + tmp);
            read_shader_file_with_includes(fullpath.c_str(), lines, p);
            continue;
        }
        else if (line.compare(0, 17, "#pragma parameter") == 0)
        {
            GLSLParam par;

            sscanf(line.c_str(), "#pragma parameter %s \"%[^\"]\" %f %f %f %f",
                   par.id, par.name, &par.val, &par.min, &par.max, &par.step);

            if (par.step == 0.0f)
                par.step = 1.0f;

            unsigned int i = 0;
            for (; i < param.size(); i++)
            {
                if (!strcmp(param[i].id, par.id))
                    break;
            }
            if (i >= param.size())
                param.push_back(par);
            continue;
        }
#ifdef USE_SLANG
        else if (line.compare(0, 12, "#pragma name") == 0)
        {
            sscanf(line.c_str(), "#pragma name %255s", pass[p].alias);
            continue;
        }
        else if (line.compare(0, 14, "#pragma format") == 0)
        {
            char format[256];
            sscanf(line.c_str(), "#pragma format %255s", format);
            pass[p].format = string_to_format(format);
            if (pass[p].format == GL_RGBA16F || pass[p].format == GL_RGBA32F)
                pass[p].fp = TRUE;
            else if (pass[p].format == GL_SRGB8_ALPHA8)
                pass[p].srgb = TRUE;
            continue;
        }
#endif
        lines.push_back(line);
    }

    return;
}

GLuint GLSLShader::compile_shader(std::vector<std::string> &lines,
                                  const char *aliases,
                                  const char *defines,
                                  GLuint type,
                                  GLuint *out)
{
    char info_log[1024];
    std::string source;
    unsigned int first_line = 0;

    if (lines.empty())
        return 0;

    if (lines[0].find("#version") == std::string::npos)
    {
        int version = gl_version();
        if (version >= 33)
            version *= 10;
        else
            version = 150;
        source += "#version " + std::to_string(version) + "\n";
    }
    else
    {
        source += lines[0] + "\n";
        first_line++;
    }

    source += aliases;
    source += defines;
    for (unsigned int i = first_line; i < lines.size(); i++)
        source += lines[i] + "\n";

    GLuint shader = glCreateShader(type);
    GLint status;
    GLint length = source.length();
    GLchar *prog = (GLchar *)source.c_str();

    glShaderSource(shader, 1, &prog, &length);
    glCompileShader(shader);
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);

    info_log[0] = '\0';
    glGetShaderInfoLog(shader, 1024, NULL, info_log);
    if (*info_log)
        printf("%s\n", info_log);
    *out = shader;

    return status;
}

bool GLSLShader::load_shader(char *filename)
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
    if (!load_shader_preset_file(filename))
        return false;

    for (unsigned int i = 1; i < pass.size(); i++)
    {
        GLSLPass *p = &pass[i];
        GLuint vertex_shader = 0, fragment_shader = 0;

        realpath(p->filename, temp);
        strcpy(p->filename, temp);

        std::vector<std::string> lines;
        read_shader_file_with_includes(p->filename, lines, i);

        if (lines.empty())
        {
            printf("Couldn't read shader file \"%s\"\n", temp);
            return false;
        }

#ifdef USE_SLANG
        if (using_slang)
        {
            GLint retval;
            retval = slang_compile(lines, "vertex");
            if (retval < 0)
            {
                printf("Vertex shader in \"%s\" failed to compile.\n", p->filename);
                return false;
            }
            vertex_shader = retval;

            retval = slang_compile(lines, "fragment");
            if (retval < 0)
            {
                printf ("Fragment shader in \"%s\" failed to compile.\n", p->filename);
                return false;
            }
            fragment_shader = retval;
        }
        else
#endif
        {
            if (!compile_shader(lines,
                                "#define VERTEX\n#define PARAMETER_UNIFORM\n",
                                aliases.c_str(),
                                GL_VERTEX_SHADER,
                                &vertex_shader) || !vertex_shader)
            {
                printf("Couldn't compile vertex shader in \"%s\".\n", p->filename);
                return false;
            }

            if (!compile_shader(lines,
                                "#define FRAGMENT\n#define PARAMETER_UNIFORM\n",
                                aliases.c_str(),
                                GL_FRAGMENT_SHADER,
                                &fragment_shader) || !fragment_shader)
            {
                printf("Couldn't compile fragment shader in \"%s\".\n", p->filename);
                return false;
            }
        }

        p->program = glCreateProgram();

        glAttachShader(p->program, vertex_shader);
        glAttachShader(p->program, fragment_shader);

        glLinkProgram(p->program);
        glGetProgramiv(p->program, GL_LINK_STATUS, &status);
        log[0] = '\0';
        glGetProgramInfoLog(p->program, 1024, NULL, log);
        if (*log)
            printf("%s\n", log);

        glDeleteShader(vertex_shader);
        glDeleteShader(fragment_shader);

        if (status != GL_TRUE)
        {
            printf("Failed to link program\n");
            glDeleteProgram(p->program);

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
        strcpy(l->filename, temp);

        // simple file extension png/tga decision
        int length = strlen(temp);
        if (length > 4)
        {
            if (!strcasecmp(&temp[length - 4], ".png"))
            {
                int width, height;
                bool hasAlpha;
                bool grayscale;
                GLubyte* texData;

                if (loadPngImage(temp, width, height, grayscale, hasAlpha,
                                 &texData))
                {
                    glPixelStorei(GL_UNPACK_ROW_LENGTH, width);
                    if (grayscale)
                        glTexImage2D(GL_TEXTURE_2D,
                                     0,
                                     GL_RGBA,
                                     width,
                                     height,
                                     0,
                                     GL_LUMINANCE,
                                     GL_UNSIGNED_BYTE,
                                     texData);
                    else
                        glTexImage2D(GL_TEXTURE_2D,
                                     0,
                                     GL_RGBA,
                                     width,
                                     height,
                                     0,
                                     hasAlpha ? GL_RGBA : GL_RGB,
                                     GL_UNSIGNED_BYTE,
                                     texData);
                    l->width = width;
                    l->height = height;
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
                    l->width = stga.width;
                    l->height = stga.height;
                }
                else
                {
                    printf("Failed to load TGA LUT: %s\n", temp);
                }
            }
        }

        if (l->mipmap)
            glGenerateMipmap(GL_TEXTURE_2D);
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
            param[i].val = atof(value);
            if (param[i].val < param[i].min)
                param[i].val = param[i].min;
            if (param[i].val > param[i].max)
                param[i].val = param[i].max;
        }
    }

    glActiveTexture(GL_TEXTURE0);

#ifdef USE_SLANG
    if (using_slang)
        slang_introspect();
    else
#endif
        register_uniforms();

    prev_frame.resize(max_prev_frame);

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
    GLint saved_framebuffer;

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &saved_framebuffer);

    frame_count++;

    // set up our dummy pass for easier loop code
    pass[0].texture = orig;
    pass[0].width = width;
    pass[0].height = height;

#ifdef USE_SLANG
    if (using_slang && using_feedback)
        for (int i = 1; i < (int)pass.size(); i++)
        {
            if (!pass[i].uses_feedback)
                continue;
            GLuint tmp = pass[i].texture;
            pass[i].texture = pass[i].feedback_texture;
            pass[i].feedback_texture = tmp;
        }
#endif

    // loop through all real passes
    for (unsigned int i = 1; i < pass.size(); i++)
    {
        bool lastpass = (i == pass.size() - 1);

        switch (pass[i].scale_type_x)
        {
        case GLSL_ABSOLUTE:
            pass[i].width = (GLuint)pass[i].scale_x;
            break;
        case GLSL_SOURCE:
            pass[i].width = (GLuint)(pass[i-1].width * pass[i].scale_x);
            break;
        case GLSL_VIEWPORT:
            pass[i].width = (GLuint)(viewport_width * pass[i].scale_x);
            break;
        default:
            if (lastpass)
                pass[i].width = viewport_width;
            else
                pass[i].width = (GLuint)(pass[i - 1].width * pass[i].scale_x);
        }

        switch (pass[i].scale_type_y)
        {
        case GLSL_ABSOLUTE:
            pass[i].height = (GLuint)pass[i].scale_y;
            break;
        case GLSL_SOURCE:
            pass[i].height = (GLuint)(pass[i - 1].height * pass[i].scale_y);
            break;
        case GLSL_VIEWPORT:
            pass[i].height = (GLuint)(viewport_height * pass[i].scale_y);
            break;
        default:
            if (lastpass)
                pass[i].height = viewport_height;
            else
                pass[i].height = (GLuint)(pass[i - 1].height * pass[i].scale_y);
        }

        bool direct_lastpass = true;
#ifdef USE_SLANG
        if (lastpass && using_feedback && pass[i].scale_type_x != GLSL_NONE &&
            pass[i].scale_type_y != GLSL_NONE && pass[i].uses_feedback)
        {
            direct_lastpass = false;
        }
#endif
        if (!lastpass || !direct_lastpass)
        {
            // Output to a framebuffer texture
            glBindTexture(GL_TEXTURE_2D, pass[i].texture);

            if (pass[i].srgb)
            {
                glEnable(GL_FRAMEBUFFER_SRGB);

                glTexImage2D(GL_TEXTURE_2D,
                             0,
                             GL_SRGB8_ALPHA8,
                             (unsigned int)pass[i].width,
                             (unsigned int)pass[i].height,
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
                             (unsigned int)pass[i].width,
                             (unsigned int)pass[i].height,
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
            glBindFramebuffer(GL_FRAMEBUFFER, saved_framebuffer);
            vpcallback (pass[i].width, pass[i].height,
                        viewport_x, viewport_y,
                        viewport_width, viewport_height,
                        &out_x, &out_y,
                        &out_width, &out_height);
            glViewport(out_x, out_y, out_width, out_height);
        }

        // set up input texture (output of previous pass) and apply filter settings
        GLuint filter = pass[i].filter;
        if (filter == GLSL_UNDEFINED)
        {
            if (lastpass && Settings.BilinearFilter)
                filter = GL_LINEAR;
            else
                filter = GL_NEAREST;
        }

        glBindTexture(GL_TEXTURE_2D, pass[i - 1].texture);
        glPixelStorei(GL_UNPACK_ROW_LENGTH, (GLint) pass[i - 1].width);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, pass[i].wrap_mode);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, pass[i].wrap_mode);

        if (pass[i].mipmap_input)
            glGenerateMipmap(GL_TEXTURE_2D);

        glUseProgram(pass[i].program);
#ifdef USE_SLANG
        if (using_slang)
            slang_set_shader_vars(i, lastpass && direct_lastpass);
        else
#endif
            set_shader_vars(i, lastpass && direct_lastpass);

        glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
        glClear(GL_COLOR_BUFFER_BIT);
        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);

#ifdef USE_SLANG
        if (lastpass && !direct_lastpass)
        {
            glBindTexture(GL_TEXTURE_2D, pass[i].texture);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, pass[i].wrap_mode);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, pass[i].wrap_mode);

            int out_x = 0, out_y = 0, out_width = 0, out_height = 0;
            vpcallback (pass[i].width, pass[i].height,
                        viewport_x, viewport_y,
                        viewport_width, viewport_height,
                        &out_x, &out_y,
                        &out_width, &out_height);

            glViewport(out_x, out_y, out_width, out_height);
            glBindFramebuffer(GL_FRAMEBUFFER, saved_framebuffer);
            glClear(GL_COLOR_BUFFER_BIT);
            glBindFramebuffer(GL_READ_FRAMEBUFFER, pass[i].fbo);
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, saved_framebuffer);
            glBlitFramebuffer(0, 0, pass[i].width, pass[i].height, out_x,
                              out_height + out_y, out_x + out_width, out_y,
                              GL_COLOR_BUFFER_BIT,
                              Settings.BilinearFilter ? GL_LINEAR : GL_NEAREST);
        }
        // reset vertex attribs set in set_shader_vars
        if (using_slang)
            slang_clear_shader_vars();
        else
#endif
            clear_shader_vars();

        if (pass[i].srgb)
        {
            glDisable(GL_FRAMEBUFFER_SRGB);
        }
    }

    // Disable framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, saved_framebuffer);
    glActiveTexture(GL_TEXTURE0);
    glUseProgram(0);

    // Pop back of previous frame stack and use as upload buffer
    if (prev_frame.size() > 0)
    {
        GLint internal_format;
        orig = prev_frame.back().texture;
        prev_frame.pop_back();

        GLSLPass *newprevframe = new GLSLPass;
        newprevframe->width = width;
        newprevframe->height = height;
        newprevframe->texture = pass[0].texture;

        prev_frame.push_front(*newprevframe);
        glBindTexture(GL_TEXTURE_2D, newprevframe->texture);
        delete newprevframe;
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
}

void GLSLShader::register_uniforms()
{
    max_prev_frame = 0;
    char varname[100];

    for (unsigned int i = 1; i < pass.size(); i++)
    {
        GLSLUniforms *u = &pass[i].unif;
        GLuint program = pass[i].program;

        glUseProgram (program);

        GLint mvp = glGetUniformLocation(program, "MVPMatrix");
        if (mvp > -1)
            glUniformMatrix4fv(mvp, 1, GL_FALSE, mvp_ortho);

        u->Texture = glGetUniformLocation(program, "Texture");
        u->InputSize = glGetUniformLocation(program, "InputSize");
        u->OutputSize = glGetUniformLocation(program, "OutputSize");
        u->TextureSize = glGetUniformLocation(program, "TextureSize");

        u->TexCoord = glGetAttribLocation(program, "TexCoord");
        u->LUTTexCoord = glGetAttribLocation(program, "LUTTexCoord");
        u->VertexCoord = glGetAttribLocation(program, "VertexCoord");

        u->FrameCount = glGetUniformLocation(program, "FrameCount");
        u->FrameDirection = glGetUniformLocation(program, "FrameDirection");

        u->OrigTexture = glGetUniformLocation(program, "OrigTexture");
        u->OrigInputSize = glGetUniformLocation(program, "OrigInputSize");
        u->OrigTextureSize = glGetUniformLocation(program, "OrigTextureSize");
        u->OrigTexCoord = glGetAttribLocation(program, "OrigTexCoord");

        u->Prev[0].Texture = glGetUniformLocation(program, "PrevTexture");
        u->Prev[0].InputSize = glGetUniformLocation(program, "PrevInputSize");
        u->Prev[0].TextureSize = glGetUniformLocation(program, "PrevTextureSize");
        u->Prev[0].TexCoord = glGetAttribLocation(program, "PrevTexCoord");

        if (u->Prev[0].Texture > -1)
            max_prev_frame = 1;

        for (unsigned int j = 1; j < 7; j++)
        {
            sprintf(varname, "Prev%dTexture", j);
            u->Prev[j].Texture = glGetUniformLocation(program, varname);
            sprintf(varname, "Prev%dInputSize", j);
            u->Prev[j].InputSize = glGetUniformLocation(program, varname);
            sprintf(varname, "Prev%dTextureSize", j);
            u->Prev[j].TextureSize = glGetUniformLocation(program, varname);
            sprintf(varname, "Prev%dTexCoord", j);
            u->Prev[j].TexCoord = glGetAttribLocation(program, varname);

            if (u->Prev[j].Texture > -1)
                max_prev_frame = j + 1;
        }
        for (unsigned int j = 0; j < pass.size(); j++)
        {
            sprintf(varname, "Pass%dTexture", j);
            u->Pass[j].Texture = glGetUniformLocation(program, varname);
            sprintf(varname, "Pass%dInputSize", j);
            u->Pass[j].InputSize = glGetUniformLocation(program, varname);
            sprintf(varname, "Pass%dTextureSize", j);
            u->Pass[j].TextureSize = glGetUniformLocation(program, varname);
            sprintf(varname, "Pass%dTexCoord", j);
            u->Pass[j].TexCoord = glGetAttribLocation(program, varname);
            if (u->Pass[j].Texture)
                u->max_pass = j;

            sprintf(varname, "PassPrev%dTexture", j);
            u->PassPrev[j].Texture = glGetUniformLocation(program, varname);
            sprintf(varname, "PassPrev%dInputSize", j);
            u->PassPrev[j].InputSize = glGetUniformLocation(program, varname);
            sprintf(varname, "PassPrev%dTextureSize", j);
            u->PassPrev[j].TextureSize = glGetUniformLocation(program, varname);
            sprintf(varname, "PassPrev%dTexCoord", j);
            u->PassPrev[j].TexCoord = glGetAttribLocation(program, varname);
            if (u->PassPrev[j].Texture > -1)
                u->max_prevpass = j;
        }

        for (unsigned int j = 0; j < lut.size(); j++)
        {
            u->Lut[j] = glGetUniformLocation(program, lut[j].id);
        }

        for (unsigned int j = 0; j < param.size(); j++)
        {
            param[j].unif[i] = glGetUniformLocation(program, param[j].id);
        }
    }

    glUseProgram(0);
}

void GLSLShader::set_shader_vars(unsigned int p, bool inverted)
{
    unsigned int texunit = 0;
    unsigned int offset = 0;

    if (inverted)
        offset = 8;
    GLSLUniforms *u = &pass[p].unif;

    GLint mvp = glGetUniformLocation(pass[p].program, "MVPMatrix");
    if (mvp > -1)
        glUniformMatrix4fv(mvp, 1, GL_FALSE, mvp_ortho);

#define setUniform2fv(uni, val) if (uni > -1) glUniform2fv(uni, 1, val);
#define setUniform1f(uni, val) if (uni > -1) glUniform1f(uni, val);
#define setUniform1i(uni, val) if (uni > -1) glUniform1i(uni, val);
#define setTexture1i(uni, val) \
    if (uni > -1) \
    { \
        glActiveTexture(GL_TEXTURE0 + texunit); \
        glBindTexture(GL_TEXTURE_2D, val); \
        glUniform1i(uni, texunit); \
        texunit++; \
    }
    // We use non-power-of-two textures,
    // so no need to mess with input size/texture size
#define setTexCoords(attr) \
    if (attr > -1) \
    { \
        glEnableVertexAttribArray(attr); \
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, 0, (void *)(sizeof(float) * offset)); \
        vaos.push_back(attr); \
    }
#define setTexCoordsNoOffset(attr) \
    if (attr > -1) \
    { \
        glEnableVertexAttribArray(attr); \
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, 0, NULL); \
        vaos.push_back(attr); \
    }

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 16, tex_coords, GL_STATIC_DRAW);

    float inputSize[2] = { (float)pass[p - 1].width, (float)pass[p - 1].height };
    float outputSize[2] = { (float)pass[p].width, (float)pass[p].height };

    setTexture1i(u->Texture, pass[p - 1].texture);
    setUniform2fv(u->InputSize, inputSize);
    setUniform2fv(u->OutputSize, outputSize);
    setUniform2fv(u->TextureSize, inputSize);

    unsigned int shaderFrameCnt = frame_count;
    if (pass[p].frame_count_mod)
        shaderFrameCnt %= pass[p].frame_count_mod;
    setUniform1i(u->FrameCount, shaderFrameCnt);
    setUniform1i(u->FrameDirection, Settings.Rewinding ? -1 : 1);

    setTexCoords(u->TexCoord);
    setTexCoords(u->LUTTexCoord);
    setTexCoordsNoOffset(u->VertexCoord);

    // Orig parameter
    float orig_videoSize[2] = { (float)pass[0].width, (float)pass[0].height };

    setUniform2fv(u->OrigInputSize,  orig_videoSize);
    setUniform2fv(u->OrigTextureSize, orig_videoSize);
    setTexture1i(u->OrigTexture, pass[0].texture);
    setTexCoords(u->OrigTexCoord);

    // Prev parameter
    if (max_prev_frame >= 1 && prev_frame[0].width > 0) {
        float prevSize[2] = { (float)prev_frame[0].width, (float)prev_frame[0].height };

        setUniform2fv(u->Prev[0].InputSize, prevSize);
        setUniform2fv(u->Prev[0].TextureSize, prevSize);
        setTexture1i(u->Prev[0].Texture, prev_frame[0].texture);
        setTexCoords(u->Prev[0].TexCoord);
    }

    // Prev[1-6] parameters
    for (unsigned int i = 1; i < prev_frame.size(); i++)
    {
        if (prev_frame[i].width <= 0)
            break;

        float prevSize[2] = { (float)prev_frame[i].width, (float)prev_frame[i].height };

        setUniform2fv(u->Prev[i].InputSize, prevSize);
        setUniform2fv(u->Prev[i].TextureSize, prevSize);
        setTexture1i(u->Prev[i].Texture, prev_frame[i].texture);
        setTexCoords(u->Prev[i].TexCoord);
    }

    // LUT parameters
    for (unsigned int i = 0; i < lut.size(); i++)
    {
        setTexture1i(u->Lut[i], lut[i].texture);
    }

    // PassX parameters, only for third pass and up
    if (p > 2) {
        for (unsigned int i = 1; i < p - 1; i++) {
            float passSize[2] = { (float)pass[i].width, (float)pass[i].height };
            setUniform2fv(u->Pass[i].InputSize, passSize);
            setUniform2fv(u->Pass[i].TextureSize, passSize);
            setTexture1i(u->Pass[i].Texture, pass[i].texture);
            setTexCoords(u->Pass[i].TexCoord);
        }
    }

    // PassPrev parameter
    for (unsigned int i = 0; i < p; i++)
    {
        float passSize[2] = { (float)pass[i].width, (float)pass[i].height };
        setUniform2fv(u->PassPrev[p - i].InputSize, passSize);
        setUniform2fv(u->PassPrev[p - i].TextureSize, passSize);
        setTexture1i(u->PassPrev[p - i].Texture, pass[i].texture);
        setTexCoords(u->PassPrev[p - i].TexCoord);
    }

    // User and Preset Parameters
    for (unsigned int i = 0; i < param.size(); i++)
    {
        setUniform1f(param[i].unif[p], param[i].val);
    }

    glActiveTexture(GL_TEXTURE0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void GLSLShader::clear_shader_vars()
{
    for (unsigned int i = 0; i < vaos.size(); i++)
        glDisableVertexAttribArray(vaos[i]);

    vaos.clear();
}

#define outs(s, v) fprintf(file, "%s%d = \"%s\"\n", s, i, v)
#define outf(s, v) fprintf(file, "%s%d = \"%f\"\n", s, i, v)
#define outd(s, v) fprintf(file, "%s%d = \"%d\"\n", s, i, v)
void GLSLShader::save(const char *filename)
{
    FILE *file = fopen(filename, "wb");
    if (!file)
    {
        printf("Couldn't save shader config to %s\n", filename);
    }

    fprintf(file, "shaders = \"%d\"\n", (unsigned int)pass.size() - 1);

    for (unsigned int pn = 1; pn < pass.size(); pn++)
    {
        GLSLPass *p = &pass[pn];
        unsigned int i = pn - 1;

        outs("shader", p->filename);
        if (p->filter != GLSL_UNDEFINED)
        {
            outs("filter_linear", p->filter == GL_LINEAR ? "true" : "false");
        }
        outs("wrap_mode", wrap_mode_enum_to_string(p->wrap_mode));
        outs("alias", p->alias);
        outs("float_framebuffer", p->fp ? "true" : "false");
        outs("srgb_framebuffer", p->srgb ? "true" : "false");
        outs("scale_type_x", scale_enum_to_string(p->scale_type_x));
        outs("mipmap_input", p->mipmap_input ? "true" : "false");
        if (p->scale_type_x == GLSL_ABSOLUTE)
            outd("scale_x", (int)p->scale_x);
        else
            outf("scale_x", p->scale_x);

        outs("scale_type_y", scale_enum_to_string(p->scale_type_y));
        if (p->scale_type_y == GLSL_ABSOLUTE)
            outd("scale_y", (int)p->scale_y);
        else
            outf("scale_y", p->scale_y);

        if (p->frame_count_mod)
            outd("frame_count_mod", p->frame_count_mod);
    }

    if (param.size() > 0)
    {
        fprintf(file, "parameters = \"");
        for (unsigned int i = 0; i < param.size(); i++)
        {
            fprintf(file, "%s%c", param[i].id, (i == param.size() - 1) ? '\"' : ';');
        }
        fprintf(file, "\n");
    }

    for (unsigned int i = 0; i < param.size(); i++)
    {
        fprintf(file, "%s = \"%f\"\n", param[i].id, param[i].val);
    }

    if (lut.size() > 0)
    {
        fprintf(file, "textures = \"");
        for (unsigned int i = 0; i < lut.size(); i++)
        {
            fprintf(file, "%s%c", lut[i].id, (i == lut.size() - 1) ? '\"' : ';');
        }
        fprintf(file, "\n");
    }

    for (unsigned int i = 0; i < lut.size(); i++)
    {
        fprintf(file, "%s = \"%s\"\n", lut[i].id, lut[i].filename);
        fprintf(file, "%s_linear = \"%s\"\n", lut[i].id, lut[i].filter == GL_LINEAR || lut[i].filter == GL_LINEAR_MIPMAP_LINEAR ? "true" : "false");
        fprintf(file, "%s_wrap_mode = \"%s\"\n", lut[i].id, wrap_mode_enum_to_string(lut[i].wrap_mode));
        fprintf(file, "%s_mipmap = \"%s\"\n", lut[i].id, lut[i].mipmap ? "true" : "false");
    }

    fclose(file);
}
#undef outf
#undef outs
#undef outd


void GLSLShader::destroy()
{
    glBindTexture(GL_TEXTURE_2D, 0);
    glUseProgram(0);
    glActiveTexture(GL_TEXTURE0);

    for (unsigned int i = 1; i < pass.size(); i++)
    {
        glDeleteProgram(pass[i].program);
        glDeleteTextures(1, &pass[i].texture);
        glDeleteFramebuffers(1, &pass[i].fbo);
#ifdef USE_SLANG
        if (using_slang)
        {
            if (pass[i].uses_feedback)
                glDeleteTextures(1, &pass[i].feedback_texture);
            if (pass[i].ubo_buffer.size() > 0)
                glDeleteBuffers(1, &pass[i].ubo);
        }
#endif
    }

    for (unsigned int i = 0; i < lut.size(); i++)
    {
        glDeleteTextures(1, &lut[i].texture);
    }

    for (unsigned int i = 0; i < prev_frame.size(); i++)
    {
        glDeleteTextures(1, &prev_frame[i].texture);
    }

    glDeleteBuffers(1, &vbo);

    param.clear();
    pass.clear();
    lut.clear();
    prev_frame.clear();
    conf.Clear();
}
