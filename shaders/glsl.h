/*****************************************************************************\
     Snes9x - Portable Super Nintendo Entertainment System (TM) emulator.
                This file is licensed under the Snes9x License.
   For further information, consult the LICENSE file in the root directory.
\*****************************************************************************/

#ifndef __GLSL_H
#define __GLSL_H

#include "../../conffile.h"
#include "shader_platform.h"
#include <deque>
#include <limits.h>
#include <vector>

static const unsigned int glsl_max_passes = 20;

typedef void (*GLSLViewportCallback)(int source_width, int source_height,
                                     int viewport_x, int viewport_y,
                                     int viewport_width, int viewport_height,
                                     int *out_dst_x, int *out_dst_y,
                                     int *out_dst_width, int *out_dst_height);

enum GLSLScaleType
{
    GLSL_NONE = 0,
    GLSL_SOURCE,
    GLSL_VIEWPORT,
    GLSL_ABSOLUTE
};

enum GLSLFilter
{
    GLSL_UNDEFINED = 0,
};

typedef struct
{
    GLint Texture;
    GLint InputSize;
    GLint TextureSize;
    GLint TexCoord;
} GLSLUniformMetrics;

typedef struct
{
    GLint Texture;
    GLint InputSize;
    GLint OutputSize;
    GLint TextureSize;

    GLint FrameCount;
    GLint FrameDirection;

    GLint TexCoord;
    GLint LUTTexCoord;
    GLint VertexCoord;

    GLint OrigTexture;
    GLint OrigInputSize;
    GLint OrigTextureSize;
    GLint OrigTexCoord;

    unsigned int max_pass;
    unsigned int max_prevpass;
    GLSLUniformMetrics Prev[7];
    GLSLUniformMetrics Pass[glsl_max_passes];
    GLSLUniformMetrics PassPrev[glsl_max_passes];
    GLint Lut[9];
} GLSLUniforms;

// Size must always follow texture type
enum
{
    SL_INVALID = 0,
    SL_PASSTEXTURE = 1,
    SL_PASSSIZE = 2,
    SL_PREVIOUSFRAMETEXTURE = 3,
    SL_PREVIOUSFRAMESIZE = 4,
    SL_LUTTEXTURE = 5,
    SL_LUTSIZE = 6,
    SL_MVP = 7,
    SL_FRAMECOUNT = 8,
    SL_PARAM = 9,
    SL_FEEDBACK = 10
};

typedef struct
{
    // Source
    int type;
    int num;

    // Output
    GLint location; // -1 Indicates UBO
    GLint offset;
} SlangUniform;

typedef struct
{
    char filename[PATH_MAX];
    char alias[256];
    int scale_type_x;
    int scale_type_y;
    float scale_x;
    float scale_y;
    bool fp;
    bool srgb;
    int frame_count_mod;
    unsigned int frame_count;

    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint texture;
    GLuint wrap_mode;
    GLuint fbo;
    GLuint width;
    GLuint height;
    GLuint filter;
    bool mipmap_input;

    GLSLUniforms unif;
#ifdef USE_SLANG
    GLuint format;
    std::vector<SlangUniform> uniforms;
    std::vector<uint8_t> ubo_buffer;
    GLuint ubo;
    bool uses_feedback = false;
    GLuint feedback_texture;
#endif
} GLSLPass;

typedef struct
{
    char id[256];
    char filename[PATH_MAX];
    GLuint filter;
    GLuint texture;
    GLuint wrap_mode;
    bool mipmap;
    int width;
    int height;
} GLSLLut;

typedef struct
{
    char name[PATH_MAX];
    char id[256];
    float min;
    float max;
    float val;
    float step;
    GLint unif[glsl_max_passes];
} GLSLParam;

typedef struct
{
    bool load_shader(char *filename);
    bool load_shader_preset_file(char *filename);
    void render(GLuint &orig, int width, int height, int viewport_x,
                int viewport_y, int viewport_width, int viewport_height,
                GLSLViewportCallback vpcallback);
    void set_shader_vars(unsigned int pass, bool inverted);
    void clear_shader_vars();
    void read_shader_file_with_includes(std::string filename,
                                        std::vector<std::string> &lines,
                                        int p);
    GLuint compile_shader(std::vector<std::string> &lines, const char *aliases,
                          const char *defines, GLuint type, GLuint *out);
    void save(const char *filename);
    void destroy();
    void register_uniforms();

    ConfigFile conf;

    std::vector<GLSLPass> pass;
    std::vector<GLSLLut> lut;
    std::vector<GLSLParam> param;
    int max_prev_frame;
    std::deque<GLSLPass> prev_frame;
    std::vector<GLuint> vaos;

    unsigned int frame_count;
    GLuint vbo;

    bool using_slang;
#ifdef USE_SLANG
    std::string slang_get_stage(std::vector<std::string> &lines,
                                std::string name);
    GLint slang_compile(std::vector<std::string> &lines, std::string stage);
    void slang_introspect();
    void slang_set_shader_vars(int p, bool inverted);
    void slang_clear_shader_vars();

    bool using_feedback;
#endif
} GLSLShader;

#endif
