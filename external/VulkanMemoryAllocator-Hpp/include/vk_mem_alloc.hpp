#ifndef VULKAN_MEMORY_ALLOCATOR_HPP
#define VULKAN_MEMORY_ALLOCATOR_HPP

#if !defined(AMD_VULKAN_MEMORY_ALLOCATOR_H)
#include <vk_mem_alloc.h>
#endif

#include <vulkan/vulkan_hpp_macros.hpp>

#ifdef VMA_ENABLE_STD_MODULE
import VULKAN_HPP_STD_MODULE;
#else
#include <algorithm>
#include <array>
#include <string>
#include <vector>
#endif

#ifdef VMA_ENABLE_VULKAN_HPP_MODULE
import vulkan_hpp;
#else
#if !defined(VULKAN_HPP)
#include <vulkan/vulkan.hpp>
#endif
#endif

#if !defined(VMA_HPP_NAMESPACE)
#define VMA_HPP_NAMESPACE vma
#endif

#define VMA_HPP_NAMESPACE_STRING VULKAN_HPP_STRINGIFY(VMA_HPP_NAMESPACE)

#ifndef VULKAN_HPP_NO_SMART_HANDLE
namespace VMA_HPP_NAMESPACE {
struct Dispatcher {}; // VMA uses function pointers from VmaAllocator instead
class Allocator;

template <class T>
VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>
createUniqueHandle(const T &t) VULKAN_HPP_NOEXCEPT {
  return VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>(t);
}
template <class T, class O>
VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>
createUniqueHandle(const T &t, O o) VULKAN_HPP_NOEXCEPT {
  return VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>(t, o);
}
template <class F, class S, class O>
std::pair<VULKAN_HPP_NAMESPACE::UniqueHandle<F, Dispatcher>,
          VULKAN_HPP_NAMESPACE::UniqueHandle<S, Dispatcher>>
createUniqueHandle(const std::pair<F, S> &t, O o) VULKAN_HPP_NOEXCEPT {
  return {VULKAN_HPP_NAMESPACE::UniqueHandle<F, Dispatcher>(t.first, o),
          VULKAN_HPP_NAMESPACE::UniqueHandle<S, Dispatcher>(t.second, o)};
}

template <class T, class UniqueVectorAllocator, class VectorAllocator, class O>
std::vector<VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>,
            UniqueVectorAllocator>
createUniqueHandleVector(const std::vector<T, VectorAllocator> &vector, O o,
                         const UniqueVectorAllocator &vectorAllocator)
    VULKAN_HPP_NOEXCEPT {
  std::vector<VULKAN_HPP_NAMESPACE::UniqueHandle<T, Dispatcher>,
              UniqueVectorAllocator>
      result(vectorAllocator);
  result.reserve(vector.size());
  for (const T &t : vector)
    result.emplace_back(t, o);
  return result;
}

template <class T, class Owner> class Deleter {
  Owner owner;

public:
  Deleter() = default;
  Deleter(Owner owner) VULKAN_HPP_NOEXCEPT : owner(owner) {}

protected:
  void destroy(const T &t)
      VULKAN_HPP_NOEXCEPT; // Implemented manually for each handle type
};
template <class T> class Deleter<T, void> {
protected:
  void destroy(const T &t) VULKAN_HPP_NOEXCEPT { t.destroy(); }
};
} // namespace VMA_HPP_NAMESPACE
namespace VULKAN_HPP_NAMESPACE {
template <> class UniqueHandleTraits<Buffer, VMA_HPP_NAMESPACE::Dispatcher> {
public:
  using deleter =
      VMA_HPP_NAMESPACE::Deleter<Buffer, VMA_HPP_NAMESPACE::Allocator>;
};
template <> class UniqueHandleTraits<Image, VMA_HPP_NAMESPACE::Dispatcher> {
public:
  using deleter =
      VMA_HPP_NAMESPACE::Deleter<Image, VMA_HPP_NAMESPACE::Allocator>;
};
} // namespace VULKAN_HPP_NAMESPACE
namespace VMA_HPP_NAMESPACE {
using UniqueBuffer =
    VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Buffer,
                                       Dispatcher>;
using UniqueImage =
    VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Image, Dispatcher>;
} // namespace VMA_HPP_NAMESPACE
#endif

