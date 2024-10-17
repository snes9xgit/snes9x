#pragma once

#include "vulkan_hpp_wrapper.hpp"

namespace Vulkan {

vk::AccessFlags get_access_flags(vk::ImageLayout layout);
vk::PipelineStageFlags get_pipeline_stage_flags(vk::ImageLayout layout);
void image_layout_transition(vk::CommandBuffer command_buffer,
                             vk::Image image,
                             vk::PipelineStageFlags src_stage_mask,
                             vk::PipelineStageFlags dst_stage_mask,
                             vk::AccessFlags src_access_mask,
                             vk::AccessFlags dst_access_mask,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout,
                             vk::ImageSubresourceRange const &subresource_range);
void image_layout_transition(vk::CommandBuffer command_buffer,
                             vk::Image image,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout,
                             vk::ImageSubresourceRange const &subresource_range);
void image_layout_transition(vk::CommandBuffer command_buffer,
                             vk::Image image,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout);

} // namespace Vulkan