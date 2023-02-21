#include "slang_preset.hpp"
#include "../external/SPIRV-Cross/spirv.hpp"
#include "slang_helpers.hpp"
#include "slang_preset_ini.hpp"

#include <algorithm>
#include <string>
#include <cstdio>
#include <vector>
#include <cctype>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <future>
#include "../external/SPIRV-Cross/spirv_cross.hpp"
#include "../external/SPIRV-Cross/spirv_glsl.hpp"
#include "slang_shader.hpp"

using std::string;
using std::to_string;

SlangPreset::SlangPreset()
{
}

SlangPreset::~SlangPreset()
{
}

bool SlangPreset::load_preset_file(string filename)
{
    if (!ends_with(filename, ".slangp"))
        return false;

    IniFile conf;

    if (!conf.load_file(filename))
        return false;

    int num_passes = conf.get_int("shaders", 0);

    if (num_passes <= 0)
        return false;

    passes.resize(num_passes);

    int index;
    auto key = [&](string s) -> string { return s + to_string(index); };
    auto iGetBool = [&](string s, bool def = false) -> bool {
        return conf.get_bool(key(s), def);
    };
    auto iGetString = [&](string s, string def = "") -> string {
        return conf.get_string(key(s), def);
    };
    auto iGetFloat = [&](string s, float def = 1.0f) -> float {
        return conf.get_float(key(s), def);
    };
    auto iGetInt = [&](string s, int def = 0) -> int {
        return conf.get_int(key(s), def);
    };

    for (index = 0; index < num_passes; index++)
    {
        auto &shader = passes[index];
        shader.filename = iGetString("shader", "");
        canonicalize(shader.filename, conf.get_source(key("shader")));
        shader.alias = iGetString("alias", "");
        shader.filter_linear = iGetBool("filter_linear");
        shader.mipmap_input = iGetBool("mipmap_input");
        shader.float_framebuffer = iGetBool("float_framebuffer");
        shader.srgb_framebuffer = iGetBool("srgb_framebuffer");
        shader.frame_count_mod = iGetInt("frame_count_mod", 0);
        shader.wrap_mode = iGetString("wrap_mode");
        // Is this correct? It gives priority to _x and _y scale types.
        string scale_type = iGetString("scale_type", "undefined");
        shader.scale_type_x = iGetString("scale_type_x", scale_type);
        shader.scale_type_y = iGetString("scale_type_y", scale_type);
        shader.scale_x = iGetFloat("scale_x", 1.0f);
        shader.scale_y = iGetFloat("scale_y", 1.0f);
        if (conf.exists(key("scale")))
        {
            float scale = iGetFloat("scale");
            shader.scale_x = scale;
            shader.scale_y = scale;
        }
    }

    string texture_string = conf.get_string("textures", "");
    if (!texture_string.empty())
    {
        auto texture_list = split_string(texture_string, ';');

        for (auto &id : texture_list)
        {
            Texture texture;
            texture.id = trim(id);
            textures.push_back(texture);
        }

        for (auto &t : textures)
        {
            t.wrap_mode = conf.get_string(t.id + "_wrap_mode", "");
            t.mipmap = conf.get_bool(t.id + "_mipmap", false);
            t.linear = conf.get_bool(t.id + "_linear", false);
            t.filename = conf.get_string(t.id, "");
            canonicalize(t.filename, conf.get_source(t.id));
        }
    }

    if (!cache_directory.empty())
        for (auto &p : passes)
            p.set_cache_directory(cache_directory);

    std::vector<std::future<bool>> futures;
    for (size_t i = 0; i < passes.size(); i++)
    {
        futures.push_back(std::async(std::launch::async, [&, i]() -> bool {
            return passes[i].load_file();
        }));
    }
    if (!std::all_of(futures.begin(), futures.end(), [](auto &f) { return f.get(); }))
        return false;

    gather_parameters();

    for (auto &p : parameters)
    {
        auto value_str = conf.get_string(p.id, "");
        if (!value_str.empty())
        {
            p.val = atof(value_str.c_str());
            if (p.val < p.min)
                p.val = p.min;
            else if (p.val > p.max)
                p.val = p.max;
        }
    }

    return true;
}

void SlangPreset::set_cache_directory(std::string filename)
{
    cache_directory = filename;
}