#ifndef VULKAN_HPP_NO_EXCEPTIONS
namespace VMA_HPP_NAMESPACE::detail {
[[noreturn]] VULKAN_HPP_INLINE void
throwResultException(VULKAN_HPP_NAMESPACE::Result result, char const *message) {
  switch (result) {
    using VULKAN_HPP_NAMESPACE::Result;
  case Result::eErrorOutOfHostMemory:
    throw VULKAN_HPP_NAMESPACE::OutOfHostMemoryError(message);
  case Result::eErrorOutOfDeviceMemory:
    throw VULKAN_HPP_NAMESPACE::OutOfDeviceMemoryError(message);
  case Result::eErrorInitializationFailed:
    throw VULKAN_HPP_NAMESPACE::InitializationFailedError(message);
  case Result::eErrorDeviceLost:
    throw VULKAN_HPP_NAMESPACE::DeviceLostError(message);
  case Result::eErrorMemoryMapFailed:
    throw VULKAN_HPP_NAMESPACE::MemoryMapFailedError(message);
  case Result::eErrorLayerNotPresent:
    throw VULKAN_HPP_NAMESPACE::LayerNotPresentError(message);
  case Result::eErrorExtensionNotPresent:
    throw VULKAN_HPP_NAMESPACE::ExtensionNotPresentError(message);
  case Result::eErrorFeatureNotPresent:
    throw VULKAN_HPP_NAMESPACE::FeatureNotPresentError(message);
  case Result::eErrorIncompatibleDriver:
    throw VULKAN_HPP_NAMESPACE::IncompatibleDriverError(message);
  case Result::eErrorTooManyObjects:
    throw VULKAN_HPP_NAMESPACE::TooManyObjectsError(message);
  case Result::eErrorFormatNotSupported:
    throw VULKAN_HPP_NAMESPACE::FormatNotSupportedError(message);
  case Result::eErrorFragmentedPool:
    throw VULKAN_HPP_NAMESPACE::FragmentedPoolError(message);
  case Result::eErrorUnknown:
    throw VULKAN_HPP_NAMESPACE::UnknownError(message);
  case Result::eErrorOutOfPoolMemory:
    throw VULKAN_HPP_NAMESPACE::OutOfPoolMemoryError(message);
  case Result::eErrorInvalidExternalHandle:
    throw VULKAN_HPP_NAMESPACE::InvalidExternalHandleError(message);
  case Result::eErrorFragmentation:
    throw VULKAN_HPP_NAMESPACE::FragmentationError(message);
  case Result::eErrorInvalidOpaqueCaptureAddress:
    throw VULKAN_HPP_NAMESPACE::InvalidOpaqueCaptureAddressError(message);
  case Result::eErrorSurfaceLostKHR:
    throw VULKAN_HPP_NAMESPACE::SurfaceLostKHRError(message);
  case Result::eErrorNativeWindowInUseKHR:
    throw VULKAN_HPP_NAMESPACE::NativeWindowInUseKHRError(message);
  case Result::eErrorOutOfDateKHR:
    throw VULKAN_HPP_NAMESPACE::OutOfDateKHRError(message);
  case Result::eErrorIncompatibleDisplayKHR:
    throw VULKAN_HPP_NAMESPACE::IncompatibleDisplayKHRError(message);
  case Result::eErrorValidationFailedEXT:
    throw VULKAN_HPP_NAMESPACE::ValidationFailedEXTError(message);
  case Result::eErrorInvalidShaderNV:
    throw VULKAN_HPP_NAMESPACE::InvalidShaderNVError(message);
  case Result::eErrorImageUsageNotSupportedKHR:
    throw VULKAN_HPP_NAMESPACE::ImageUsageNotSupportedKHRError(message);
  case Result::eErrorVideoPictureLayoutNotSupportedKHR:
    throw VULKAN_HPP_NAMESPACE::VideoPictureLayoutNotSupportedKHRError(message);
  case Result::eErrorVideoProfileOperationNotSupportedKHR:
    throw VULKAN_HPP_NAMESPACE::VideoProfileOperationNotSupportedKHRError(
        message);
  case Result::eErrorVideoProfileFormatNotSupportedKHR:
    throw VULKAN_HPP_NAMESPACE::VideoProfileFormatNotSupportedKHRError(message);
  case Result::eErrorVideoProfileCodecNotSupportedKHR:
    throw VULKAN_HPP_NAMESPACE::VideoProfileCodecNotSupportedKHRError(message);
  case Result::eErrorVideoStdVersionNotSupportedKHR:
    throw VULKAN_HPP_NAMESPACE::VideoStdVersionNotSupportedKHRError(message);
  case Result::eErrorInvalidDrmFormatModifierPlaneLayoutEXT:
    throw VULKAN_HPP_NAMESPACE::InvalidDrmFormatModifierPlaneLayoutEXTError(
        message);
  case Result::eErrorNotPermitted:
    throw VULKAN_HPP_NAMESPACE::NotPermittedError(message);
#if defined(VK_USE_PLATFORM_WIN32_KHR)
  case Result::eErrorFullScreenExclusiveModeLostEXT:
    throw VULKAN_HPP_NAMESPACE::FullScreenExclusiveModeLostEXTError(message);
#endif /*VK_USE_PLATFORM_WIN32_KHR*/
  case Result::eErrorInvalidVideoStdParametersKHR:
    throw VULKAN_HPP_NAMESPACE::InvalidVideoStdParametersKHRError(message);
  case Result::eErrorCompressionExhaustedEXT:
    throw VULKAN_HPP_NAMESPACE::CompressionExhaustedEXTError(message);
  default:
    throw VULKAN_HPP_NAMESPACE::SystemError(
        VULKAN_HPP_NAMESPACE::make_error_code(result), message);
  }
}
} // namespace VMA_HPP_NAMESPACE::detail
#endif // VULKAN_HPP_NO_EXCEPTIONS

