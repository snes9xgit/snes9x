#include <cassert>

#include "vulkan_shader_chain.hpp"
#include "slang_helpers.hpp"
#include "stb_image.h"
#include "vulkan/vulkan_enums.hpp"

namespace Vulkan
{

ShaderChain::ShaderChain(Context *context_)
{
    context = context_;
    original_history_size = 3;
    original_width = 0;
    original_height = 0;
    viewport_width = 0;
    viewport_height = 0;
    vertex_buffer = nullptr;
    vertex_buffer_allocation = nullptr;
    last_frame_index = 2;
    current_frame_index = 0;
}

ShaderChain::~ShaderChain()
{
    if (context && context->device)
    {
        context->wait_idle();
        if (vertex_buffer)
            context->allocator.destroyBuffer(vertex_buffer, vertex_buffer_allocation);
        vertex_buffer = nullptr;
        vertex_buffer_allocation = nullptr;
    }
    pipelines.clear();
}

void ShaderChain::construct_buffer_objects()
{
    for (size_t i = 0; i < pipelines.size(); i++)
    {
        auto &pipeline = *pipelines[i];
        uint8_t *ubo_memory = nullptr;

        if (pipeline.shader->ubo_size > 0)
            ubo_memory = (uint8_t *)context->allocator.mapMemory(pipeline.uniform_buffer_allocation).value;

        for (auto &uniform : pipeline.shader->uniforms)
        {
            void *location = 0;
            const float MVP[16] = { 1.0f, 0.0f, 0.0f, 0.0f,
                                    0.0f, 1.0f, 0.0f, 0.0f,
                                    0.0f, 0.0f, 1.0f, 0.0f,
                                    0.0f, 0.0f, 0.0f, 1.0f };

            switch (uniform.block)
            {
                case SlangShader::Uniform::UBO:
                    location = &ubo_memory[uniform.offset];
                    break;
                case SlangShader::Uniform::PushConstant:
                    location = &pipeline.push_constants[uniform.offset];
                    break;
            }

            auto write_size = [&location](int width, int height) {
                std::array<float, 4> size;
                size[0] = (float)width;
                size[1] = (float)height;
                size[2] = 1.0f / size[0];
                size[3] = 1.0f / size[1];
                memcpy(location, size.data(), sizeof(float) * 4);
            };

            switch (uniform.type)
            {
                case SlangShader::Uniform::PassSize:
                case SlangShader::Uniform::PassFeedbackSize: // TODO: Does this need to differ?

                    if (uniform.specifier == -1)
                    {
                        write_size(original_width, original_height);
                    }
                    else
                    {
                        write_size(pipelines[uniform.specifier]->destination_width,
                                   pipelines[uniform.specifier]->destination_height);
                    }

                    break;

                case SlangShader::Uniform::ViewportSize:
                    write_size(viewport_width, viewport_height);
                    break;

                case SlangShader::Uniform::PreviousFrameSize:
                    if (original.size() > 1)
                        write_size(original[1]->image_width, original[1]->image_height);
                    else
                        write_size(original_width, original_height);
                    break;

                case SlangShader::Uniform::LutSize:
                    if (uniform.specifier < (int)lookup_textures.size())
                        write_size(lookup_textures[uniform.specifier]->image_width, lookup_textures[uniform.specifier]->image_height);
                    else
                        write_size(1, 1);
                    break;

                case SlangShader::Uniform::MVP:
                    memcpy(location, MVP, sizeof(float) * 16);
                    break;

                case SlangShader::Uniform::Parameter:
                    if (uniform.specifier < (int)preset->parameters.size())
                        memcpy(location, &preset->parameters[uniform.specifier].val, sizeof(float));
                    break;

                case SlangShader::Uniform::FrameCount:
                    memcpy(location, &frame_count, sizeof(uint32_t));
                    break;

                case SlangShader::Uniform::FrameDirection:
                    const int32_t frame_direction = 1;
                    memcpy(location, &frame_direction, sizeof(int32_t));
                    break;
            }
        }

        if (pipeline.shader->ubo_size > 0)
        {
            context->allocator.unmapMemory(pipeline.uniform_buffer_allocation);
            context->allocator.flushAllocation(pipeline.uniform_buffer_allocation, 0, pipeline.shader->ubo_size);
        }
    }
}

void ShaderChain::update_and_propagate_sizes(int original_width_new, int original_height_new, int viewport_width_new, int viewport_height_new)
{
    if (pipelines.empty())
        return;

    if (original_width == original_width_new &&
        original_height == original_height_new &&
        viewport_width == viewport_width_new &&
        viewport_height == viewport_height_new)
        return;

    original_width = original_width_new;
    original_height = original_height_new;
    viewport_width = viewport_width_new;
    viewport_height = viewport_height_new;

    for (size_t i = 0; i < pipelines.size(); i++)
    {
        auto &p = pipelines[i];
        if (i != 0)
        {
            p->source_width = pipelines[i - 1]->destination_width;
            p->source_height = pipelines[i - 1]->destination_height;
        }
        else
        {
            p->source_width = original_width_new;
            p->source_height = original_height_new;
        }

        if (p->shader->scale_type_x == "viewport")
            p->destination_width = viewport_width * p->shader->scale_x;
        else if (p->shader->scale_type_x == "absolute")
            p->destination_width = p->shader->scale_x;
        else
            p->destination_width = p->source_width * p->shader->scale_x;

        if (p->shader->scale_type_y == "viewport")
            p->destination_height = viewport_height * p->shader->scale_y;
        else if (p->shader->scale_type_y == "absolute")
            p->destination_height = p->shader->scale_y;
        else
            p->destination_height = p->source_height * p->shader->scale_y;

        if (i == pipelines.size() - 1)
        {
            p->destination_width = viewport_width;
            p->destination_height = viewport_height;
        }
    }
}

bool ShaderChain::load_shader_preset(std::string filename)
{
    if (!ends_with(filename, ".slangp"))
        printf("Warning: loading preset without .slangp extension\n");

    preset = std::make_unique<SlangPreset>();

    if (!preset->load_preset_file(filename))
    {
        printf("Couldn't load preset file: %s\n", filename.c_str());
        return false;
    }

    if (!preset->introspect())
    {
        printf("Failed introspection process in preset: %s\n", filename.c_str());
        return false;
    }

    pipelines.clear();
    pipelines.resize(preset->passes.size());

    int num_ubos = 0;
    int num_samplers = 0;

    for (size_t i = 0; i < preset->passes.size(); i++)
    {
        auto &p = preset->passes[i];
        pipelines[i] = std::make_unique<SlangPipeline>();
        pipelines[i]->init(context, &p);
        bool lastpass = (i == preset->passes.size() - 1);
        if (!pipelines[i]->generate_pipeline(lastpass))
        {
            printf("Couldn't create pipeline for shader: %s\n", p.filename.c_str());
            return false;
        }

        for (auto &u : p.samplers)
            if (u.type == SlangShader::Sampler::PreviousFrame)
                if (u.specifier > (int)original_history_size)
                    original_history_size = u.specifier;

        if (p.ubo_size)
            num_ubos++;
        if (p.samplers.size() > 0)
            num_samplers += p.samplers.size();
    }

    std::array<vk::DescriptorPoolSize, 2> descriptor_pool_sizes;
    descriptor_pool_sizes[0]
        .setType(vk::DescriptorType::eUniformBuffer)
        .setDescriptorCount(num_ubos * queue_size);
    descriptor_pool_sizes[1]
        .setType(vk::DescriptorType::eCombinedImageSampler)
        .setDescriptorCount(num_samplers * queue_size);

    auto descriptor_pool_create_info = vk::DescriptorPoolCreateInfo{}
        .setPoolSizes(descriptor_pool_sizes)
        .setMaxSets(pipelines.size() * queue_size)
        .setFlags(vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet);

    descriptor_pool = context->device.createDescriptorPoolUnique(descriptor_pool_create_info).value;

    for (auto &p : pipelines)
        p->generate_frame_resources(descriptor_pool.get());

    load_lookup_textures();

    float vertex_data[] = { -1.0f, -3.0f, 0.0f, 1.0f, /* texcoords */   0.0, -1.0f,
                             3.0f,  1.0f, 0.0f, 1.0f,                   2.0f, 1.0f,
                            -1.0f,  1.0f, 0.0f, 1.0f,                   0.0f, 1.0f };

    auto buffer_create_info = vk::BufferCreateInfo{}
        .setSize(sizeof(vertex_data))
        .setUsage(vk::BufferUsageFlagBits::eVertexBuffer);

    auto allocation_create_info = vma::AllocationCreateInfo{}
        .setFlags(vma::AllocationCreateFlagBits::eHostAccessSequentialWrite)
        .setRequiredFlags(vk::MemoryPropertyFlagBits::eHostVisible);

    std::tie(vertex_buffer, vertex_buffer_allocation) = context->allocator.createBuffer(buffer_create_info, allocation_create_info).value;

    auto vertex_buffer_memory = context->allocator.mapMemory(vertex_buffer_allocation).value;
    memcpy(vertex_buffer_memory, vertex_data, sizeof(vertex_data));
    context->allocator.unmapMemory(vertex_buffer_allocation);
    context->allocator.flushAllocation(vertex_buffer_allocation, 0, sizeof(vertex_data));

    frame_count = 0;
    current_frame_index = 0;
    last_frame_index = 2;

    context->wait_idle();

    return true;
}

void ShaderChain::update_framebuffers(vk::CommandBuffer cmd, int frame_num)
{
    size_t pass_count = pipelines.size() - 1;
    if (preset->last_pass_uses_feedback)
        pass_count++;

    for (size_t i = 0; i < pass_count; i++)
    {
        bool mipmap = false;
        if (i < pipelines.size() - 1)
            mipmap = pipelines[i + 1]->shader->mipmap_input;
        pipelines[i]->update_framebuffer(cmd, frame_num, mipmap);
    }
}

void ShaderChain::update_descriptor_set(vk::CommandBuffer cmd, int pipe_num, int swapchain_index)
{
    auto &pipe = *pipelines[pipe_num];
    auto &frame = pipe.frame[swapchain_index];

    if (pipe.shader->ubo_size > 0)
    {
        auto descriptor_buffer_info = vk::DescriptorBufferInfo{}
                .setBuffer(pipe.uniform_buffer)
                .setOffset(0)
                .setRange(pipe.shader->ubo_size);

        auto write_descriptor_set = vk::WriteDescriptorSet{}
            .setDescriptorType(vk::DescriptorType::eUniformBuffer)
            .setBufferInfo(descriptor_buffer_info)
            .setDstBinding(pipe.shader->ubo_binding)
            .setDstSet(frame.descriptor_set.get());

        context->device.updateDescriptorSets(write_descriptor_set, {});
    }

    auto descriptor_image_info = vk::DescriptorImageInfo{}
        .setImageLayout(vk::ImageLayout::eShaderReadOnlyOptimal);

    for (auto &sampler : pipe.shader->samplers)
    {
        if (sampler.type == SlangShader::Sampler::Lut)
        {
            descriptor_image_info
                .setImageView(lookup_textures[sampler.specifier]->image_view)
                .setSampler(lookup_textures[sampler.specifier]->sampler);
        }
        else if (sampler.type == SlangShader::Sampler::PassFeedback)
        {
            assert(sampler.specifier < (int)pipelines.size());
            assert(sampler.specifier >= 0);

            if (!pipelines[sampler.specifier]->frame[last_frame_index].image.image)
                update_framebuffers(cmd, last_frame_index);
            auto &feedback_frame = pipelines[sampler.specifier]->frame[last_frame_index];
            if (feedback_frame.image.current_layout == vk::ImageLayout::eUndefined)
                feedback_frame.image.clear(cmd);

            descriptor_image_info
                .setImageView(pipelines[sampler.specifier]->frame[last_frame_index].image.image_view);
            if (sampler.specifier == (int)pipelines.size() - 1)
                descriptor_image_info.setSampler(pipelines[sampler.specifier]->sampler.get());
            else
                descriptor_image_info.setSampler(pipelines[sampler.specifier + 1]->sampler.get());;

        }
        else if (sampler.type == SlangShader::Sampler::Pass)
        {
            assert(sampler.specifier + 1 < (int)pipelines.size());
            auto &sampler_to_use = pipelines[sampler.specifier + 1]->sampler.get();

            if (sampler.specifier == -1)
            {
                descriptor_image_info
                    .setSampler(sampler_to_use)
                    .setImageView(original[0]->image_view);
            }
            else
            {
                descriptor_image_info
                    .setSampler(sampler_to_use)
                    .setImageView(pipelines[sampler.specifier]->frame[swapchain_index].image.image_view);
            }
        }
        else if (sampler.type == SlangShader::Sampler::PreviousFrame)
        {
            int which_original = sampler.specifier;
            if (which_original >= (int)original.size())
                which_original = original.size() - 1;

            assert(which_original > -1);

            descriptor_image_info
                .setSampler(pipelines[0]->sampler.get())
                .setImageView(original[which_original]->image_view);
        }

        auto write_descriptor_set = vk::WriteDescriptorSet{}
            .setDescriptorType(vk::DescriptorType::eCombinedImageSampler)
            .setDstSet(frame.descriptor_set.get())
            .setDstBinding(sampler.binding)
            .setImageInfo(descriptor_image_info);
        context->device.updateDescriptorSets(write_descriptor_set, {});
    }
}

bool ShaderChain::do_frame(uint8_t *data, int width, int height, int stride, vk::Format format, int viewport_x, int viewport_y, int viewport_width, int viewport_height)
{
    if (!do_frame_without_swap(data, width, height, stride, format, viewport_x, viewport_y, viewport_width, viewport_height))
        return false;
    context->swapchain->swap();
    return true;
}

bool ShaderChain::do_frame_without_swap(uint8_t *data, int width, int height, int stride, vk::Format format, int viewport_x, int viewport_y, int viewport_width, int viewport_height)
{
    if (!context->swapchain->begin_frame())
         return false;

    auto cmd = context->swapchain->get_cmd();

    update_and_propagate_sizes(width, height, viewport_width, viewport_height);

    update_framebuffers(cmd, current_frame_index);

    upload_original(cmd, data, width, height, stride, format);

    construct_buffer_objects();

    for (size_t i = 0; i < pipelines.size(); i++)
    {
        auto &pipe = *pipelines[i];
        auto &frame = pipe.frame[current_frame_index];
        bool is_last_pass = (i == pipelines.size() - 1);

        update_descriptor_set(cmd, i, current_frame_index);

        vk::ClearValue value{};
        value.color = { 0.0f, 0.0f, 0.0f, 1.0f };

        auto render_pass_begin_info = vk::RenderPassBeginInfo{}
            .setRenderPass(pipe.render_pass.get())
            .setFramebuffer(frame.image.framebuffer.get())
            .setRenderArea(vk::Rect2D({}, vk::Extent2D(frame.image.image_width, frame.image.image_height)))
            .setClearValues(value);

        if (is_last_pass)
            context->swapchain->begin_render_pass();
        else
            cmd.beginRenderPass(render_pass_begin_info, vk::SubpassContents::eInline);

        cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, pipe.pipeline.get());
        cmd.bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipe.pipeline_layout.get(), 0, frame.descriptor_set.get(), {});
        cmd.bindVertexBuffers(0, vertex_buffer, { 0 });
        if (pipe.push_constants.size() > 0)
            cmd.pushConstants(pipe.pipeline_layout.get(), vk::ShaderStageFlagBits::eAllGraphics, 0, pipe.push_constants.size(), pipe.push_constants.data());

