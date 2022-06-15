#pragma once

#include "slang_shader.hpp"
#include "vulkan/vulkan_core.h"

#include <string>
#include <vector>

struct SlangPreset
{
    SlangPreset();
    ~SlangPreset();

    void print();
    bool load_preset_file(std::string filename);
    bool introspect();
    bool introspect_shader(SlangShader &s, int index, SlangShader::Stage stage);
    bool match_buffer_semantic(const std::string &name, int pass, SlangShader::Uniform::Type &type, int &specifier);
    bool match_sampler_semantic(const std::string &name, int pass, SlangShader::Sampler::Type &type, int &specifier);
    void gather_parameters();
    bool save_to_file(std::string filename);

    struct Texture
    {
        std::string id;
        std::string filename;
        std::string wrap_mode;
        bool mipmap;
        bool linear;
    };

    std::vector<SlangShader> passes;
    std::vector<Texture> textures;
    std::vector<SlangShader::Parameter> parameters;
    int oldest_previous_frame;
    bool uses_feedback;
    bool last_pass_uses_feedback;
};