namespace VMA_HPP_NAMESPACE::detail {
template <typename T> void ignore(T const &) VULKAN_HPP_NOEXCEPT {}

VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type
createResultValueType(VULKAN_HPP_NAMESPACE::Result result) {
#ifdef VULKAN_HPP_NO_EXCEPTIONS
  return result;
#else
  VMA_HPP_NAMESPACE::detail::ignore(result);
#endif
}

template <typename T>
VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<T>::type
createResultValueType(VULKAN_HPP_NAMESPACE::Result result, T &data) {
#ifdef VULKAN_HPP_NO_EXCEPTIONS
  return VULKAN_HPP_NAMESPACE::ResultValue<T>(result, data);
#else
  VMA_HPP_NAMESPACE::detail::ignore(result);
  return data;
#endif
}

template <typename T>
VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<T>::type
createResultValueType(VULKAN_HPP_NAMESPACE::Result result, T &&data) {
#ifdef VULKAN_HPP_NO_EXCEPTIONS
  return VULKAN_HPP_NAMESPACE::ResultValue<T>(result, std::move(data));
#else
  VMA_HPP_NAMESPACE::detail::ignore(result);
  return std::move(data);
#endif
}
} // namespace VMA_HPP_NAMESPACE::detail

namespace VMA_HPP_NAMESPACE::detail {
VULKAN_HPP_INLINE void 
resultCheck(VULKAN_HPP_NAMESPACE::Result result, char const *message) {
#ifdef VULKAN_HPP_NO_EXCEPTIONS
  VMA_HPP_NAMESPACE::detail::ignore(
      result); // just in case VULKAN_HPP_ASSERT_ON_RESULT is empty
  VMA_HPP_NAMESPACE::detail::ignore(message);
  VULKAN_HPP_ASSERT_ON_RESULT(result == VULKAN_HPP_NAMESPACE::Result::eSuccess);
#else
  if (result != VULKAN_HPP_NAMESPACE::Result::eSuccess) {
    VMA_HPP_NAMESPACE::detail::throwResultException(result, message);
  }
#endif
}

VULKAN_HPP_INLINE void
resultCheck(VULKAN_HPP_NAMESPACE::Result result, char const *message,
            std::initializer_list<VULKAN_HPP_NAMESPACE::Result> successCodes) {
#ifdef VULKAN_HPP_NO_EXCEPTIONS
  VMA_HPP_NAMESPACE::detail::ignore(
      result); // just in case VULKAN_HPP_ASSERT_ON_RESULT is empty
  VMA_HPP_NAMESPACE::detail::ignore(message);
  VMA_HPP_NAMESPACE::detail::ignore(
      successCodes); // just in case VULKAN_HPP_ASSERT_ON_RESULT is empty
  VULKAN_HPP_ASSERT_ON_RESULT(std::find(successCodes.begin(),
                                        successCodes.end(),
                                        result) != successCodes.end());
#else
  if (std::find(successCodes.begin(), successCodes.end(), result) ==
      successCodes.end()) {
    VMA_HPP_NAMESPACE::detail::throwResultException(result, message);
  }
#endif
}
} // namespace VMA_HPP_NAMESPACE::detail