        if (is_last_pass)
        {
            cmd.setViewport(0, vk::Viewport(viewport_x, viewport_y, viewport_width, viewport_height, 0.0f, 1.0f));
            cmd.setScissor(0, vk::Rect2D(vk::Offset2D(viewport_x, viewport_y), vk::Extent2D(viewport_width, viewport_height)));
        }
        else
        {
            cmd.setViewport(0, vk::Viewport(0, 0, pipe.destination_width, pipe.destination_height, 0.0f, 1.0f));
            cmd.setScissor(0, vk::Rect2D({}, vk::Extent2D(pipe.destination_width, pipe.destination_height)));
        }

        cmd.draw(3, 1, 0, 0);

        if (is_last_pass)
            context->swapchain->end_render_pass();
        else
            cmd.endRenderPass();

        frame.image.barrier(cmd);
        if (!is_last_pass)
            frame.image.generate_mipmaps(cmd);

        if (preset->last_pass_uses_feedback && is_last_pass)
        {
            std::array<vk::ImageMemoryBarrier, 2> image_memory_barrier{};
            image_memory_barrier[0]
                .setImage(frame.image.image)
                .setOldLayout(vk::ImageLayout::eUndefined)
                .setNewLayout(vk::ImageLayout::eTransferDstOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            image_memory_barrier[1]
                .setImage(context->swapchain->get_image())
                .setOldLayout(vk::ImageLayout::ePresentSrcKHR)
                .setNewLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eColorAttachmentWrite)
                .setDstAccessMask(vk::AccessFlagBits::eTransferRead)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eColorAttachmentOutput,
                                vk::PipelineStageFlagBits::eTransfer,
                                {}, {}, {}, image_memory_barrier);

