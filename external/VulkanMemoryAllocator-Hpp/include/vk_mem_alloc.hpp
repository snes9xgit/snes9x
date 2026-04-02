// Macros
#ifndef VMA_HPP_NAMESPACE
#define VMA_HPP_NAMESPACE vma
#endif
#ifndef VMA_HPP_RAII_NAMESPACE
#define VMA_HPP_RAII_NAMESPACE raii
#endif
#include <vulkan/vulkan_hpp_macros.hpp>

// VMA header
#ifndef AMD_VULKAN_MEMORY_ALLOCATOR_H
#include "vk_mem_alloc.h"
#endif

// The body
#ifndef VULKAN_MEMORY_ALLOCATOR_HPP
#define VULKAN_MEMORY_ALLOCATOR_HPP

#ifdef VMA_HPP_CXX_MODULE
import vulkan;
#else
#include <vulkan/vulkan.hpp>
#endif

#define VMA_HPP_NAMESPACE_STRING VULKAN_HPP_STRINGIFY(VMA_HPP_NAMESPACE)
#define VMA_HPP_RAII_NAMESPACE_STRING VULKAN_HPP_STRINGIFY(VMA_HPP_NAMESPACE::VMA_HPP_RAII_NAMESPACE)

// Workaround for breaking changes in Vulkan-Hpp:
// https://github.com/KhronosGroup/Vulkan-Hpp/commit/a0a4c22975c89f70202542cc52d0f182db296002
// Just add dummy operators, so that using declarations would always compile.
namespace VMA_HPP_NAMESPACE { class Dummy; }
namespace VULKAN_HPP_NAMESPACE {
  void operator<(VMA_HPP_NAMESPACE::Dummy, VMA_HPP_NAMESPACE::Dummy);
  void operator<=(VMA_HPP_NAMESPACE::Dummy, VMA_HPP_NAMESPACE::Dummy);
  void operator>(VMA_HPP_NAMESPACE::Dummy, VMA_HPP_NAMESPACE::Dummy);
  void operator>=(VMA_HPP_NAMESPACE::Dummy, VMA_HPP_NAMESPACE::Dummy);
#ifdef VULKAN_HPP_HAS_SPACESHIP_OPERATOR
  void operator<=>(VMA_HPP_NAMESPACE::Dummy, VMA_HPP_NAMESPACE::Dummy);
#endif
}

namespace VMA_HPP_NAMESPACE {
  using VULKAN_HPP_NAMESPACE::operator&;
  using VULKAN_HPP_NAMESPACE::operator|;
  using VULKAN_HPP_NAMESPACE::operator^;
  using VULKAN_HPP_NAMESPACE::operator~;
  using VULKAN_HPP_NAMESPACE::operator<;
  using VULKAN_HPP_NAMESPACE::operator<=;
  using VULKAN_HPP_NAMESPACE::operator>;
  using VULKAN_HPP_NAMESPACE::operator>=;
  using VULKAN_HPP_NAMESPACE::operator==;
  using VULKAN_HPP_NAMESPACE::operator!=;
#ifdef VULKAN_HPP_HAS_SPACESHIP_OPERATOR
  using VULKAN_HPP_NAMESPACE::operator<=>;
#endif
}

#include "vk_mem_alloc_enums.hpp"

#ifndef VULKAN_HPP_NO_TO_STRING
#include "vk_mem_alloc_to_string.hpp"
#endif

#include "vk_mem_alloc_handles.hpp"

#include "vk_mem_alloc_structs.hpp"

#include "vk_mem_alloc_funcs.hpp"

#endif
