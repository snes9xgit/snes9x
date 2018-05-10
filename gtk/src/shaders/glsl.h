#include <vector>
#include <deque>
#include <limits.h>
#include <epoxy/gl.h>

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
    GLSLUniformMetrics Pass[20];
    GLSLUniformMetrics PassPrev[20];
    GLint Lut[9];

} GLSLUniforms;

typedef struct
{
    char filename[PATH_MAX];
    int scale_type_x;
    int scale_type_y;
    float scale_x;
    float scale_y;
    bool fp;
    int frame_count_mod;
    unsigned int frame_count;

    GLuint program;
    GLuint vertex_shader;
    GLuint fragment_shader;
    GLuint texture;
    GLuint fbo;
    GLuint width;
    GLuint height;
    GLuint filter;

    GLSLUniforms unif;
} GLSLPass;

typedef struct
{
    char id[PATH_MAX];
    char filename[PATH_MAX];
    GLuint filter;
    GLuint texture;
} GLSLLut;

typedef struct
{
    char name[PATH_MAX];
    float min;
    float max;
    float def;
    float step;
} GLSLParam;

typedef struct
{
    bool load_shader (char *filename);
    bool load_shader_file (char *filename);
    void render (GLuint &orig, int width, int height, int viewport_width, int viewport_height);
    void set_shader_vars (int pass);
    void clear_shader_vars (void);
    void destroy (void);
    void register_uniforms (void);

    std::vector<GLSLPass> pass;
    std::vector<GLSLLut> lut;
    std::vector<GLSLParam> param;
    int max_prev_frame;
    std::deque<GLSLPass> prev_frame;
    std::vector<GLuint> vaos;

    unsigned int frame_count;
    GLuint vbo;
    GLuint prev_fbo;
    GLfloat *fa;

} GLSLShader;
