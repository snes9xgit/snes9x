/* Based on code from Vulkan-Samples:
    https://github.com/KhronosGroup/Vulkan-Samples
*/

/* Copyright (c) 2018-2024, Arm Limited and Contributors
 * Copyright (c) 2019-2024, Sascha Willems
 *
 * SPDX-License-Identifier: Apache-2.0
 *
 * Licensed under the Apache License, Version 2.0 the "License";
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "vulkan_hpp_wrapper.hpp"

namespace Vulkan {

vk::AccessFlags get_access_flags(vk::ImageLayout layout)
{
    switch (layout) {
    case vk::ImageLayout::eUndefined:
    case vk::ImageLayout::ePresentSrcKHR:
        return vk::AccessFlagBits::eNone;
    case vk::ImageLayout::ePreinitialized:
        return vk::AccessFlagBits::eHostWrite;
    case vk::ImageLayout::eColorAttachmentOptimal:
        return vk::AccessFlagBits::eColorAttachmentRead | vk::AccessFlagBits::eColorAttachmentWrite;
    case vk::ImageLayout::eDepthAttachmentOptimal:
        return vk::AccessFlagBits::eDepthStencilAttachmentRead | vk::AccessFlagBits::eDepthStencilAttachmentWrite;
    case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
        return vk::AccessFlagBits::eFragmentShadingRateAttachmentReadKHR;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        return vk::AccessFlagBits::eShaderRead | vk::AccessFlagBits::eShaderWrite;
    case vk::ImageLayout::eTransferSrcOptimal:
        return vk::AccessFlagBits::eTransferRead;
    case vk::ImageLayout::eTransferDstOptimal:
        return vk::AccessFlagBits::eTransferWrite;
    case vk::ImageLayout::eGeneral:
        return vk::AccessFlagBits::eNone;
    default:
        return vk::AccessFlagBits::eNone;
    }
}

vk::PipelineStageFlags get_pipeline_stage_flags(vk::ImageLayout layout)
{
    switch (layout) {
    case vk::ImageLayout::eUndefined:
        return vk::PipelineStageFlagBits::eTopOfPipe;
    case vk::ImageLayout::ePreinitialized:
        return vk::PipelineStageFlagBits::eHost;
    case vk::ImageLayout::eTransferDstOptimal:
    case vk::ImageLayout::eTransferSrcOptimal:
        return vk::PipelineStageFlagBits::eTransfer;
    case vk::ImageLayout::eColorAttachmentOptimal:
        return vk::PipelineStageFlagBits::eColorAttachmentOutput;
    case vk::ImageLayout::eDepthAttachmentOptimal:
        return vk::PipelineStageFlagBits::eEarlyFragmentTests | vk::PipelineStageFlagBits::eLateFragmentTests;
    case vk::ImageLayout::eFragmentShadingRateAttachmentOptimalKHR:
        return vk::PipelineStageFlagBits::eFragmentShadingRateAttachmentKHR;
    case vk::ImageLayout::eShaderReadOnlyOptimal:
        return vk::PipelineStageFlagBits::eVertexShader | vk::PipelineStageFlagBits::eFragmentShader;
    case vk::ImageLayout::ePresentSrcKHR:
        return vk::PipelineStageFlagBits::eBottomOfPipe;
    case vk::ImageLayout::eGeneral:
        return vk::PipelineStageFlagBits::eNone;
    default:
        return vk::PipelineStageFlagBits::eNone;
    }
}

void image_layout_transition(vk::CommandBuffer command_buffer,
                             vk::Image image,
                             vk::PipelineStageFlags src_stage_mask,
                             vk::PipelineStageFlags dst_stage_mask,
                             vk::AccessFlags src_access_mask,
                             vk::AccessFlags dst_access_mask,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout,
                             vk::ImageSubresourceRange const &subresource_range)
{
    auto image_memory_barrier = vk::ImageMemoryBarrier{}
                                    .setSrcAccessMask(src_access_mask)
                                    .setDstAccessMask(dst_access_mask)
                                    .setOldLayout(old_layout)
                                    .setNewLayout(new_layout)
                                    .setImage(image)
                                    .setSubresourceRange(subresource_range);

    command_buffer.pipelineBarrier(src_stage_mask,
                                   dst_stage_mask,
                                   {}, {}, {}, image_memory_barrier);
}

void image_layout_transition(vk::CommandBuffer command_buffer,
                             vk::Image image,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout,
                             vk::ImageSubresourceRange const &subresource_range)
{
    vk::PipelineStageFlags src_stage_mask = get_pipeline_stage_flags(old_layout);
    vk::PipelineStageFlags dst_stage_mask = get_pipeline_stage_flags(new_layout);
    vk::AccessFlags src_access_mask = get_access_flags(old_layout);
    vk::AccessFlags dst_access_mask = get_access_flags(new_layout);

    image_layout_transition(command_buffer, image, src_stage_mask, dst_stage_mask, src_access_mask, dst_access_mask, old_layout, new_layout, subresource_range);
}

// Fixed sub resource on first mip level and layer
void image_layout_transition(vk::CommandBuffer command_buffer,
                             vk::Image image,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout)
{
    auto subresource_range = vk::ImageSubresourceRange{}
                                 .setAspectMask(vk::ImageAspectFlagBits::eColor)
                                 .setBaseMipLevel(0)
                                 .setLevelCount(1)
                                 .setBaseArrayLayer(0)
                                 .setLayerCount(1);

    image_layout_transition(command_buffer, image, old_layout, new_layout, subresource_range);
}

void image_layout_transition(vk::CommandBuffer command_buffer,
                             std::vector<std::pair<vk::Image, vk::ImageSubresourceRange>> const &imagesAndRanges,
                             vk::ImageLayout old_layout,
                             vk::ImageLayout new_layout)
{
    vk::PipelineStageFlags src_stage_mask = get_pipeline_stage_flags(old_layout);
    vk::PipelineStageFlags dst_stage_mask = get_pipeline_stage_flags(new_layout);
    vk::AccessFlags src_access_mask = get_access_flags(old_layout);
    vk::AccessFlags dst_access_mask = get_access_flags(new_layout);

    // Create image barrier objects
    std::vector<vk::ImageMemoryBarrier> image_memory_barriers;
    for (size_t i = 0; i < imagesAndRanges.size(); i++) {
        image_memory_barriers.push_back(vk::ImageMemoryBarrier(
            src_access_mask,
            dst_access_mask,
            old_layout,
            new_layout,
            VK_QUEUE_FAMILY_IGNORED,
            VK_QUEUE_FAMILY_IGNORED,
            imagesAndRanges[i].first,
            imagesAndRanges[i].second));
    }

    // Put barriers inside setup command buffer
    command_buffer.pipelineBarrier(src_stage_mask,
                                   dst_stage_mask,
                                   {}, {}, {}, image_memory_barriers);
}

} // namespace Vulkan