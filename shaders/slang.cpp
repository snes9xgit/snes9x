#include "glsl.h"
#include "shader_helpers.h"
#include <vector>
#include <string>
#include <sstream>
#include <glslang/Public/ShaderLang.h>
#include <SPIRV/GlslangToSpv.h>
#include "SPIRV-Cross/spirv_cross.hpp"
#include "SPIRV-Cross/spirv_glsl.hpp"

std::string GLSLShader::slang_get_stage(std::vector<std::string> &lines,
                                        std::string name)
{
    std::ostringstream output;
    bool in_stage = true;

    if (name.empty())
        return std::string("");

    for (auto &line : lines)
    {
        if (line.find("#pragma stage") != std::string::npos)
        {
            if (line.find(std::string("#pragma stage ") + name) !=
                std::string::npos)
                in_stage = true;
            else
                in_stage = false;
        }
        else if (line.find("#pragma name") != std::string::npos ||
                 line.find("#pragma format") != std::string::npos)
        {
        }
        else if (in_stage)
        {
            output << line << '\n';
        }
    }

    return output.str();
}

static void printuniforms(std::vector<SlangUniform> &unif)
{
    for (int i = 0; i < (int)unif.size(); i++)
    {
        SlangUniform &u = unif[i];
        printf("Uniform %d: ", i);
        switch (u.type)
        {
        case SL_PREVIOUSFRAMETEXTURE:
            printf("OriginalHistory%d\n", u.num);
            break;
        case SL_PASSTEXTURE:
            printf("Pass%d\n", u.num);
            break;
        case SL_LUTTEXTURE:
            printf("User%d\n", u.num);
            break;
        case SL_PREVIOUSFRAMESIZE:
            printf("OriginalHistorySize%d\n", u.num);
            break;
        case SL_PASSSIZE:
            printf("PassSize%d\n", u.num);
            break;
        case SL_LUTSIZE:
            printf("UserSize%d\n", u.num);
            break;
        case SL_MVP:
            printf("MVP\n");
            break;
        case SL_FRAMECOUNT:
            printf("FrameCount\n");
            break;
        case SL_PARAM:
            printf("Parameter %d\n", u.num);
            break;
        }
    }
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

    MATCH("A2B10G10R10_UNORM_PACK32", GL_RGB10_A2);
    MATCH("A2B10G10R10_UINT_PACK32", GL_RGB10_A2UI);

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

void GLSLShader::slang_parse_pragmas(std::vector<std::string> &lines, int p)
{
    pass[p].format = GL_RGBA;

    for (auto &line : lines)
    {
        if (line.find("#pragma name") != std::string::npos)
        {
            sscanf(line.c_str(), "#pragma name %255s", pass[p].alias);
        }
        else if (line.find("#pragma format") != std::string::npos)
        {
            char format[256];
            sscanf(line.c_str(), "#pragma format %255s", format);
            pass[p].format = string_to_format(format);
            if (pass[p].format == GL_RGBA16F || pass[p].format == GL_RGBA32F)
                pass[p].fp = TRUE;
            else if (pass[p].format == GL_SRGB8_ALPHA8)
                pass[p].srgb = TRUE;
        }
    }
}

static const TBuiltInResource DefaultTBuiltInResource = {
    /* .MaxLights = */ 32,
    /* .MaxClipPlanes = */ 6,
    /* .MaxTextureUnits = */ 32,
    /* .MaxTextureCoords = */ 32,
    /* .MaxVertexAttribs = */ 64,
    /* .MaxVertexUniformComponents = */ 4096,
    /* .MaxVaryingFloats = */ 64,
    /* .MaxVertexTextureImageUnits = */ 32,
    /* .MaxCombinedTextureImageUnits = */ 80,
    /* .MaxTextureImageUnits = */ 32,
    /* .MaxFragmentUniformComponents = */ 4096,
    /* .MaxDrawBuffers = */ 32,
    /* .MaxVertexUniformVectors = */ 128,
    /* .MaxVaryingVectors = */ 8,
    /* .MaxFragmentUniformVectors = */ 16,
    /* .MaxVertexOutputVectors = */ 16,
    /* .MaxFragmentInputVectors = */ 15,
    /* .MinProgramTexelOffset = */ -8,
    /* .MaxProgramTexelOffset = */ 7,
    /* .MaxClipDistances = */ 8,
    /* .MaxComputeWorkGroupCountX = */ 65535,
    /* .MaxComputeWorkGroupCountY = */ 65535,
    /* .MaxComputeWorkGroupCountZ = */ 65535,
    /* .MaxComputeWorkGroupSizeX = */ 1024,
    /* .MaxComputeWorkGroupSizeY = */ 1024,
    /* .MaxComputeWorkGroupSizeZ = */ 64,
    /* .MaxComputeUniformComponents = */ 1024,
    /* .MaxComputeTextureImageUnits = */ 16,
    /* .MaxComputeImageUniforms = */ 8,
    /* .MaxComputeAtomicCounters = */ 8,
    /* .MaxComputeAtomicCounterBuffers = */ 1,
    /* .MaxVaryingComponents = */ 60,
    /* .MaxVertexOutputComponents = */ 64,
    /* .MaxGeometryInputComponents = */ 64,
    /* .MaxGeometryOutputComponents = */ 128,
    /* .MaxFragmentInputComponents = */ 128,
    /* .MaxImageUnits = */ 8,
    /* .MaxCombinedImageUnitsAndFragmentOutputs = */ 8,
    /* .MaxCombinedShaderOutputResources = */ 8,
    /* .MaxImageSamples = */ 0,
    /* .MaxVertexImageUniforms = */ 0,
    /* .MaxTessControlImageUniforms = */ 0,
    /* .MaxTessEvaluationImageUniforms = */ 0,
    /* .MaxGeometryImageUniforms = */ 0,
    /* .MaxFragmentImageUniforms = */ 8,
    /* .MaxCombinedImageUniforms = */ 8,
    /* .MaxGeometryTextureImageUnits = */ 16,
    /* .MaxGeometryOutputVertices = */ 256,
    /* .MaxGeometryTotalOutputComponents = */ 1024,
    /* .MaxGeometryUniformComponents = */ 1024,
    /* .MaxGeometryVaryingComponents = */ 64,
    /* .MaxTessControlInputComponents = */ 128,
    /* .MaxTessControlOutputComponents = */ 128,
    /* .MaxTessControlTextureImageUnits = */ 16,
    /* .MaxTessControlUniformComponents = */ 1024,
    /* .MaxTessControlTotalOutputComponents = */ 4096,
    /* .MaxTessEvaluationInputComponents = */ 128,
    /* .MaxTessEvaluationOutputComponents = */ 128,
    /* .MaxTessEvaluationTextureImageUnits = */ 16,
    /* .MaxTessEvaluationUniformComponents = */ 1024,
    /* .MaxTessPatchComponents = */ 120,
    /* .MaxPatchVertices = */ 32,
    /* .MaxTessGenLevel = */ 64,
    /* .MaxViewports = */ 16,
    /* .MaxVertexAtomicCounters = */ 0,
    /* .MaxTessControlAtomicCounters = */ 0,
    /* .MaxTessEvaluationAtomicCounters = */ 0,
    /* .MaxGeometryAtomicCounters = */ 0,
    /* .MaxFragmentAtomicCounters = */ 8,
    /* .MaxCombinedAtomicCounters = */ 8,
    /* .MaxAtomicCounterBindings = */ 1,
    /* .MaxVertexAtomicCounterBuffers = */ 0,
    /* .MaxTessControlAtomicCounterBuffers = */ 0,
    /* .MaxTessEvaluationAtomicCounterBuffers = */ 0,
    /* .MaxGeometryAtomicCounterBuffers = */ 0,
    /* .MaxFragmentAtomicCounterBuffers = */ 1,
    /* .MaxCombinedAtomicCounterBuffers = */ 1,
    /* .MaxAtomicCounterBufferSize = */ 16384,
    /* .MaxTransformFeedbackBuffers = */ 4,
    /* .MaxTransformFeedbackInterleavedComponents = */ 64,
    /* .MaxCullDistances = */ 8,
    /* .MaxCombinedClipAndCullDistances = */ 8,
    /* .MaxSamples = */ 4,
    /* .maxMeshOutputVerticesNV = */ 256,
    /* .maxMeshOutputPrimitivesNV = */ 512,
    /* .maxMeshWorkGroupSizeX_NV = */ 32,
    /* .maxMeshWorkGroupSizeY_NV = */ 1,
    /* .maxMeshWorkGroupSizeZ_NV = */ 1,
    /* .maxTaskWorkGroupSizeX_NV = */ 32,
    /* .maxTaskWorkGroupSizeY_NV = */ 1,
    /* .maxTaskWorkGroupSizeZ_NV = */ 1,
    /* .maxMeshViewCountNV = */ 4,

    /* .limits = */ {
        /* .nonInductiveForLoops = */ 1,
        /* .whileLoops = */ 1,
        /* .doWhileLoops = */ 1,
        /* .generalUniformIndexing = */ 1,
        /* .generalAttributeMatrixVectorIndexing = */ 1,
        /* .generalVaryingIndexing = */ 1,
        /* .generalSamplerIndexing = */ 1,
        /* .generalVariableIndexing = */ 1,
        /* .generalConstantMatrixVectorIndexing = */ 1,
    }};

GLint GLSLShader::slang_compile(std::vector<std::string> &lines,
                                std::string stage)
{
    static bool ProcessInitialized = false;

    if (!ProcessInitialized)
    {
        ShInitialize();
        ProcessInitialized = true;
    }

    std::string source = slang_get_stage(lines, stage);

    EShLanguage language;
    if (!stage.compare("fragment"))
        language = EShLangFragment;
    else
        language = EShLangVertex;
    glslang::TShader shader(language);

    const char *csource = source.c_str();
    shader.setStrings(&csource, 1);

    EShMessages messages =
        (EShMessages)(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);

    std::string debug;
    auto forbid_includer = glslang::TShader::ForbidIncluder();

    if (!shader.preprocess(&DefaultTBuiltInResource, 100, ENoProfile, false,
                           false, messages, &debug, forbid_includer))
    {
        puts(debug.c_str());
        return -1;
    }

    if (!shader.parse(&DefaultTBuiltInResource, 100, false, messages))
    {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return -1;
    }

    glslang::TProgram program;
    program.addShader(&shader);

    if (!program.link(messages))
    {
        puts(shader.getInfoLog());
        puts(shader.getInfoDebugLog());
        return -1;
    }

    std::vector<uint32_t> spirv;
    glslang::GlslangToSpv(*program.getIntermediate(language), spirv);

    spirv_cross::CompilerGLSL glsl(std::move(spirv));

    auto resources = glsl.get_shader_resources();
    if (resources.push_constant_buffers.size() > 1 ||
        resources.uniform_buffers.size() > 1)
    {
        puts("slang shader doesn't comply with spec:\n"
             " Too many UBOs or push constant buffers.");
        return -1;
    }

    if (language == EShLangFragment)
    {
        for (auto &rsrc : resources.stage_inputs)
        {
            // Some of the converted shaders have unmatched declarations for
            // this in fragment shader
            if (glsl.get_name(rsrc.id) == "FragCoord")
            {
                glsl.set_remapped_variable_state(rsrc.id, true);
                glsl.set_name(rsrc.id, "gl_FragCoord");
            }
        }
    }

    spirv_cross::CompilerGLSL::Options opts;
    opts.version = gl_version() * 10;
    opts.vulkan_semantics = false;
    glsl.set_common_options(opts);

    std::string glsl_source = glsl.compile();

    GLint status = 0;
    GLchar *cstring = (GLchar *)glsl_source.c_str();

    GLint shaderid = glCreateShader(
        language == EShLangFragment ? GL_FRAGMENT_SHADER : GL_VERTEX_SHADER);
    glShaderSource(shaderid, 1, &cstring, NULL);
    glCompileShader(shaderid);

    glGetShaderiv(shaderid, GL_COMPILE_STATUS, &status);

    char info_log[1024];
    glGetShaderInfoLog(shaderid, 1024, NULL, info_log);
    if (*info_log)
        puts(info_log);

    if (status == GL_FALSE)
        return -1;

    return shaderid;
}

static inline bool isalldigits(std::string str)
{
    for (auto c : str)
    {
        if (!isdigit(c))
            return false;
    }

    return true;
}

void GLSLShader::slang_introspect()
{
    max_prev_frame = 0;

    for (int i = 1; i < (int)pass.size(); i++)
    {
        GLSLPass &p = pass[i];

        int num_uniforms;
        glGetProgramiv(p.program, GL_ACTIVE_UNIFORMS, &num_uniforms);
        std::vector<GLuint> indices(num_uniforms);
        std::vector<GLint> block_indices(num_uniforms);
        std::vector<GLint> offsets(num_uniforms);
        std::vector<std::string> names;
        std::vector<GLint> locations(num_uniforms);

        for (int j = 0; j < num_uniforms; j++)
            indices[j] = j;

        glGetActiveUniformsiv(p.program, num_uniforms, indices.data(), GL_UNIFORM_BLOCK_INDEX, block_indices.data());
        glGetActiveUniformsiv(p.program, num_uniforms, indices.data(), GL_UNIFORM_OFFSET, offsets.data());

        for (int j = 0; j < num_uniforms; j++)
        {
            char name[1024];
            glGetActiveUniformName(p.program, j, 1024, NULL, name);
            names.push_back(std::string(name));
            locations[j] = glGetUniformLocation(p.program, name);
        }

        for (int j = 0; j < num_uniforms; j++)
        {
            SlangUniform u = { 0, 0, 0, 0 };
            std::string name;

            if (locations[j] == -1)
            {
                u.location = -1;
                u.offset = offsets[j];
            }
            else
                u.location = locations[j];

            // Strip off any container
            size_t dotloc = names[j].find('.');
            if (dotloc != std::string::npos)
                name = names[j].substr(dotloc + 1);
            else
                name = names[j];

            auto indexedtexorsize = [&](std::string needle, int type) -> bool {
                if (name.find(needle) == 0)
                {
                    std::string tmp = name.substr(needle.length());
                    if (tmp.find("Size") == 0)
                    {
                        u.type = type + 1;
                        tmp = tmp.substr(4);
                    }
                    else
                        u.type = type;

                    if (isalldigits(tmp))
                    {
                        u.num = std::stoi(tmp);
                        return true;
                    }
                }
                return false;
            };

            if (!name.compare("MVP"))
            {
                u.type = SL_MVP;
            }
            else if (!name.compare("Original"))
            {
                u.type = SL_PASSTEXTURE;
                u.num  = 0;
            }
            else if (!name.compare("OriginalSize"))
            {
                u.type = SL_PASSSIZE;
                u.num = 0;
            }
            else if (!name.compare("Source"))
            {
                u.type = SL_PASSTEXTURE;
                u.num = i - 1;
            }
            else if (!name.compare("SourceSize"))
            {
                u.type = SL_PASSSIZE;
                u.num = i - 1;
            }
            else if (!name.compare("OutputSize"))
            {
                u.type = SL_PASSSIZE;
                u.num = i;
            }
            else if (!name.compare("FrameCount"))
            {
                u.type = SL_FRAMECOUNT;
            }
            else if (indexedtexorsize("OriginalHistory", SL_PREVIOUSFRAMETEXTURE))
            {
                // OriginalHistory0 is just this frame's pass 0
                if (u.num == 0)
                {
                    if (u.type == SL_PREVIOUSFRAMETEXTURE)
                        u.type = SL_PASSTEXTURE;
                    if (u.type == SL_PREVIOUSFRAMESIZE)
                        u.type = SL_PASSSIZE;
                }
                if (u.num > max_prev_frame)
                    max_prev_frame = u.num;
            }
            else if (indexedtexorsize("PassOutput", SL_PASSTEXTURE))
            {
                u.num++;
            }
            else if (indexedtexorsize("User", SL_LUTTEXTURE))
            {
            }
            if (u.type != SL_INVALID)
            {
                p.uniforms.push_back(u);
                continue;
            }

            bool matched_lut = false;
            for (int k = 0; k < (int)lut.size(); k++)
            {
                std::string lutname(lut[k].id);

                if (name.compare(lutname) == 0)
                {
                    u.type = SL_LUTTEXTURE;
                    u.num  = k;
                    matched_lut = true;
                }
                else if (name.compare(lutname + "Size") == 0)
                {
                    u.type = SL_LUTSIZE;
                    u.num  = k;
                    matched_lut = true;
                }
            }
            if (matched_lut)
            {
                p.uniforms.push_back(u);
                continue;
            }

            bool matched_alias = false;
            for (int k = 0; k < i; k++)
            {
                std::string alias(pass[k].alias);

                if (name.compare(alias) == 0)
                {
                    u.type = SL_PASSTEXTURE;
                    u.num  = k;
                    matched_alias = true;
                }
                else if (name.compare(alias + "Size") == 0)
                {
                    u.type = SL_PASSSIZE;
                    u.num  = k;
                    matched_alias = true;
                }
            }
            if (matched_alias)
            {
                p.uniforms.push_back(u);
                continue;
            }

            u.type = SL_INVALID;
            for (int k = 0; k < (int)param.size(); k++)
            {
                if (name.compare(param[k].id) == 0)
                {
                    u.type = SL_PARAM;
                    u.num  = k;
                    break;
                }
            }
            if (u.type != SL_INVALID)
                p.uniforms.push_back(u);
        }

        bool uniform_block = false;
        for (auto &u : p.uniforms)
            if (u.location == -1)
            {
                uniform_block = true;
                break;
            }

        GLint uniform_block_size = 0;
        if (uniform_block)
        {
            glGetActiveUniformBlockiv(p.program, 0, GL_UNIFORM_BLOCK_DATA_SIZE,
                                      &uniform_block_size);
            glGenBuffers(1, &p.ubo);
            p.ubo_buffer.resize(uniform_block_size);
        }
    }
}

static const GLfloat coords[] =  { 0.0f, 0.0f, 0.0f, 1.0f, // Vertex Positions
                                   1.0f, 0.0f, 0.0f, 1.0f,
                                   0.0f, 1.0f, 0.0f, 1.0f,
                                   1.0f, 1.0f, 0.0f, 1.0f,
                                   0.0f, 0.0f, // Tex coords regular + 16
                                   1.0f, 0.0f,
                                   0.0f, 1.0f,
                                   1.0f, 1.0f,
                                   0.0f, 1.0f, // Tex coords inverted + 24
                                   1.0f, 1.0f,
                                   0.0f, 0.0f,
                                   1.0f, 0.0f };

static const GLfloat mvp_ortho[16] = { 2.0f,  0.0f,  0.0f,  0.0f,
                                       0.0f,  2.0f,  0.0f,  0.0f,
                                       0.0f,  0.0f, -1.0f,  0.0f,
                                      -1.0f, -1.0f,  0.0f,  1.0f };

void GLSLShader::slang_clear_shader_vars()
{
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glBindBuffer(GL_UNIFORM_BUFFER, 0);
}

void GLSLShader::slang_set_shader_vars(int p)
{
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(GLfloat) * 32, coords, GL_STATIC_DRAW);

    GLint attr = glGetAttribLocation(pass[p].program, "Position");
    if (attr > -1)
    {
        glEnableVertexAttribArray(attr);
        glVertexAttribPointer(attr, 4, GL_FLOAT, GL_FALSE, 0, (void *)(0));
    }

    attr = glGetAttribLocation(pass[p].program, "TexCoord");
    if (attr > -1)
    {
        GLfloat *offset = 0;
        offset += 16;
        if (p == (int)pass.size() - 1)
            offset += 8;

        glEnableVertexAttribArray(attr);
        glVertexAttribPointer(attr, 2, GL_FLOAT, GL_FALSE, 0, (void *)(offset));
    }

    glBindBuffer(GL_ARRAY_BUFFER, 0);

    uint8_t *ubo = pass[p].ubo_buffer.data();
    GLint texunit = 0;

    for (auto &u : pass[p].uniforms)
    {
        switch (u.type)
        {
        case SL_PREVIOUSFRAMETEXTURE:
        case SL_PASSTEXTURE:
        case SL_LUTTEXTURE:
            glActiveTexture(GL_TEXTURE0 + texunit);

            if (u.type == SL_PASSTEXTURE)
                glBindTexture(GL_TEXTURE_2D, pass[u.num].texture);
            else if (u.type == SL_PREVIOUSFRAMETEXTURE)
                glBindTexture(GL_TEXTURE_2D, prev_frame[u.num - 1].texture);
            else if (u.type == SL_LUTTEXTURE)
                glBindTexture(GL_TEXTURE_2D, lut[u.num].texture);

            if (u.location == -1)
                *((GLint *)(ubo + u.offset)) = texunit;
            else
                glUniform1i(u.location, texunit);
            texunit++;
            break;

        case SL_PREVIOUSFRAMESIZE:
        case SL_PASSSIZE:
        case SL_LUTSIZE:
        {
            GLfloat size[4] = { 0.0f, 0.0f, 0.0f, 0.0f };
            if (u.type == SL_PASSSIZE)
            {
                size[0] = (GLfloat)pass[u.num].width;
                size[1] = (GLfloat)pass[u.num].height;
                size[2] = (GLfloat)1.0f / pass[u.num].width;
                size[3] = (GLfloat)1.0f / pass[u.num].height;
            }
            else if (u.type == SL_PREVIOUSFRAMESIZE)
            {
                if (u.num < 1)
                    u.num = 0;
                size[0] = (GLfloat)prev_frame[u.num - 1].width;
                size[1] = (GLfloat)prev_frame[u.num - 1].height;
                size[2] = (GLfloat)1.0f / prev_frame[u.num - 1].width;
                size[3] = (GLfloat)1.0f / prev_frame[u.num - 1].height;
            }
            else if (u.type == SL_LUTSIZE)
            {
                size[0] = (GLfloat)lut[u.num].width;
                size[1] = (GLfloat)lut[u.num].height;
                size[2] = (GLfloat)1.0f / lut[u.num].width;
                size[3] = (GLfloat)1.0f / lut[u.num].height;
            }

            if (u.location == -1)
            {
                GLfloat *data = (GLfloat *)(ubo + u.offset);
                data[0] = size[0];
                data[1] = size[1];
                data[2] = size[2];
                data[3] = size[3];
            }
            else
                glUniform4fv(u.location, 1, size);
            break;
        }

        case SL_MVP:
            if (u.location == -1)
            {

                GLfloat *data = (GLfloat *)(ubo + u.offset);
                for (int i = 0; i < 16; i++)
                    data[i] = mvp_ortho[i];
            }
            else
                glUniformMatrix4fv(u.location, 1, GL_FALSE, mvp_ortho);
            break;

        case SL_FRAMECOUNT:
            if (u.location == -1)
                *((GLuint *)(ubo + u.offset)) = frame_count;
            else
                glUniform1ui(u.location, frame_count);
            break;

        case SL_PARAM:
            if (u.location == -1)
                *((GLfloat *)(ubo + u.offset)) = param[u.num].val;
            else
                glUniform1f(u.location, param[u.num].val);
            break;
        }
    }

    if (pass[p].ubo_buffer.size() > 0)
    {
        glBindBuffer(GL_UNIFORM_BUFFER, pass[p].ubo);
        glBufferData(GL_UNIFORM_BUFFER, pass[p].ubo_buffer.size(), ubo,
                     GL_DYNAMIC_DRAW);
        glBindBufferBase(GL_UNIFORM_BUFFER, 0, pass[p].ubo);
        glUniformBlockBinding(pass[p].program, 0, 0);
    }
}
