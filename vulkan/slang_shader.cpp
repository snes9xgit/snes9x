#include "slang_shader.hpp"
#include "slang_helpers.hpp"

#include <ostream>
#include <string>
#include <string_view>
#include <sstream>
#include <vector>
#include <fstream>
#include "../external/glslang/glslang/Public/ShaderLang.h"
#include "../external/glslang/SPIRV/GlslangToSpv.h"
#include "../external/glslang/StandAlone/ResourceLimits.h"
#include "../external/xxh64/xxh64.hpp"
#include <filesystem>

using std::string;
using std::vector;

SlangShader::SlangShader()
{
    ubo_size = 0;
}

SlangShader::~SlangShader()
{
}

void SlangShader::set_cache_directory(std::string dir)
{
    cache_directory = dir;
}

/*
    Recursively load shader file and included files into memory, applying
    #include and #pragma directives. Will strip all directives except
    #pragma stage.
*/
bool SlangShader::preprocess_shader_file(string filename, vector<string> &lines)
{
    std::ifstream stream(filename.c_str(), std::ios::binary);

    if (stream.fail())
        return false;

    string line;

    while (std::getline(stream, line, '\n'))
    {
        std::string_view sv(line);
        trim(sv);

        if (sv.empty())
            continue;
        else if (sv.compare(0, 8, "#include") == 0)
        {
            sv.remove_prefix(8);
            trim(sv);
            if (sv.length() && sv[0] == '\"' && sv[sv.length() - 1] == '\"')
            {
                sv.remove_prefix(1);
                sv.remove_suffix(1);
                string include_file(sv);
                canonicalize(include_file, filename);
                preprocess_shader_file(include_file, lines);
            }
            else
            {
                printf("Syntax error: #include.\n");
                return false;
            }
        }
        else if (sv.compare(0, 17, "#pragma parameter") == 0)
        {
            Parameter p{};
            sv.remove_prefix(17);

            auto tokens = split_string_quotes(string{ sv });
            if (tokens.size() < 5)
            {
                printf("Syntax error: #pragma parameter\n");
                printf("%s\n", string{ sv }.c_str());
                return false;
            }

            p.id = tokens[0];
            p.name = tokens[1];
            p.val = std::stof(tokens[2]);
            p.min = std::stof(tokens[3]);
            p.max = std::stof(tokens[4]);
            if (tokens.size() >= 6)
                p.step = std::stof(tokens[5]);

            p.significant_digits = 0;
            for (size_t i = 2; i < tokens.size(); i++)
            {
                int significant_digits = get_significant_digits(tokens[i]);
                if (significant_digits > p.significant_digits)
                    p.significant_digits = significant_digits;
            }

            parameters.push_back(p);

            continue;
        }
        else if (sv.compare(0, 12, "#pragma name") == 0)
        {
            alias = sv.substr(13);
        }
        else if (sv.compare(0, 14, "#pragma format") == 0)
        {
            format = sv.substr(15);
        }
        else
        {
            if (sv.compare(0, 13, "#pragma stage") == 0)
                pragma_stage_lines.push_back(lines.size());

            lines.push_back(line);
        }
    }

    return true;
}

/*
    Use the #pragma stage lines to divide the file into separate vertex and
    fragment shaders. Must have called preprocess beforehand.
*/
void SlangShader::divide_into_stages(const std::vector<std::string> &lines)
{
    enum
    {
        vertex,
        fragment,
        both
    } stage;

    stage = both;
    std::ostringstream vertex_shader_stream;
    std::ostringstream fragment_shader_stream;

    auto p = pragma_stage_lines.begin();
    for (size_t i = 0; i < lines.size(); i++)
    {
        if (p != pragma_stage_lines.end() && i == *p)
        {
            if (lines[i].find("vertex") != string::npos)
                stage = vertex;
            else if (lines[i].find("fragment") != string::npos)
                stage = fragment;

            p++;
        }
        else
        {
            if (stage == vertex || stage == both)
                vertex_shader_stream << lines[i] << '\n';
            if (stage == fragment || stage == both)
                fragment_shader_stream << lines[i] << '\n';
        }
    }

    vertex_shader_string = vertex_shader_stream.str();
    fragment_shader_string = fragment_shader_stream.str();
}

