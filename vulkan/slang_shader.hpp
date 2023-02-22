#pragma once
#include <string>
#include <vector>

struct SlangShader
{
    struct Parameter
    {
        std::string name;
        std::string id;
        float min;
        float max;
        float val;
        float step;
        int significant_digits;
    };

    struct Uniform
    {
        enum Block
        {
           UBO,
           PushConstant,
        };

        enum Type
        {
            ViewportSize,
            PreviousFrameSize,
            PassSize,
            PassFeedbackSize,
            LutSize,
            MVP,
            FrameCount,
            Parameter,
            FrameDirection
        };

        Block block;
        size_t offset;

        Type type;
        int specifier;
    };

    struct Sampler
    {
        enum Type
        {
            PreviousFrame,
            Pass,
            PassFeedback,
            Lut
        };

        Type type;
        int specifier;
        int binding;
    };

    enum class Stage
    {
        Vertex,
        Fragment
    };

    SlangShader();
    ~SlangShader();

    bool preprocess_shader_file(std::string filename, std::vector<std::string> &lines);
    void set_base_path(std::string filename);
    bool load_file(std::string new_filename = "");
    void divide_into_stages(const std::vector<std::string> &lines);
    bool generate_spirv();
    static void initialize_glslang();
    static std::vector<uint32_t> generate_spirv(std::string shader_string, std::string stage);

    std::string filename;
    std::string alias;
    bool filter_linear;
    bool mipmap_input;
    bool float_framebuffer;
    bool srgb_framebuffer;
    int frame_count_mod;
    std::string wrap_mode;
    std::string scale_type_x;
    std::string scale_type_y;
    float scale_x;
    float scale_y;
    std::string format;
    std::vector<Parameter> parameters;
    std::vector<size_t> pragma_stage_lines;

    std::string vertex_shader_string;
    std::string fragment_shader_string;
    std::vector<uint32_t> vertex_shader_spirv;
    std::vector<uint32_t> fragment_shader_spirv;

    size_t push_constant_block_size;
    size_t ubo_size;
    int ubo_binding;
    std::vector<Uniform> uniforms;
    std::vector<Sampler> samplers;
};