            auto image_blit = vk::ImageBlit{}
                .setSrcOffsets({ vk::Offset3D(viewport_x, viewport_y, 0), vk::Offset3D(viewport_x + viewport_width, viewport_y + viewport_height, 1) })
                .setDstOffsets({ vk::Offset3D(0, 0, 0), vk::Offset3D(viewport_width, viewport_height, 1) })
                .setSrcSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1))
                .setDstSubresource(vk::ImageSubresourceLayers(vk::ImageAspectFlagBits::eColor, 0, 0, 1));

            cmd.blitImage(context->swapchain->get_image(), vk::ImageLayout::eTransferSrcOptimal, frame.image.image, vk::ImageLayout::eTransferDstOptimal, image_blit, vk::Filter::eNearest);

            image_memory_barrier[0]
                .setOldLayout(vk::ImageLayout::eTransferDstOptimal)
                .setNewLayout(vk::ImageLayout::eShaderReadOnlyOptimal)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits::eShaderRead)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));
            image_memory_barrier[1]
                .setOldLayout(vk::ImageLayout::eTransferSrcOptimal)
                .setNewLayout(vk::ImageLayout::ePresentSrcKHR)
                .setSrcAccessMask(vk::AccessFlagBits::eTransferWrite)
                .setDstAccessMask(vk::AccessFlagBits::eMemoryRead)
                .setSubresourceRange(vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

            cmd.pipelineBarrier(vk::PipelineStageFlagBits::eTransfer,
                                vk::PipelineStageFlagBits::eAllGraphics,
                                {}, {}, {}, image_memory_barrier);

            frame.image.current_layout = vk::ImageLayout::eTransferDstOptimal;
        }
    }
    context->swapchain->end_frame_without_swap();

    last_frame_index = current_frame_index;
    current_frame_index = (current_frame_index + 1) % queue_size;
    frame_count++;
    return true;
}