/*
    Aggregates the parameters from individual stages and separate shader files,
    resolving duplicates.
*/
void SlangPreset::gather_parameters()
{
    parameters.clear();
    for (auto &s : passes)
    {
        for (auto &p : s.parameters)
        {
            auto it = std::find_if(parameters.begin(), parameters.end(), [&p](SlangShader::Parameter &needle) {
                return (needle.id == p.id);
            });

            if (it == parameters.end())
                parameters.push_back(p);
        }
    }
}

/*
    Print to stdout the entire shader preset chain and parameters, minus source
    and SPIRV output
*/
void SlangPreset::print()
{
    printf("Number of Shaders: %zu\n", passes.size());
    for (size_t i = 0; i < passes.size(); i++)
    {
        auto &s = passes[i];
        printf(" Shader \n");
        printf("  filename:          %s\n", s.filename.c_str());
        printf("  alias:             %s\n", s.alias.c_str());
        printf("  filter_linear:     %d\n", s.filter_linear);;
        printf("  mipmap_input:      %d\n", s.mipmap_input);
        printf("  float_framebuffer: %d\n", s.float_framebuffer);
        printf("  srgb_framebuffer:  %d\n", s.srgb_framebuffer);
        printf("  frame_count_mod:   %d\n", s.frame_count_mod);
        printf("  wrap_mode:         %s\n", s.wrap_mode.c_str());
        printf("  scale_type_x:      %s\n", s.scale_type_x.c_str());
        printf("  scale_type_y:      %s\n", s.scale_type_y.c_str());
        printf("  scale_x:           %f\n", s.scale_x);
        printf("  scale_y:           %f\n", s.scale_y);
        printf("  pragma lines: ");
        for (auto &p : s.pragma_stage_lines)
            printf("%zu ", p);
        printf("\n");
        printf("  Number of parameters:    %zu\n", s.parameters.size());
        for (auto &p : s.parameters)
        {
            printf("   %s \"%s\" min: %f max: %f val: %f step: %f digits: %d\n",
                p.id.c_str(), p.name.c_str(), p.min, p.max, p.val, p.step, p.significant_digits);
        }

        printf("  Uniforms: %zu\n", s.uniforms.size());
        printf("   UBO size: %zu, binding: %d\n", s.ubo_size, s.ubo_binding);
        printf("   Push Constant block size: %zu\n", s.push_constant_block_size);
        for (auto &u : s.uniforms)
        {
            const char *strings[] = {
                "Output Size",
                "Previous Frame Size",
                "Pass Size",
                "Pass Feedback Size",
                "Lut Size",
                "MVP Matrix",
                "Frame Count",
                "Parameter"
            };

            const char *block = u.block == SlangShader::Uniform::UBO ? "UBO" : "Push Constants";
            const char *type = strings[u.type];

            printf("   ");
            switch (u.type)
            {
            case SlangShader::Uniform::Type::ViewportSize:
                printf("%s at offset %zu in %s\n", type, u.offset, block);
                break;
            case SlangShader::Uniform::Type::PreviousFrameSize:
            case SlangShader::Uniform::Type::PassSize:
            case SlangShader::Uniform::Type::PassFeedbackSize:
            case SlangShader::Uniform::Type::LutSize:
                printf("%s %d at offset %zu in %s\n", type, u.specifier, u.offset, block);
                break;
            case SlangShader::Uniform::Type::MVP:
            case SlangShader::Uniform::Type::FrameCount:
                printf("%s at offset %zu in %s\n", type, u.offset, block);
                break;
            case SlangShader::Uniform::Type::Parameter:
                printf("%s #%d named \"%s\" at offset %zu in %s\n", type, u.specifier, parameters[u.specifier].id.c_str(), u.offset, block);
            default:
                break;
            }
        }

        printf("  Samplers: %zu\n", s.samplers.size());
        for (auto &sampler : s.samplers)
        {
            const char *strings[] =
            {
                "Previous Frame",
                "Pass",
                "Pass Feedback",
                "Lut",
            };

            const char *type = strings[sampler.type];

            printf("   ");
            switch (sampler.type)
            {
                case SlangShader::Sampler::Type::PreviousFrame:
                case SlangShader::Sampler::Type::Pass:
                case SlangShader::Sampler::Type::PassFeedback:
                    printf("%s %d at binding %d\n", type, sampler.specifier, sampler.binding);
                    break;
                case SlangShader::Sampler::Type::Lut:
                    printf("%s %d \"%s\" at binding %d\n", type, sampler.specifier, textures[sampler.specifier].id.c_str(), sampler.binding);
                    break;
                default:
                    break;
            }
        }
    }

    printf("Num textures: %zu\n", textures.size());
    for (size_t i = 0; i < textures.size(); i++)
    {
        auto &t = textures[i];
        printf(" Texture %zu\n", i);
        printf("  id:        %s\n", t.id.c_str());
        printf("  filename:  %s\n", t.filename.c_str());
        printf("  wrap_mode: %s\n", t.wrap_mode.c_str());
        printf("  mipmap:    %d\n", t.mipmap);
        printf("  linear:    %d\n", t.linear);
    }

    printf("Parameters: %zu count\n", parameters.size());
    for (size_t i = 0; i < parameters.size(); i++)
    {
        auto &p = parameters[i];

        printf(" Parameter: %zu\n", i);
        printf("  id:         %s\n", p.id.c_str());
        printf("  name:       %s\n", p.name.c_str());
        printf("  min:        %f\n", p.min);
        printf("  max:        %f\n", p.max);
        printf("  step:       %f\n", p.step);
        printf("  sigdigits:  %d\n", p.significant_digits);
        printf("  value:      %f\n", p.val);
    }

    printf("Oldest previous frame used: %d\n", oldest_previous_frame);
}