#include "vk_mem_alloc_enums.hpp"

#include "vk_mem_alloc_handles.hpp"

#include "vk_mem_alloc_structs.hpp"

#include "vk_mem_alloc_funcs.hpp"

namespace VMA_HPP_NAMESPACE {

#ifndef VULKAN_HPP_NO_SMART_HANDLE
#define VMA_HPP_DESTROY_IMPL(NAME)                                             \
  template <>                                                                  \
  VULKAN_HPP_INLINE void VULKAN_HPP_NAMESPACE::UniqueHandleTraits<             \
      NAME, Dispatcher>::deleter::destroy(const NAME &t) VULKAN_HPP_NOEXCEPT

VMA_HPP_DESTROY_IMPL(VULKAN_HPP_NAMESPACE::Buffer) {
  owner.destroyBuffer(t, nullptr);
}
VMA_HPP_DESTROY_IMPL(VULKAN_HPP_NAMESPACE::Image) {
  owner.destroyImage(t, nullptr);
}
VMA_HPP_DESTROY_IMPL(Pool) { owner.destroyPool(t); }
VMA_HPP_DESTROY_IMPL(Allocation) { owner.freeMemory(t); }
VMA_HPP_DESTROY_IMPL(VirtualAllocation) { owner.virtualFree(t); }

#undef VMA_HPP_DESTROY_IMPL
#endif

template <class InstanceDispatcher, class DeviceDispatcher>
VULKAN_HPP_CONSTEXPR VulkanFunctions
functionsFromDispatcher(InstanceDispatcher const *instance,
                        DeviceDispatcher const *device) VULKAN_HPP_NOEXCEPT {
  return VulkanFunctions{
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
      device->vkGetBufferMemoryRequirements2KHR
          ? device->vkGetBufferMemoryRequirements2KHR
          : device->vkGetBufferMemoryRequirements2,
      device->vkGetImageMemoryRequirements2KHR
          ? device->vkGetImageMemoryRequirements2KHR
          : device->vkGetImageMemoryRequirements2,
      device->vkBindBufferMemory2KHR ? device->vkBindBufferMemory2KHR
                                     : device->vkBindBufferMemory2,
      device->vkBindImageMemory2KHR ? device->vkBindImageMemory2KHR
                                    : device->vkBindImageMemory2,
      instance->vkGetPhysicalDeviceMemoryProperties2KHR
          ? instance->vkGetPhysicalDeviceMemoryProperties2KHR
          : instance->vkGetPhysicalDeviceMemoryProperties2,
      device->vkGetDeviceBufferMemoryRequirementsKHR
          ? device->vkGetDeviceBufferMemoryRequirementsKHR
          : device->vkGetDeviceBufferMemoryRequirements,
      device->vkGetDeviceImageMemoryRequirementsKHR
          ? device->vkGetDeviceImageMemoryRequirementsKHR
          : device->vkGetDeviceImageMemoryRequirements,
  };
}

template <class Dispatch = VULKAN_HPP_DEFAULT_DISPATCHER_TYPE>
VULKAN_HPP_CONSTEXPR VulkanFunctions
functionsFromDispatcher(Dispatch const &dispatch) VULKAN_HPP_NOEXCEPT {
  return functionsFromDispatcher(&dispatch, &dispatch);
}
} // namespace VMA_HPP_NAMESPACE

#endif
