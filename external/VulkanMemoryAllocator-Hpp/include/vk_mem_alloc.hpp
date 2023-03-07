#ifndef VULKAN_MEMORY_ALLOCATOR_HPP
#define VULKAN_MEMORY_ALLOCATOR_HPP

#if !defined(AMD_VULKAN_MEMORY_ALLOCATOR_H)
#include <vk_mem_alloc.h>
#endif

#include <vulkan/vulkan.hpp>

#if !defined(VMA_HPP_NAMESPACE)
#define VMA_HPP_NAMESPACE vma
#endif

#define VMA_HPP_NAMESPACE_STRING VULKAN_HPP_STRINGIFY(VMA_HPP_NAMESPACE)

#ifndef VULKAN_HPP_NO_SMART_HANDLE
namespace VMA_HPP_NAMESPACE {
  struct Dispatcher {}; // VMA uses function pointers from VmaAllocator instead
  class Allocator;

  template<class T>
  VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher> createUniqueHandle(const T& t) VULKAN_HPP_NOEXCEPT {
    return VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>(t);
  }
  template<class T, class O>
  VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher> createUniqueHandle(const T& t, const O* o) VULKAN_HPP_NOEXCEPT {
    return VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>(t, o);
  }
  template<class F, class S, class O>
  std::pair<VULKAN_HPP_NAMESPACE::UniqueHandle<F, Dispatcher>, VULKAN_HPP_NAMESPACE::UniqueHandle<S, Dispatcher>>
          createUniqueHandle(const std::pair<F, S>& t, const O* o) VULKAN_HPP_NOEXCEPT {
    return {
            VULKAN_HPP_NAMESPACE::UniqueHandle<F, Dispatcher>(t.first, o),
            VULKAN_HPP_NAMESPACE::UniqueHandle<S, Dispatcher>(t.second, o)
    };
  }

  template<class T, class UniqueVectorAllocator, class VectorAllocator, class O>
  std::vector<VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>, UniqueVectorAllocator>
        createUniqueHandleVector(const std::vector<T, VectorAllocator>& vector, const O* o,
                                 const UniqueVectorAllocator& vectorAllocator) VULKAN_HPP_NOEXCEPT {
    std::vector<VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>, UniqueVectorAllocator> result(vectorAllocator);
    result.reserve(vector.size());
    for (const T& t : vector) result.emplace_back(t, o);
    return result;
  }

  template<class T, class Owner> class Deleter {
    const Owner* owner;
  public:
    Deleter() = default;
    Deleter(const Owner* owner) VULKAN_HPP_NOEXCEPT : owner(owner) {}
  protected:
    void destroy(const T& t) VULKAN_HPP_NOEXCEPT; // Implemented manually for each handle type
  };
  template<class T> class Deleter<T, void> {
  protected:
    void destroy(const T& t) VULKAN_HPP_NOEXCEPT { t.destroy(); }
  };
}
namespace VULKAN_HPP_NAMESPACE {
  template<> struct UniqueHandleTraits<Buffer, VMA_HPP_NAMESPACE::Dispatcher> {
    using deleter = VMA_HPP_NAMESPACE::Deleter<Buffer, VMA_HPP_NAMESPACE::Allocator>;
  };
  template<> struct UniqueHandleTraits<Image, VMA_HPP_NAMESPACE::Dispatcher> {
    using deleter = VMA_HPP_NAMESPACE::Deleter<Image, VMA_HPP_NAMESPACE::Allocator>;
  };
}
namespace VMA_HPP_NAMESPACE {
  using UniqueBuffer = VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Buffer, Dispatcher>;
  using UniqueImage = VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Image, Dispatcher>;
}
#endif

#include "vk_mem_alloc_enums.hpp"
#include "vk_mem_alloc_handles.hpp"
#include "vk_mem_alloc_structs.hpp"
#include "vk_mem_alloc_funcs.hpp"

namespace VMA_HPP_NAMESPACE {
  
#ifndef VULKAN_HPP_NO_SMART_HANDLE
# define VMA_HPP_DESTROY_IMPL(NAME) \
  template<> VULKAN_HPP_INLINE void VULKAN_HPP_NAMESPACE::UniqueHandleTraits<NAME, Dispatcher>::deleter::destroy(const NAME& t) VULKAN_HPP_NOEXCEPT

  VMA_HPP_DESTROY_IMPL(VULKAN_HPP_NAMESPACE::Buffer) { owner->destroyBuffer(t, nullptr); }
  VMA_HPP_DESTROY_IMPL(VULKAN_HPP_NAMESPACE::Image) { owner->destroyImage(t, nullptr); }
  VMA_HPP_DESTROY_IMPL(Pool) { owner->destroyPool(t); }
  VMA_HPP_DESTROY_IMPL(Allocation) { owner->freeMemory(t); }
  VMA_HPP_DESTROY_IMPL(VirtualAllocation) { owner->virtualFree(t); }

# undef VMA_HPP_DESTROY_IMPL
#endif

  template<class InstanceDispatcher, class DeviceDispatcher>
  VULKAN_HPP_CONSTEXPR VulkanFunctions functionsFromDispatcher(InstanceDispatcher const * instance,
                                                               DeviceDispatcher const * device) VULKAN_HPP_NOEXCEPT {
    return VulkanFunctions {
            instance->vkGetInstanceProcAddr,
            instance->vkGetDeviceProcAddr,
            instance->vkGetPhysicalDeviceProperties,
            instance->vkGetPhysicalDeviceMemoryProperties,
            device->vkAllocateMemory,
            device->vkFreeMemory,
            device->vkMapMemory,
            device->vkUnmapMemory,
            device->vkFlushMappedMemoryRanges,
            device->vkInvalidateMappedMemoryRanges,
            device->vkBindBufferMemory,
            device->vkBindImageMemory,
            device->vkGetBufferMemoryRequirements,
            device->vkGetImageMemoryRequirements,
            device->vkCreateBuffer,
            device->vkDestroyBuffer,
            device->vkCreateImage,
            device->vkDestroyImage,
            device->vkCmdCopyBuffer,
            device->vkGetBufferMemoryRequirements2KHR ? device->vkGetBufferMemoryRequirements2KHR : device->vkGetBufferMemoryRequirements2,
            device->vkGetImageMemoryRequirements2KHR ? device->vkGetImageMemoryRequirements2KHR : device->vkGetImageMemoryRequirements2,
            device->vkBindBufferMemory2KHR ? device->vkBindBufferMemory2KHR : device->vkBindBufferMemory2,
            device->vkBindImageMemory2KHR ? device->vkBindImageMemory2KHR : device->vkBindImageMemory2,
            instance->vkGetPhysicalDeviceMemoryProperties2KHR ? instance->vkGetPhysicalDeviceMemoryProperties2KHR : instance->vkGetPhysicalDeviceMemoryProperties2,
            device->vkGetDeviceBufferMemoryRequirements,
            device->vkGetDeviceImageMemoryRequirements
    };
  }

  template<class Dispatch = VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>
  VULKAN_HPP_CONSTEXPR VulkanFunctions functionsFromDispatcher(Dispatch const & dispatch
    VULKAN_HPP_DEFAULT_DISPATCHER_ASSIGNMENT) VULKAN_HPP_NOEXCEPT {
    return functionsFromDispatcher(&dispatch, &dispatch);
  }
}

#endif