/*
    Load a shader file into memory, preprocess, divide and compile it to
    SPIRV bytecode. Returns true on success.
*/
bool SlangShader::load_file(string new_filename)
{
    if (!new_filename.empty())
        filename = new_filename;

    pragma_stage_lines.clear();
    vector<string> lines;
    if (!preprocess_shader_file(filename, lines))
    {
        printf("Failed to load shader file: %s\n", filename.c_str());
        return false;
    }
    divide_into_stages(lines);

    if (!generate_spirv())
        return false;

    return true;
}

static void Initializeglslang()
{
    static bool ProcessInitialized = false;

    if (!ProcessInitialized)
    {
        glslang::InitializeProcess();
        ProcessInitialized = true;
    }
}

std::vector<uint32_t> SlangShader::generate_spirv(std::string shader_string, std::string stage)
{
    Initializeglslang();
    const EShMessages messages = (EShMessages)(EShMsgDefault | EShMsgVulkanRules | EShMsgSpvRules);
    string debug;
    auto forbid_includer = glslang::TShader::ForbidIncluder();

    auto language = stage == "vertex" ? EShLangVertex : stage == "fragment" ? EShLangFragment : EShLangCompute;

    glslang::TShader shaderTShader(language);

    auto compile = [&](glslang::TShader &shader, string &shader_string, std::vector<uint32_t> &spirv) -> bool {
        const char *source = shader_string.c_str();
        shader.setStrings(&source, 1);
        if (!shader.preprocess(&glslang::DefaultTBuiltInResource, 450, ENoProfile, false, false, messages, &debug, forbid_includer))
            return false;
        if (!shader.parse(&glslang::DefaultTBuiltInResource, 450, false, messages))
            return false;

        glslang::TProgram program;
        program.addShader(&shader);
        if (!program.link(messages))
            return false;

        glslang::GlslangToSpv(*program.getIntermediate(shader.getStage()), spirv);
        return true;
    };

    std::vector<uint32_t> spirv;

    if (!compile(shaderTShader, shader_string, spirv))
    {
        printf("%s\n%s\n%s\n", debug.c_str(), shaderTShader.getInfoLog(), shaderTShader.getInfoDebugLog());
    }

    return spirv;

}

std::string SlangShader::get_hash_filename(const std::string &shader_string)
{
    uint64_t hash = xxh64::hash(shader_string.c_str(), fragment_shader_string.size(), 56);
    char hash_hex_string[32];
    snprintf(hash_hex_string, 32, "%016lx.spv", hash);
    auto cache_path = std::filesystem::path(cache_directory);
    cache_path /= hash_hex_string;

    return cache_path.string();
}

static const char *header = "SPVCACHE1";
static size_t header_size = 9;

bool SlangShader::load_cache(std::string hash_filename, std::vector<uint32_t> &output)
{
    if (cache_directory.empty())
        return false;
    if (!std::filesystem::exists(hash_filename))
        return false;
    auto size = std::filesystem::file_size(hash_filename);
    size -= header_size;
    if ((size & 3) != 0)
        return false;

    std::ifstream file(hash_filename, std::ios::binary);
    char headerblock[header_size];
    file.read(headerblock, header_size);
    if (memcmp(headerblock, header, header_size))
    {
        printf("Invalid cache file format %s.\n", headerblock);
        return false;
    }

    output.resize(size / 4);
    file.read((char *)output.data(), size);

    return true;
}

void SlangShader::save_cache(std::string hash_filename, std::vector<uint32_t> &spv)
{
    if (cache_directory.empty())
        return;
    std::ofstream file(hash_filename, std::ios::binary);
    file.write(header, header_size);
    file.write((char *)spv.data(), spv.size() * 4);
}

/*
    Generate SPIRV from separate preprocessed fragment and vertex shaders.
    Must have called divide_into_stages beforehand. Returns true on success.
*/
bool SlangShader::generate_spirv()
{
    fragment_shader_spirv.clear();
    vertex_shader_spirv.clear();

    auto vertex_cache_filename = get_hash_filename(vertex_shader_string);
    auto fragment_cache_filename = get_hash_filename(fragment_shader_string);

    if (load_cache(vertex_cache_filename, vertex_shader_spirv))
        if (load_cache(fragment_cache_filename, fragment_shader_spirv))
            return true;

    vertex_shader_spirv = generate_spirv(vertex_shader_string, "vertex");
    fragment_shader_spirv = generate_spirv(fragment_shader_string, "fragment");

    save_cache(vertex_cache_filename, vertex_shader_spirv);
    save_cache(fragment_cache_filename, fragment_shader_spirv);

    return true;
}