bool SlangPreset::match_sampler_semantic(const string &name, int pass, SlangShader::Sampler::Type &type, int &specifier)
{
    auto match_with_specifier = [&name, &specifier](string prefix) -> bool {
        if (name.compare(0, prefix.length(), prefix) != 0)
            return false;

        if (name.length() <= prefix.length())
            return false;

        for (auto iter = name.begin() + prefix.length(); iter < name.end(); iter++)
        {
            if (!std::isdigit(*iter))
                return false;
        }

        specifier = std::stoi(name.substr(prefix.length()));
        return true;
    };

    if (name == "Original")
    {
        type = SlangShader::Sampler::Type::Pass;
        specifier = -1;
        return true;
    }
    else if (name == "Source")
    {
        type = SlangShader::Sampler::Type::Pass;
        specifier = pass - 1;
        return true;
    }
    else if (match_with_specifier("OriginalHistory"))
    {
        type = SlangShader::Sampler::Type::PreviousFrame;
        return true;
    }
    else if (match_with_specifier("PassOutput"))
    {
        type = SlangShader::Sampler::Type::Pass;
        return true;
    }
    else if (match_with_specifier("PassFeedback"))
    {
        type = SlangShader::Sampler::Type::PassFeedback;
        return true;
    }
    else if (match_with_specifier("User"))
    {
        type = SlangShader::Sampler::Type::Lut;
        return true;
    }
    else
    {
        for (size_t i = 0; i < passes.size(); i++)
        {
            if (passes[i].alias == name)
            {
                type = SlangShader::Sampler::Type::Pass;
                specifier = i;
                return true;
            }
            else if (passes[i].alias + "Feedback" == name)
            {
                type = SlangShader::Sampler::Type::PassFeedback;
                specifier = i;
                return true;
            }
        }
    }

    for (size_t i = 0; i < textures.size(); i++)
    {
        if (name == textures[i].id)
        {
            type = SlangShader::Sampler::Type::Lut;
            specifier = i;
            return true;
        }
    }

    return false;
}

bool SlangPreset::match_buffer_semantic(const string &name, int pass, SlangShader::Uniform::Type &type, int &specifier)
{
    if (name == "MVP")
    {
        type = SlangShader::Uniform::Type::MVP;
        return true;
    }

    if (name == "FrameCount")
    {
        type = SlangShader::Uniform::Type::FrameCount;
        return true;
    }

    if (name == "FinalViewportSize")
    {
        type = SlangShader::Uniform::Type::ViewportSize;
        return true;
    }

    if (name == "FrameDirection")
    {
        type = SlangShader::Uniform::Type::FrameDirection;
        return true;
    }

    if (name.find("Size") != string::npos)
    {
        auto match = [&name, &specifier](string prefix) -> bool {
            if (name.compare(0, prefix.length(), prefix) != 0)
                return false;

            if (name.compare(prefix.length(), 4, "Size") != 0)
                return false;

            if (prefix.length() + 4 < name.length())
                specifier = std::stoi(name.substr(prefix.length() + 4));

            return true;
        };

        if (match("Original"))
        {
            type = SlangShader::Uniform::Type::PassSize;
            specifier = -1;
            return true;
        }
        else if (match("Source"))
        {
            type = SlangShader::Uniform::Type::PassSize;
            specifier = pass - 1;
            return true;
        }
        else if (match("Output"))
        {
            type = SlangShader::Uniform::Type::PassSize;
            specifier = pass;
            return true;
        }
        else if (match("OriginalHistory"))
        {
            type = SlangShader::Uniform::Type::PreviousFrameSize;
            return true;
        }
        else if (match("PassOutput"))
        {
            type = SlangShader::Uniform::Type::PassSize;
            return true;
        }
        else if (match("PassFeedback"))
        {
            type = SlangShader::Uniform::Type::PassFeedbackSize;
            return true;
        }
        else if (match("User"))
        {
            type = SlangShader::Uniform::Type::LutSize;
            return true;
        }

        for (size_t i = 0; i < passes.size(); i++)
        {
            if (match(passes[i].alias))
            {
                type = SlangShader::Uniform::Type::PassSize;
                specifier = i;
                return true;
            }
        }

        for (size_t i = 0; i < textures.size(); i++)
        {
            if (match(textures[i].id))
            {
                type = SlangShader::Uniform::Type::LutSize;
                return true;
            }
        }
    }

    for (size_t i = 0; i < parameters.size(); i++)
    {
        if (name == parameters[i].id)
        {
            type = SlangShader::Uniform::Type::Parameter;
            specifier = i;
            return true;
        }
    }

    return false;
}