void ShaderChain::upload_original(vk::CommandBuffer cmd, uint8_t *data, int width, int height, int stride, vk::Format format)
{
    std::unique_ptr<Texture> texture;
    bool create_texture = false;

    if (original.size() > original_history_size)
    {
        texture = std::move(original.back());
        original.pop_back();

        if (texture->image_width != width || texture->image_height != height || texture->format != format)
        {
            texture->destroy();
            create_texture = true;
        }
    }
    else
    {
        texture = std::make_unique<Texture>();
        texture->init(context);
        create_texture = true;
    }

    if (create_texture)
        texture->create(width,
                        height,
                        format,
                        wrap_mode_from_string(pipelines[0]->shader->wrap_mode),
                        pipelines[0]->shader->filter_linear,
                        pipelines[0]->shader->mipmap_input);

    if (cmd)
        texture->from_buffer(cmd, data, width, height, stride);
    else
        texture->from_buffer(data, width, height, stride);

    original.push_front(std::move(texture));
}

void ShaderChain::upload_original(uint8_t *data, int width, int height, int stride, vk::Format format)
{
    upload_original(nullptr, data, width, height, stride, format);
}

bool ShaderChain::load_lookup_textures()
{
    if (preset->textures.size() < 1)
        return true;

    lookup_textures.clear();

    for (auto &l : preset->textures)
    {
        int width, height, channels;
        stbi_uc *bytes = stbi_load(l.filename.c_str(), &width, &height, &channels, 4);

        if (!bytes)
        {
            printf("Couldn't load look-up texture: %s\n", l.filename.c_str());
            return false;
        }

        auto wrap_mode = wrap_mode_from_string(l.wrap_mode);

        lookup_textures.push_back(std::make_unique<Texture>());
        auto &t = lookup_textures.back();
        t->init(context);
        t->create(width, height, vk::Format::eR8G8B8A8Unorm, wrap_mode, l.linear, l.mipmap);
        t->from_buffer(bytes, width, height);
        t->discard_staging_buffer();
        free(bytes);
    }

    return true;
}

} // namespace Vulkan