/*
    Introspect an individual shader pass, collecting external resource info
    in order to build uniform blocks.
*/
bool SlangPreset::introspect_shader(SlangShader &shader, int pass, SlangShader::Stage stage)
{
    spirv_cross::CompilerGLSL cross(stage == SlangShader::Stage::Vertex ? shader.vertex_shader_spirv : shader.fragment_shader_spirv);

    auto res = cross.get_shader_resources();

    if (res.push_constant_buffers.size() > 1)
    {
        printf("%s: Too many push constant buffers.\n", shader.filename.c_str());
        return false;
    }
    else if (res.uniform_buffers.size() > 1)
    {
        printf("%s: Too many uniform buffers.\n", shader.filename.c_str());
        return false;
    }

    auto exists = [&shader](const SlangShader::Uniform &uniform) -> bool {
        for (const auto &u : shader.uniforms)
        {
            if (u.block == uniform.block &&
                u.offset == uniform.offset &&
                u.specifier == uniform.specifier &&
                u.type == uniform.type)
            {
                return true;
            }
        }
        return false;
    };

    if (res.push_constant_buffers.size() == 0)
    {
        shader.push_constant_block_size = 0;
    }
    else
    {
        auto &pcb = res.push_constant_buffers[0];
        auto &pcb_type = cross.get_type(pcb.base_type_id);
        shader.push_constant_block_size = cross.get_declared_struct_size(pcb_type);

        for (size_t i = 0; i < pcb_type.member_types.size(); i++)
        {
            auto name = cross.get_member_name(pcb.base_type_id, i);
            auto offset = cross.get_member_decoration(pcb.base_type_id, i, spv::DecorationOffset);

            SlangShader::Uniform::Type semantic_type;
            int specifier;
            if (match_buffer_semantic(name, pass, semantic_type, specifier))
            {
                SlangShader::Uniform uniform{ SlangShader::Uniform::Block::PushConstant,
                                              offset,
                                              semantic_type,
                                              specifier };

                if (!exists(uniform))
                    shader.uniforms.push_back(uniform);
            }
            else
            {
                printf("%s: Failed to match push constant semantic: \"%s\"\n", shader.filename.c_str(), name.c_str());
            }
        }
    }

    if (res.uniform_buffers.size() == 0)
    {
        shader.ubo_size = 0;
    }
    else
    {
        auto &ubo = res.uniform_buffers[0];
        auto &ubo_type = cross.get_type(ubo.base_type_id);
        shader.ubo_size = cross.get_declared_struct_size(ubo_type);
        shader.ubo_binding = cross.get_decoration(ubo.base_type_id, spv::DecorationBinding);

        for (size_t i = 0; i < ubo_type.member_types.size(); i++)
        {
            auto name = cross.get_member_name(ubo.base_type_id, i);
            auto offset = cross.get_member_decoration(ubo.base_type_id, i, spv::DecorationOffset);

            SlangShader::Uniform::Type semantic_type;
            int specifier;
            if (match_buffer_semantic(name, pass, semantic_type, specifier))
            {
                SlangShader::Uniform uniform{ SlangShader::Uniform::Block::UBO,
                                              offset,
                                              semantic_type,
                                              specifier };

                if (!exists(uniform))
                    shader.uniforms.push_back(uniform);
            }
            else
            {
                printf("%s: Failed to match uniform buffer semantic: \"%s\"\n", shader.filename.c_str(), name.c_str());
            }
        }
    }

    if (res.sampled_images.size() == 0 && stage == SlangShader::Stage::Fragment)
    {
        printf("No sampled images found in fragment shader.\n");
        return false;
    }

    if (res.sampled_images.size() > 0 && stage == SlangShader::Stage::Vertex)
    {
        printf("Sampled image found in vertex shader.\n");
        return false;
    }

    if (stage == SlangShader::Stage::Fragment)
    {
        for (auto &image : res.sampled_images)
        {
            SlangShader::Sampler::Type semantic_type;
            int specifier;

            if (match_sampler_semantic(image.name, pass, semantic_type, specifier))
            {
                int binding = cross.get_decoration(image.id, spv::DecorationBinding);
                shader.samplers.push_back({ semantic_type, specifier, binding });
            }
            else
            {
                printf("%s: Failed to match sampler semantic: \"%s\"\n", shader.filename.c_str(), image.name.c_str());
                return false;
            }
        }
    }

    return true;
}

/*
    Introspect all of preset's shaders.
*/
bool SlangPreset::introspect()
{
    for (size_t i = 0; i < passes.size(); i++)
    {
        if (!introspect_shader(passes[i], i, SlangShader::Stage::Vertex))
            return false;
        if (!introspect_shader(passes[i], i, SlangShader::Stage::Fragment))
            return false;
    }

    oldest_previous_frame = 0;
    uses_feedback = false;
    last_pass_uses_feedback = false;

    for (auto &p : passes)
    {
        for (auto &s : p.samplers)
        {
            if (s.type == SlangShader::Sampler::PreviousFrame && s.specifier > oldest_previous_frame)
                oldest_previous_frame = s.specifier;
            if (s.type == SlangShader::Sampler::PassFeedback)
            {
                uses_feedback = true;
                if (s.specifier == (int)passes.size() - 1)
                    last_pass_uses_feedback = true;
            }
        }
    }

    return true;
}

bool SlangPreset::save_to_file(std::string filename)
{
    std::ofstream out(filename);
    if (!out.is_open())
        return false;

    auto outs = [&](std::string key, std::string value) { out << key << " = \"" << value << "\"\n"; };
    auto outb = [&](std::string key, bool value) { outs(key, value ? "true" : "false"); };
    auto outa = [&](std::string key, auto value) { outs(key, to_string(value)); };

    outa("shaders", passes.size());

    for (size_t i = 0; i < passes.size(); i++)
    {
        auto &pass = passes[i];
        auto indexed = [i](std::string str) { return str + to_string(i); };
        outs(indexed("shader"), pass.filename);
        outb(indexed("filter_linear"), pass.filter_linear);
        outs(indexed("wrap_mode"), pass.wrap_mode);
        outs(indexed("alias"), pass.alias);
        outb(indexed("float_framebuffer"), pass.float_framebuffer);
        outb(indexed("srgb_framebuffer"), pass.srgb_framebuffer);
        outb(indexed("mipmap_input"), pass.mipmap_input);
        outs(indexed("scale_type_x"), pass.scale_type_x);
        outs(indexed("scale_type_y"), pass.scale_type_y);
        outa(indexed("scale_x"), pass.scale_x);
        outa(indexed("scale_y"), pass.scale_y);
        outa(indexed("frame_count_mod"), pass.frame_count_mod);
    }

    if (parameters.size() > 0)
    {
        std::string parameter_list = "";
        for (size_t i = 0; i < parameters.size(); i++)
        {
            parameter_list += parameters[i].id;
            if (i < parameters.size() - 1)
                parameter_list += ";";
        }
        outs("parameters", parameter_list);
    }

    for (auto &item : parameters)
        outa(item.id, item.val);

    if (textures.size() > 0)
    {
        std::string texture_list = "";
        for (size_t i = 0; i < textures.size(); i++)
        {
            texture_list += textures[i].id;
            if (i < textures.size() - 1)
                texture_list += ";";
        }
        outs("textures", texture_list);
    }

    for (auto &item : textures)
    {
        outs(item.id, item.filename);
        outb(item.id + "_linear", item.linear);
        outs(item.id + "_wrap_mode", item.wrap_mode);
        outb(item.id + "_mipmap", item.mipmap);
    }

    out.close();

    return true;
}