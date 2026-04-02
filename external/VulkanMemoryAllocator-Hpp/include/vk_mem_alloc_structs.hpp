// Generated from the Vulkan Memory Allocator (vk_mem_alloc.h).
#ifndef VULKAN_MEMORY_ALLOCATOR_STRUCTS_HPP
#define VULKAN_MEMORY_ALLOCATOR_STRUCTS_HPP

namespace VMA_HPP_NAMESPACE {

  // wrapper struct for struct VmaDeviceMemoryCallbacks, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_device_memory_callbacks.html
  struct DeviceMemoryCallbacks {
    using NativeType = VmaDeviceMemoryCallbacks;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DeviceMemoryCallbacks(
        PFN_vmaAllocateDeviceMemoryFunction pfnAllocate_ = {}
      , PFN_vmaFreeDeviceMemoryFunction pfnFree_ = {}
      , void* pUserData_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : pfnAllocate { pfnAllocate_ }
      , pfnFree { pfnFree_ }
      , pUserData { pUserData_ } {}

    VULKAN_HPP_CONSTEXPR DeviceMemoryCallbacks(DeviceMemoryCallbacks const &) VULKAN_HPP_NOEXCEPT = default;
    DeviceMemoryCallbacks(VmaDeviceMemoryCallbacks const & rhs) VULKAN_HPP_NOEXCEPT : DeviceMemoryCallbacks(*reinterpret_cast<DeviceMemoryCallbacks const *>(&rhs)) {}

    DeviceMemoryCallbacks& operator=(DeviceMemoryCallbacks const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    DeviceMemoryCallbacks& operator=(VmaDeviceMemoryCallbacks const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DeviceMemoryCallbacks const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 DeviceMemoryCallbacks& setPfnAllocate(PFN_vmaAllocateDeviceMemoryFunction pfnAllocate_) VULKAN_HPP_NOEXCEPT {
      pfnAllocate = pfnAllocate_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DeviceMemoryCallbacks& setPfnFree(PFN_vmaFreeDeviceMemoryFunction pfnFree_) VULKAN_HPP_NOEXCEPT {
      pfnFree = pfnFree_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DeviceMemoryCallbacks& setPUserData(void* pUserData_) VULKAN_HPP_NOEXCEPT {
      pUserData = pUserData_;
      return *this;
    }
#endif

    operator VmaDeviceMemoryCallbacks const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDeviceMemoryCallbacks *>(this);
    }

    operator VmaDeviceMemoryCallbacks&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDeviceMemoryCallbacks *>(this);
    }

    operator VmaDeviceMemoryCallbacks const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaDeviceMemoryCallbacks *>(this);
    }

    operator VmaDeviceMemoryCallbacks*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaDeviceMemoryCallbacks *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<PFN_vmaAllocateDeviceMemoryFunction const &
             , PFN_vmaFreeDeviceMemoryFunction const &
             , void* const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(pfnAllocate
                    , pfnFree
                    , pUserData);
    }
#endif

  bool operator==(DeviceMemoryCallbacks const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return pfnAllocate == rhs.pfnAllocate
        && pfnFree == rhs.pfnFree
        && pUserData == rhs.pUserData;
#endif
  }
  bool operator!=(DeviceMemoryCallbacks const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }

  public:
    PFN_vmaAllocateDeviceMemoryFunction pfnAllocate = {};
    PFN_vmaFreeDeviceMemoryFunction pfnFree = {};
    void* pUserData = {};
  };

  // wrapper struct for struct VmaVulkanFunctions, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_vulkan_functions.html
  struct VulkanFunctions {
    using NativeType = VmaVulkanFunctions;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR VulkanFunctions(
        PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr_ = {}
      , PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr_ = {}
      , PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties_ = {}
      , PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties_ = {}
      , PFN_vkAllocateMemory vkAllocateMemory_ = {}
      , PFN_vkFreeMemory vkFreeMemory_ = {}
      , PFN_vkMapMemory vkMapMemory_ = {}
      , PFN_vkUnmapMemory vkUnmapMemory_ = {}
      , PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges_ = {}
      , PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges_ = {}
      , PFN_vkBindBufferMemory vkBindBufferMemory_ = {}
      , PFN_vkBindImageMemory vkBindImageMemory_ = {}
      , PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements_ = {}
      , PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements_ = {}
      , PFN_vkCreateBuffer vkCreateBuffer_ = {}
      , PFN_vkDestroyBuffer vkDestroyBuffer_ = {}
      , PFN_vkCreateImage vkCreateImage_ = {}
      , PFN_vkDestroyImage vkDestroyImage_ = {}
      , PFN_vkCmdCopyBuffer vkCmdCopyBuffer_ = {}
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
      , PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR_ = {}
      , PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR_ = {}
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
      , PFN_vkBindBufferMemory2KHR vkBindBufferMemory2KHR_ = {}
      , PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR_ = {}
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
      , PFN_vkGetPhysicalDeviceMemoryProperties2KHR vkGetPhysicalDeviceMemoryProperties2KHR_ = {}
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
      , PFN_vkGetDeviceBufferMemoryRequirementsKHR vkGetDeviceBufferMemoryRequirements_ = {}
      , PFN_vkGetDeviceImageMemoryRequirementsKHR vkGetDeviceImageMemoryRequirements_ = {}
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
      , PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR_ = {}
#else
      , void* vkGetMemoryWin32HandleKHR_ = {}
#endif
    ) VULKAN_HPP_NOEXCEPT
      : vkGetInstanceProcAddr { vkGetInstanceProcAddr_ }
      , vkGetDeviceProcAddr { vkGetDeviceProcAddr_ }
      , vkGetPhysicalDeviceProperties { vkGetPhysicalDeviceProperties_ }
      , vkGetPhysicalDeviceMemoryProperties { vkGetPhysicalDeviceMemoryProperties_ }
      , vkAllocateMemory { vkAllocateMemory_ }
      , vkFreeMemory { vkFreeMemory_ }
      , vkMapMemory { vkMapMemory_ }
      , vkUnmapMemory { vkUnmapMemory_ }
      , vkFlushMappedMemoryRanges { vkFlushMappedMemoryRanges_ }
      , vkInvalidateMappedMemoryRanges { vkInvalidateMappedMemoryRanges_ }
      , vkBindBufferMemory { vkBindBufferMemory_ }
      , vkBindImageMemory { vkBindImageMemory_ }
      , vkGetBufferMemoryRequirements { vkGetBufferMemoryRequirements_ }
      , vkGetImageMemoryRequirements { vkGetImageMemoryRequirements_ }
      , vkCreateBuffer { vkCreateBuffer_ }
      , vkDestroyBuffer { vkDestroyBuffer_ }
      , vkCreateImage { vkCreateImage_ }
      , vkDestroyImage { vkDestroyImage_ }
      , vkCmdCopyBuffer { vkCmdCopyBuffer_ }
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
      , vkGetBufferMemoryRequirements2KHR { vkGetBufferMemoryRequirements2KHR_ }
      , vkGetImageMemoryRequirements2KHR { vkGetImageMemoryRequirements2KHR_ }
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
      , vkBindBufferMemory2KHR { vkBindBufferMemory2KHR_ }
      , vkBindImageMemory2KHR { vkBindImageMemory2KHR_ }
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
      , vkGetPhysicalDeviceMemoryProperties2KHR { vkGetPhysicalDeviceMemoryProperties2KHR_ }
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
      , vkGetDeviceBufferMemoryRequirements { vkGetDeviceBufferMemoryRequirements_ }
      , vkGetDeviceImageMemoryRequirements { vkGetDeviceImageMemoryRequirements_ }
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
      , vkGetMemoryWin32HandleKHR { vkGetMemoryWin32HandleKHR_ }
#else
      , vkGetMemoryWin32HandleKHR { vkGetMemoryWin32HandleKHR_ }
#endif
         {}

    VULKAN_HPP_CONSTEXPR VulkanFunctions(VulkanFunctions const &) VULKAN_HPP_NOEXCEPT = default;
    VulkanFunctions(VmaVulkanFunctions const & rhs) VULKAN_HPP_NOEXCEPT : VulkanFunctions(*reinterpret_cast<VulkanFunctions const *>(&rhs)) {}

    VulkanFunctions& operator=(VulkanFunctions const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    VulkanFunctions& operator=(VmaVulkanFunctions const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::VulkanFunctions const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetInstanceProcAddr(PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr_) VULKAN_HPP_NOEXCEPT {
      vkGetInstanceProcAddr = vkGetInstanceProcAddr_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetDeviceProcAddr(PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr_) VULKAN_HPP_NOEXCEPT {
      vkGetDeviceProcAddr = vkGetDeviceProcAddr_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetPhysicalDeviceProperties(PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties_) VULKAN_HPP_NOEXCEPT {
      vkGetPhysicalDeviceProperties = vkGetPhysicalDeviceProperties_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetPhysicalDeviceMemoryProperties(PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties_) VULKAN_HPP_NOEXCEPT {
      vkGetPhysicalDeviceMemoryProperties = vkGetPhysicalDeviceMemoryProperties_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkAllocateMemory(PFN_vkAllocateMemory vkAllocateMemory_) VULKAN_HPP_NOEXCEPT {
      vkAllocateMemory = vkAllocateMemory_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkFreeMemory(PFN_vkFreeMemory vkFreeMemory_) VULKAN_HPP_NOEXCEPT {
      vkFreeMemory = vkFreeMemory_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkMapMemory(PFN_vkMapMemory vkMapMemory_) VULKAN_HPP_NOEXCEPT {
      vkMapMemory = vkMapMemory_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkUnmapMemory(PFN_vkUnmapMemory vkUnmapMemory_) VULKAN_HPP_NOEXCEPT {
      vkUnmapMemory = vkUnmapMemory_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkFlushMappedMemoryRanges(PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges_) VULKAN_HPP_NOEXCEPT {
      vkFlushMappedMemoryRanges = vkFlushMappedMemoryRanges_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkInvalidateMappedMemoryRanges(PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges_) VULKAN_HPP_NOEXCEPT {
      vkInvalidateMappedMemoryRanges = vkInvalidateMappedMemoryRanges_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkBindBufferMemory(PFN_vkBindBufferMemory vkBindBufferMemory_) VULKAN_HPP_NOEXCEPT {
      vkBindBufferMemory = vkBindBufferMemory_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkBindImageMemory(PFN_vkBindImageMemory vkBindImageMemory_) VULKAN_HPP_NOEXCEPT {
      vkBindImageMemory = vkBindImageMemory_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetBufferMemoryRequirements(PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements_) VULKAN_HPP_NOEXCEPT {
      vkGetBufferMemoryRequirements = vkGetBufferMemoryRequirements_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetImageMemoryRequirements(PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements_) VULKAN_HPP_NOEXCEPT {
      vkGetImageMemoryRequirements = vkGetImageMemoryRequirements_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkCreateBuffer(PFN_vkCreateBuffer vkCreateBuffer_) VULKAN_HPP_NOEXCEPT {
      vkCreateBuffer = vkCreateBuffer_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkDestroyBuffer(PFN_vkDestroyBuffer vkDestroyBuffer_) VULKAN_HPP_NOEXCEPT {
      vkDestroyBuffer = vkDestroyBuffer_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkCreateImage(PFN_vkCreateImage vkCreateImage_) VULKAN_HPP_NOEXCEPT {
      vkCreateImage = vkCreateImage_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkDestroyImage(PFN_vkDestroyImage vkDestroyImage_) VULKAN_HPP_NOEXCEPT {
      vkDestroyImage = vkDestroyImage_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkCmdCopyBuffer(PFN_vkCmdCopyBuffer vkCmdCopyBuffer_) VULKAN_HPP_NOEXCEPT {
      vkCmdCopyBuffer = vkCmdCopyBuffer_;
      return *this;
    }

#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetBufferMemoryRequirements2KHR(PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR_) VULKAN_HPP_NOEXCEPT {
      vkGetBufferMemoryRequirements2KHR = vkGetBufferMemoryRequirements2KHR_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetImageMemoryRequirements2KHR(PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR_) VULKAN_HPP_NOEXCEPT {
      vkGetImageMemoryRequirements2KHR = vkGetImageMemoryRequirements2KHR_;
      return *this;
    }

#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkBindBufferMemory2KHR(PFN_vkBindBufferMemory2KHR vkBindBufferMemory2KHR_) VULKAN_HPP_NOEXCEPT {
      vkBindBufferMemory2KHR = vkBindBufferMemory2KHR_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkBindImageMemory2KHR(PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR_) VULKAN_HPP_NOEXCEPT {
      vkBindImageMemory2KHR = vkBindImageMemory2KHR_;
      return *this;
    }

#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetPhysicalDeviceMemoryProperties2KHR(PFN_vkGetPhysicalDeviceMemoryProperties2KHR vkGetPhysicalDeviceMemoryProperties2KHR_) VULKAN_HPP_NOEXCEPT {
      vkGetPhysicalDeviceMemoryProperties2KHR = vkGetPhysicalDeviceMemoryProperties2KHR_;
      return *this;
    }

#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetDeviceBufferMemoryRequirements(PFN_vkGetDeviceBufferMemoryRequirementsKHR vkGetDeviceBufferMemoryRequirements_) VULKAN_HPP_NOEXCEPT {
      vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetDeviceImageMemoryRequirements(PFN_vkGetDeviceImageMemoryRequirementsKHR vkGetDeviceImageMemoryRequirements_) VULKAN_HPP_NOEXCEPT {
      vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements_;
      return *this;
    }

#endif
#if VMA_EXTERNAL_MEMORY_WIN32
    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetMemoryWin32HandleKHR(PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR_) VULKAN_HPP_NOEXCEPT {
      vkGetMemoryWin32HandleKHR = vkGetMemoryWin32HandleKHR_;
      return *this;
    }

#else
    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetMemoryWin32HandleKHR(void* vkGetMemoryWin32HandleKHR_) VULKAN_HPP_NOEXCEPT {
      vkGetMemoryWin32HandleKHR = vkGetMemoryWin32HandleKHR_;
      return *this;
    }
#endif
#endif

    operator VmaVulkanFunctions const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaVulkanFunctions *>(this);
    }

    operator VmaVulkanFunctions&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaVulkanFunctions *>(this);
    }

    operator VmaVulkanFunctions const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaVulkanFunctions *>(this);
    }

    operator VmaVulkanFunctions*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaVulkanFunctions *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<PFN_vkGetInstanceProcAddr const &
             , PFN_vkGetDeviceProcAddr const &
             , PFN_vkGetPhysicalDeviceProperties const &
             , PFN_vkGetPhysicalDeviceMemoryProperties const &
             , PFN_vkAllocateMemory const &
             , PFN_vkFreeMemory const &
             , PFN_vkMapMemory const &
             , PFN_vkUnmapMemory const &
             , PFN_vkFlushMappedMemoryRanges const &
             , PFN_vkInvalidateMappedMemoryRanges const &
             , PFN_vkBindBufferMemory const &
             , PFN_vkBindImageMemory const &
             , PFN_vkGetBufferMemoryRequirements const &
             , PFN_vkGetImageMemoryRequirements const &
             , PFN_vkCreateBuffer const &
             , PFN_vkDestroyBuffer const &
             , PFN_vkCreateImage const &
             , PFN_vkDestroyImage const &
             , PFN_vkCmdCopyBuffer const &
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
             , PFN_vkGetBufferMemoryRequirements2KHR const &
             , PFN_vkGetImageMemoryRequirements2KHR const &
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
             , PFN_vkBindBufferMemory2KHR const &
             , PFN_vkBindImageMemory2KHR const &
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
             , PFN_vkGetPhysicalDeviceMemoryProperties2KHR const &
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
             , PFN_vkGetDeviceBufferMemoryRequirementsKHR const &
             , PFN_vkGetDeviceImageMemoryRequirementsKHR const &
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
             , PFN_vkGetMemoryWin32HandleKHR const &
#else
             , void* const &
#endif
               >
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(vkGetInstanceProcAddr
                    , vkGetDeviceProcAddr
                    , vkGetPhysicalDeviceProperties
                    , vkGetPhysicalDeviceMemoryProperties
                    , vkAllocateMemory
                    , vkFreeMemory
                    , vkMapMemory
                    , vkUnmapMemory
                    , vkFlushMappedMemoryRanges
                    , vkInvalidateMappedMemoryRanges
                    , vkBindBufferMemory
                    , vkBindImageMemory
                    , vkGetBufferMemoryRequirements
                    , vkGetImageMemoryRequirements
                    , vkCreateBuffer
                    , vkDestroyBuffer
                    , vkCreateImage
                    , vkDestroyImage
                    , vkCmdCopyBuffer
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
                    , vkGetBufferMemoryRequirements2KHR
                    , vkGetImageMemoryRequirements2KHR
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
                    , vkBindBufferMemory2KHR
                    , vkBindImageMemory2KHR
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
                    , vkGetPhysicalDeviceMemoryProperties2KHR
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
                    , vkGetDeviceBufferMemoryRequirements
                    , vkGetDeviceImageMemoryRequirements
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
                    , vkGetMemoryWin32HandleKHR
#else
                    , vkGetMemoryWin32HandleKHR
#endif
                      );
    }
#endif

  bool operator==(VulkanFunctions const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return vkGetInstanceProcAddr == rhs.vkGetInstanceProcAddr
        && vkGetDeviceProcAddr == rhs.vkGetDeviceProcAddr
        && vkGetPhysicalDeviceProperties == rhs.vkGetPhysicalDeviceProperties
        && vkGetPhysicalDeviceMemoryProperties == rhs.vkGetPhysicalDeviceMemoryProperties
        && vkAllocateMemory == rhs.vkAllocateMemory
        && vkFreeMemory == rhs.vkFreeMemory
        && vkMapMemory == rhs.vkMapMemory
        && vkUnmapMemory == rhs.vkUnmapMemory
        && vkFlushMappedMemoryRanges == rhs.vkFlushMappedMemoryRanges
        && vkInvalidateMappedMemoryRanges == rhs.vkInvalidateMappedMemoryRanges
        && vkBindBufferMemory == rhs.vkBindBufferMemory
        && vkBindImageMemory == rhs.vkBindImageMemory
        && vkGetBufferMemoryRequirements == rhs.vkGetBufferMemoryRequirements
        && vkGetImageMemoryRequirements == rhs.vkGetImageMemoryRequirements
        && vkCreateBuffer == rhs.vkCreateBuffer
        && vkDestroyBuffer == rhs.vkDestroyBuffer
        && vkCreateImage == rhs.vkCreateImage
        && vkDestroyImage == rhs.vkDestroyImage
        && vkCmdCopyBuffer == rhs.vkCmdCopyBuffer
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
        && vkGetBufferMemoryRequirements2KHR == rhs.vkGetBufferMemoryRequirements2KHR
        && vkGetImageMemoryRequirements2KHR == rhs.vkGetImageMemoryRequirements2KHR
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
        && vkBindBufferMemory2KHR == rhs.vkBindBufferMemory2KHR
        && vkBindImageMemory2KHR == rhs.vkBindImageMemory2KHR
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
        && vkGetPhysicalDeviceMemoryProperties2KHR == rhs.vkGetPhysicalDeviceMemoryProperties2KHR
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
        && vkGetDeviceBufferMemoryRequirements == rhs.vkGetDeviceBufferMemoryRequirements
        && vkGetDeviceImageMemoryRequirements == rhs.vkGetDeviceImageMemoryRequirements
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
        && vkGetMemoryWin32HandleKHR == rhs.vkGetMemoryWin32HandleKHR
#else
        && vkGetMemoryWin32HandleKHR == rhs.vkGetMemoryWin32HandleKHR
#endif
           ;
#endif
  }
  bool operator!=(VulkanFunctions const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }

  public:
    PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr = {};
    PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr = {};
    PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties = {};
    PFN_vkGetPhysicalDeviceMemoryProperties vkGetPhysicalDeviceMemoryProperties = {};
    PFN_vkAllocateMemory vkAllocateMemory = {};
    PFN_vkFreeMemory vkFreeMemory = {};
    PFN_vkMapMemory vkMapMemory = {};
    PFN_vkUnmapMemory vkUnmapMemory = {};
    PFN_vkFlushMappedMemoryRanges vkFlushMappedMemoryRanges = {};
    PFN_vkInvalidateMappedMemoryRanges vkInvalidateMappedMemoryRanges = {};
    PFN_vkBindBufferMemory vkBindBufferMemory = {};
    PFN_vkBindImageMemory vkBindImageMemory = {};
    PFN_vkGetBufferMemoryRequirements vkGetBufferMemoryRequirements = {};
    PFN_vkGetImageMemoryRequirements vkGetImageMemoryRequirements = {};
    PFN_vkCreateBuffer vkCreateBuffer = {};
    PFN_vkDestroyBuffer vkDestroyBuffer = {};
    PFN_vkCreateImage vkCreateImage = {};
    PFN_vkDestroyImage vkDestroyImage = {};
    PFN_vkCmdCopyBuffer vkCmdCopyBuffer = {};
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
    PFN_vkGetBufferMemoryRequirements2KHR vkGetBufferMemoryRequirements2KHR = {};
    PFN_vkGetImageMemoryRequirements2KHR vkGetImageMemoryRequirements2KHR = {};
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
    PFN_vkBindBufferMemory2KHR vkBindBufferMemory2KHR = {};
    PFN_vkBindImageMemory2KHR vkBindImageMemory2KHR = {};
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
    PFN_vkGetPhysicalDeviceMemoryProperties2KHR vkGetPhysicalDeviceMemoryProperties2KHR = {};
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
    PFN_vkGetDeviceBufferMemoryRequirementsKHR vkGetDeviceBufferMemoryRequirements = {};
    PFN_vkGetDeviceImageMemoryRequirementsKHR vkGetDeviceImageMemoryRequirements = {};
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
    PFN_vkGetMemoryWin32HandleKHR vkGetMemoryWin32HandleKHR = {};
#else
    void* vkGetMemoryWin32HandleKHR = {};
#endif
  };

  namespace detail {
    struct VulkanPFN {
      struct vkGetInstanceProcAddr { using type = PFN_vkGetInstanceProcAddr; };
      struct vkGetDeviceProcAddr { using type = PFN_vkGetDeviceProcAddr; };
      struct vkGetPhysicalDeviceProperties { using type = PFN_vkGetPhysicalDeviceProperties; };
      struct vkGetPhysicalDeviceMemoryProperties { using type = PFN_vkGetPhysicalDeviceMemoryProperties; };
      struct vkAllocateMemory { using type = PFN_vkAllocateMemory; };
      struct vkFreeMemory { using type = PFN_vkFreeMemory; };
      struct vkMapMemory { using type = PFN_vkMapMemory; };
      struct vkUnmapMemory { using type = PFN_vkUnmapMemory; };
      struct vkFlushMappedMemoryRanges { using type = PFN_vkFlushMappedMemoryRanges; };
      struct vkInvalidateMappedMemoryRanges { using type = PFN_vkInvalidateMappedMemoryRanges; };
      struct vkBindBufferMemory { using type = PFN_vkBindBufferMemory; };
      struct vkBindImageMemory { using type = PFN_vkBindImageMemory; };
      struct vkGetBufferMemoryRequirements { using type = PFN_vkGetBufferMemoryRequirements; };
      struct vkGetImageMemoryRequirements { using type = PFN_vkGetImageMemoryRequirements; };
      struct vkCreateBuffer { using type = PFN_vkCreateBuffer; };
      struct vkDestroyBuffer { using type = PFN_vkDestroyBuffer; };
      struct vkCreateImage { using type = PFN_vkCreateImage; };
      struct vkDestroyImage { using type = PFN_vkDestroyImage; };
      struct vkCmdCopyBuffer { using type = PFN_vkCmdCopyBuffer; };
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
      struct vkGetBufferMemoryRequirements2 { using type = PFN_vkGetBufferMemoryRequirements2; };
      struct vkGetImageMemoryRequirements2 { using type = PFN_vkGetImageMemoryRequirements2; };
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
      struct vkBindBufferMemory2 { using type = PFN_vkBindBufferMemory2; };
      struct vkBindImageMemory2 { using type = PFN_vkBindImageMemory2; };
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
      struct vkGetPhysicalDeviceMemoryProperties2 { using type = PFN_vkGetPhysicalDeviceMemoryProperties2; };
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
      struct vkGetDeviceBufferMemoryRequirements { using type = PFN_vkGetDeviceBufferMemoryRequirements; };
      struct vkGetDeviceImageMemoryRequirements { using type = PFN_vkGetDeviceImageMemoryRequirements; };
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
      struct vkGetMemoryWin32HandleKHR { using type = PFN_vkGetMemoryWin32HandleKHR; };
#else
#endif
      template<class Fun, class PFN, class T, class... Ts> struct FromDispatchers {
        static PFN get(const T&, const Ts&... ts) VULKAN_HPP_NOEXCEPT {
          return FromDispatchers<Fun, PFN, Ts...>::get(ts...);
        }
      };
      template<class Fun, class... Ts> static typename Fun::type get(const Ts&... ts) VULKAN_HPP_NOEXCEPT {
        return FromDispatchers<Fun, typename Fun::type, Ts...>::get(ts...);
      }
    };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetInstanceProcAddr, decltype(T::vkGetInstanceProcAddr), T, Ts...>
    { static PFN_vkGetInstanceProcAddr get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetInstanceProcAddr; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetDeviceProcAddr, decltype(T::vkGetDeviceProcAddr), T, Ts...>
    { static PFN_vkGetDeviceProcAddr get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetDeviceProcAddr; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetPhysicalDeviceProperties, decltype(T::vkGetPhysicalDeviceProperties), T, Ts...>
    { static PFN_vkGetPhysicalDeviceProperties get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetPhysicalDeviceProperties; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetPhysicalDeviceMemoryProperties, decltype(T::vkGetPhysicalDeviceMemoryProperties), T, Ts...>
    { static PFN_vkGetPhysicalDeviceMemoryProperties get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetPhysicalDeviceMemoryProperties; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkAllocateMemory, decltype(T::vkAllocateMemory), T, Ts...>
    { static PFN_vkAllocateMemory get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkAllocateMemory; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkFreeMemory, decltype(T::vkFreeMemory), T, Ts...>
    { static PFN_vkFreeMemory get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkFreeMemory; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkMapMemory, decltype(T::vkMapMemory), T, Ts...>
    { static PFN_vkMapMemory get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkMapMemory; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkUnmapMemory, decltype(T::vkUnmapMemory), T, Ts...>
    { static PFN_vkUnmapMemory get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkUnmapMemory; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkFlushMappedMemoryRanges, decltype(T::vkFlushMappedMemoryRanges), T, Ts...>
    { static PFN_vkFlushMappedMemoryRanges get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkFlushMappedMemoryRanges; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkInvalidateMappedMemoryRanges, decltype(T::vkInvalidateMappedMemoryRanges), T, Ts...>
    { static PFN_vkInvalidateMappedMemoryRanges get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkInvalidateMappedMemoryRanges; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkBindBufferMemory, decltype(T::vkBindBufferMemory), T, Ts...>
    { static PFN_vkBindBufferMemory get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkBindBufferMemory; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkBindImageMemory, decltype(T::vkBindImageMemory), T, Ts...>
    { static PFN_vkBindImageMemory get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkBindImageMemory; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetBufferMemoryRequirements, decltype(T::vkGetBufferMemoryRequirements), T, Ts...>
    { static PFN_vkGetBufferMemoryRequirements get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetBufferMemoryRequirements; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetImageMemoryRequirements, decltype(T::vkGetImageMemoryRequirements), T, Ts...>
    { static PFN_vkGetImageMemoryRequirements get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetImageMemoryRequirements; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkCreateBuffer, decltype(T::vkCreateBuffer), T, Ts...>
    { static PFN_vkCreateBuffer get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkCreateBuffer; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkDestroyBuffer, decltype(T::vkDestroyBuffer), T, Ts...>
    { static PFN_vkDestroyBuffer get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkDestroyBuffer; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkCreateImage, decltype(T::vkCreateImage), T, Ts...>
    { static PFN_vkCreateImage get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkCreateImage; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkDestroyImage, decltype(T::vkDestroyImage), T, Ts...>
    { static PFN_vkDestroyImage get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkDestroyImage; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkCmdCopyBuffer, decltype(T::vkCmdCopyBuffer), T, Ts...>
    { static PFN_vkCmdCopyBuffer get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkCmdCopyBuffer; } };
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetBufferMemoryRequirements2, decltype(T::vkGetBufferMemoryRequirements2), T, Ts...>
    { static PFN_vkGetBufferMemoryRequirements2 get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetBufferMemoryRequirements2; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetImageMemoryRequirements2, decltype(T::vkGetImageMemoryRequirements2), T, Ts...>
    { static PFN_vkGetImageMemoryRequirements2 get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetImageMemoryRequirements2; } };
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkBindBufferMemory2, decltype(T::vkBindBufferMemory2), T, Ts...>
    { static PFN_vkBindBufferMemory2 get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkBindBufferMemory2; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkBindImageMemory2, decltype(T::vkBindImageMemory2), T, Ts...>
    { static PFN_vkBindImageMemory2 get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkBindImageMemory2; } };
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetPhysicalDeviceMemoryProperties2, decltype(T::vkGetPhysicalDeviceMemoryProperties2), T, Ts...>
    { static PFN_vkGetPhysicalDeviceMemoryProperties2 get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetPhysicalDeviceMemoryProperties2; } };
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetDeviceBufferMemoryRequirements, decltype(T::vkGetDeviceBufferMemoryRequirements), T, Ts...>
    { static PFN_vkGetDeviceBufferMemoryRequirements get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetDeviceBufferMemoryRequirements; } };
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetDeviceImageMemoryRequirements, decltype(T::vkGetDeviceImageMemoryRequirements), T, Ts...>
    { static PFN_vkGetDeviceImageMemoryRequirements get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetDeviceImageMemoryRequirements; } };
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
    template<class T, class... Ts> struct VulkanPFN::FromDispatchers<VulkanPFN::vkGetMemoryWin32HandleKHR, decltype(T::vkGetMemoryWin32HandleKHR), T, Ts...>
    { static PFN_vkGetMemoryWin32HandleKHR get(const T& t, const Ts&...) VULKAN_HPP_NOEXCEPT { return t.vkGetMemoryWin32HandleKHR; } };
#else
#endif
  }

  // utility function for retrieving function table from vk dispatchers
  template <class... Dispatch> VulkanFunctions
  functionsFromDispatchers(Dispatch const &... dispatch) VULKAN_HPP_NOEXCEPT {
    return VulkanFunctions {
        detail::VulkanPFN::get<detail::VulkanPFN::vkGetInstanceProcAddr>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetDeviceProcAddr>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetPhysicalDeviceProperties>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetPhysicalDeviceMemoryProperties>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkAllocateMemory>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkFreeMemory>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkMapMemory>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkUnmapMemory>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkFlushMappedMemoryRanges>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkInvalidateMappedMemoryRanges>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkBindBufferMemory>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkBindImageMemory>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetBufferMemoryRequirements>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetImageMemoryRequirements>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkCreateBuffer>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkDestroyBuffer>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkCreateImage>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkDestroyImage>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkCmdCopyBuffer>(dispatch...)
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetBufferMemoryRequirements2>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetImageMemoryRequirements2>(dispatch...)
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
      , detail::VulkanPFN::get<detail::VulkanPFN::vkBindBufferMemory2>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkBindImageMemory2>(dispatch...)
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetPhysicalDeviceMemoryProperties2>(dispatch...)
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetDeviceBufferMemoryRequirements>(dispatch...)
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetDeviceImageMemoryRequirements>(dispatch...)
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
      , detail::VulkanPFN::get<detail::VulkanPFN::vkGetMemoryWin32HandleKHR>(dispatch...)
#else
      , nullptr
#endif
    };
  }
  template <class Dispatch = VULKAN_HPP_DEFAULT_DISPATCHER_TYPE> VulkanFunctions
  functionsFromDispatcher(Dispatch const & dispatch VULKAN_HPP_DEFAULT_DISPATCHER_ASSIGNMENT) VULKAN_HPP_NOEXCEPT {
    return functionsFromDispatchers(dispatch);
  }
#if !defined( VK_NO_PROTOTYPES )
  template <> VULKAN_HPP_CONSTEXPR_INLINE VulkanFunctions
  functionsFromDispatcher<VULKAN_HPP_DISPATCH_LOADER_STATIC_TYPE>(VULKAN_HPP_DISPATCH_LOADER_STATIC_TYPE const &) VULKAN_HPP_NOEXCEPT {
    return VulkanFunctions {
        &vkGetInstanceProcAddr
      , &vkGetDeviceProcAddr
      , &vkGetPhysicalDeviceProperties
      , &vkGetPhysicalDeviceMemoryProperties
      , &vkAllocateMemory
      , &vkFreeMemory
      , &vkMapMemory
      , &vkUnmapMemory
      , &vkFlushMappedMemoryRanges
      , &vkInvalidateMappedMemoryRanges
      , &vkBindBufferMemory
      , &vkBindImageMemory
      , &vkGetBufferMemoryRequirements
      , &vkGetImageMemoryRequirements
      , &vkCreateBuffer
      , &vkDestroyBuffer
      , &vkCreateImage
      , &vkDestroyImage
      , &vkCmdCopyBuffer
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
      , &vkGetBufferMemoryRequirements2
      , &vkGetImageMemoryRequirements2
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
      , &vkBindBufferMemory2
      , &vkBindImageMemory2
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
      , &vkGetPhysicalDeviceMemoryProperties2
#endif
#if VMA_KHR_MAINTENANCE4 || VMA_VULKAN_VERSION >= 1003000
      , &vkGetDeviceBufferMemoryRequirements
      , &vkGetDeviceImageMemoryRequirements
#endif
#if VMA_EXTERNAL_MEMORY_WIN32
      , &vkGetMemoryWin32HandleKHR
#else
      , nullptr
#endif
    };
  }
#endif

  // wrapper struct for struct VmaAllocatorCreateInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocator_create_info.html
  struct AllocatorCreateInfo {
    using NativeType = VmaAllocatorCreateInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR AllocatorCreateInfo(
        AllocatorCreateFlags flags_ = {}
      , VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice_ = {}
      , VULKAN_HPP_NAMESPACE::Device device_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize preferredLargeHeapBlockSize_ = {}
      , const VULKAN_HPP_NAMESPACE::AllocationCallbacks* pAllocationCallbacks_ = {}
      , const DeviceMemoryCallbacks* pDeviceMemoryCallbacks_ = {}
      , const VULKAN_HPP_NAMESPACE::DeviceSize* pHeapSizeLimit_ = {}
      , const VulkanFunctions* pVulkanFunctions_ = {}
      , VULKAN_HPP_NAMESPACE::Instance instance_ = {}
      , uint32_t vulkanApiVersion_ = {}
#if VMA_EXTERNAL_MEMORY
      , const VULKAN_HPP_NAMESPACE::ExternalMemoryHandleTypeFlagsKHR* pTypeExternalMemoryHandleTypes_ = {}
#endif 
    ) VULKAN_HPP_NOEXCEPT
      : flags { flags_ }
      , physicalDevice { physicalDevice_ }
      , device { device_ }
      , preferredLargeHeapBlockSize { preferredLargeHeapBlockSize_ }
      , pAllocationCallbacks { pAllocationCallbacks_ }
      , pDeviceMemoryCallbacks { pDeviceMemoryCallbacks_ }
      , pHeapSizeLimit { pHeapSizeLimit_ }
      , pVulkanFunctions { pVulkanFunctions_ }
      , instance { instance_ }
      , vulkanApiVersion { vulkanApiVersion_ }
#if VMA_EXTERNAL_MEMORY
      , pTypeExternalMemoryHandleTypes { pTypeExternalMemoryHandleTypes_ }
#endif 
         {}

    VULKAN_HPP_CONSTEXPR AllocatorCreateInfo(AllocatorCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocatorCreateInfo(VmaAllocatorCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : AllocatorCreateInfo(*reinterpret_cast<AllocatorCreateInfo const *>(&rhs)) {}

    AllocatorCreateInfo& operator=(AllocatorCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    AllocatorCreateInfo& operator=(VmaAllocatorCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::AllocatorCreateInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setFlags(AllocatorCreateFlags flags_) VULKAN_HPP_NOEXCEPT {
      flags = flags_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setPhysicalDevice(VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice_) VULKAN_HPP_NOEXCEPT {
      physicalDevice = physicalDevice_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setDevice(VULKAN_HPP_NAMESPACE::Device device_) VULKAN_HPP_NOEXCEPT {
      device = device_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setPreferredLargeHeapBlockSize(VULKAN_HPP_NAMESPACE::DeviceSize preferredLargeHeapBlockSize_) VULKAN_HPP_NOEXCEPT {
      preferredLargeHeapBlockSize = preferredLargeHeapBlockSize_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setPAllocationCallbacks(const VULKAN_HPP_NAMESPACE::AllocationCallbacks* pAllocationCallbacks_) VULKAN_HPP_NOEXCEPT {
      pAllocationCallbacks = pAllocationCallbacks_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setPDeviceMemoryCallbacks(const DeviceMemoryCallbacks* pDeviceMemoryCallbacks_) VULKAN_HPP_NOEXCEPT {
      pDeviceMemoryCallbacks = pDeviceMemoryCallbacks_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setPHeapSizeLimit(const VULKAN_HPP_NAMESPACE::DeviceSize* pHeapSizeLimit_) VULKAN_HPP_NOEXCEPT {
      pHeapSizeLimit = pHeapSizeLimit_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setPVulkanFunctions(const VulkanFunctions* pVulkanFunctions_) VULKAN_HPP_NOEXCEPT {
      pVulkanFunctions = pVulkanFunctions_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setInstance(VULKAN_HPP_NAMESPACE::Instance instance_) VULKAN_HPP_NOEXCEPT {
      instance = instance_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setVulkanApiVersion(uint32_t vulkanApiVersion_) VULKAN_HPP_NOEXCEPT {
      vulkanApiVersion = vulkanApiVersion_;
      return *this;
    }

#if VMA_EXTERNAL_MEMORY
    VULKAN_HPP_CONSTEXPR_14 AllocatorCreateInfo& setPTypeExternalMemoryHandleTypes(const VULKAN_HPP_NAMESPACE::ExternalMemoryHandleTypeFlagsKHR* pTypeExternalMemoryHandleTypes_) VULKAN_HPP_NOEXCEPT {
      pTypeExternalMemoryHandleTypes = pTypeExternalMemoryHandleTypes_;
      return *this;
    }
#endif 
#endif

    operator VmaAllocatorCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaAllocatorCreateInfo *>(this);
    }

    operator VmaAllocatorCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaAllocatorCreateInfo *>(this);
    }

    operator VmaAllocatorCreateInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaAllocatorCreateInfo *>(this);
    }

    operator VmaAllocatorCreateInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaAllocatorCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<AllocatorCreateFlags const &
             , VULKAN_HPP_NAMESPACE::PhysicalDevice const &
             , VULKAN_HPP_NAMESPACE::Device const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , const VULKAN_HPP_NAMESPACE::AllocationCallbacks* const &
             , const DeviceMemoryCallbacks* const &
             , const VULKAN_HPP_NAMESPACE::DeviceSize* const &
             , const VulkanFunctions* const &
             , VULKAN_HPP_NAMESPACE::Instance const &
             , uint32_t const &
#if VMA_EXTERNAL_MEMORY
             , const VULKAN_HPP_NAMESPACE::ExternalMemoryHandleTypeFlagsKHR* const &
#endif 
               >
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(flags
                    , physicalDevice
                    , device
                    , preferredLargeHeapBlockSize
                    , pAllocationCallbacks
                    , pDeviceMemoryCallbacks
                    , pHeapSizeLimit
                    , pVulkanFunctions
                    , instance
                    , vulkanApiVersion
#if VMA_EXTERNAL_MEMORY
                    , pTypeExternalMemoryHandleTypes
#endif 
                      );
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(AllocatorCreateInfo const &) const = default;
#else
  bool operator==(AllocatorCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return flags == rhs.flags
        && physicalDevice == rhs.physicalDevice
        && device == rhs.device
        && preferredLargeHeapBlockSize == rhs.preferredLargeHeapBlockSize
        && pAllocationCallbacks == rhs.pAllocationCallbacks
        && pDeviceMemoryCallbacks == rhs.pDeviceMemoryCallbacks
        && pHeapSizeLimit == rhs.pHeapSizeLimit
        && pVulkanFunctions == rhs.pVulkanFunctions
        && instance == rhs.instance
        && vulkanApiVersion == rhs.vulkanApiVersion
#if VMA_EXTERNAL_MEMORY
        && pTypeExternalMemoryHandleTypes == rhs.pTypeExternalMemoryHandleTypes
#endif 
           ;
#endif
  }
  bool operator!=(AllocatorCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    AllocatorCreateFlags flags = {};
    VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice = {};
    VULKAN_HPP_NAMESPACE::Device device = {};
    VULKAN_HPP_NAMESPACE::DeviceSize preferredLargeHeapBlockSize = {};
    const VULKAN_HPP_NAMESPACE::AllocationCallbacks* pAllocationCallbacks = {};
    const DeviceMemoryCallbacks* pDeviceMemoryCallbacks = {};
    const VULKAN_HPP_NAMESPACE::DeviceSize* pHeapSizeLimit = {};
    const VulkanFunctions* pVulkanFunctions = {};
    VULKAN_HPP_NAMESPACE::Instance instance = {};
    uint32_t vulkanApiVersion = {};
#if VMA_EXTERNAL_MEMORY
    const VULKAN_HPP_NAMESPACE::ExternalMemoryHandleTypeFlagsKHR* pTypeExternalMemoryHandleTypes = {};
#endif 
  };

  // wrapper struct for struct VmaAllocatorInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocator_info.html
  struct AllocatorInfo {
    using NativeType = VmaAllocatorInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR AllocatorInfo(
        VULKAN_HPP_NAMESPACE::Instance instance_ = {}
      , VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice_ = {}
      , VULKAN_HPP_NAMESPACE::Device device_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : instance { instance_ }
      , physicalDevice { physicalDevice_ }
      , device { device_ } {}

    VULKAN_HPP_CONSTEXPR AllocatorInfo(AllocatorInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocatorInfo(VmaAllocatorInfo const & rhs) VULKAN_HPP_NOEXCEPT : AllocatorInfo(*reinterpret_cast<AllocatorInfo const *>(&rhs)) {}

    AllocatorInfo& operator=(AllocatorInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    AllocatorInfo& operator=(VmaAllocatorInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::AllocatorInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 AllocatorInfo& setInstance(VULKAN_HPP_NAMESPACE::Instance instance_) VULKAN_HPP_NOEXCEPT {
      instance = instance_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorInfo& setPhysicalDevice(VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice_) VULKAN_HPP_NOEXCEPT {
      physicalDevice = physicalDevice_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocatorInfo& setDevice(VULKAN_HPP_NAMESPACE::Device device_) VULKAN_HPP_NOEXCEPT {
      device = device_;
      return *this;
    }
#endif

    operator VmaAllocatorInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaAllocatorInfo *>(this);
    }

    operator VmaAllocatorInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaAllocatorInfo *>(this);
    }

    operator VmaAllocatorInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaAllocatorInfo *>(this);
    }

    operator VmaAllocatorInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaAllocatorInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<VULKAN_HPP_NAMESPACE::Instance const &
             , VULKAN_HPP_NAMESPACE::PhysicalDevice const &
             , VULKAN_HPP_NAMESPACE::Device const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(instance
                    , physicalDevice
                    , device);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(AllocatorInfo const &) const = default;
#else
  bool operator==(AllocatorInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return instance == rhs.instance
        && physicalDevice == rhs.physicalDevice
        && device == rhs.device;
#endif
  }
  bool operator!=(AllocatorInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    VULKAN_HPP_NAMESPACE::Instance instance = {};
    VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice = {};
    VULKAN_HPP_NAMESPACE::Device device = {};
  };

  // wrapper struct for struct VmaStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_statistics.html
  struct Statistics {
    using NativeType = VmaStatistics;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR Statistics(
        uint32_t blockCount_ = {}
      , uint32_t allocationCount_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize blockBytes_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize allocationBytes_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : blockCount { blockCount_ }
      , allocationCount { allocationCount_ }
      , blockBytes { blockBytes_ }
      , allocationBytes { allocationBytes_ } {}

    VULKAN_HPP_CONSTEXPR Statistics(Statistics const &) VULKAN_HPP_NOEXCEPT = default;
    Statistics(VmaStatistics const & rhs) VULKAN_HPP_NOEXCEPT : Statistics(*reinterpret_cast<Statistics const *>(&rhs)) {}

    Statistics& operator=(Statistics const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    Statistics& operator=(VmaStatistics const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::Statistics const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 Statistics& setBlockCount(uint32_t blockCount_) VULKAN_HPP_NOEXCEPT {
      blockCount = blockCount_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 Statistics& setAllocationCount(uint32_t allocationCount_) VULKAN_HPP_NOEXCEPT {
      allocationCount = allocationCount_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 Statistics& setBlockBytes(VULKAN_HPP_NAMESPACE::DeviceSize blockBytes_) VULKAN_HPP_NOEXCEPT {
      blockBytes = blockBytes_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 Statistics& setAllocationBytes(VULKAN_HPP_NAMESPACE::DeviceSize allocationBytes_) VULKAN_HPP_NOEXCEPT {
      allocationBytes = allocationBytes_;
      return *this;
    }
#endif

    operator VmaStatistics const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaStatistics *>(this);
    }

    operator VmaStatistics&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaStatistics *>(this);
    }

    operator VmaStatistics const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaStatistics *>(this);
    }

    operator VmaStatistics*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaStatistics *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<uint32_t const &
             , uint32_t const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(blockCount
                    , allocationCount
                    , blockBytes
                    , allocationBytes);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(Statistics const &) const = default;
#else
  bool operator==(Statistics const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return blockCount == rhs.blockCount
        && allocationCount == rhs.allocationCount
        && blockBytes == rhs.blockBytes
        && allocationBytes == rhs.allocationBytes;
#endif
  }
  bool operator!=(Statistics const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    uint32_t blockCount = {};
    uint32_t allocationCount = {};
    VULKAN_HPP_NAMESPACE::DeviceSize blockBytes = {};
    VULKAN_HPP_NAMESPACE::DeviceSize allocationBytes = {};
  };

  // wrapper struct for struct VmaDetailedStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_detailed_statistics.html
  struct DetailedStatistics {
    using NativeType = VmaDetailedStatistics;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DetailedStatistics(
        Statistics statistics_ = {}
      , uint32_t unusedRangeCount_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMin_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMax_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMin_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMax_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : statistics { statistics_ }
      , unusedRangeCount { unusedRangeCount_ }
      , allocationSizeMin { allocationSizeMin_ }
      , allocationSizeMax { allocationSizeMax_ }
      , unusedRangeSizeMin { unusedRangeSizeMin_ }
      , unusedRangeSizeMax { unusedRangeSizeMax_ } {}

    VULKAN_HPP_CONSTEXPR DetailedStatistics(DetailedStatistics const &) VULKAN_HPP_NOEXCEPT = default;
    DetailedStatistics(VmaDetailedStatistics const & rhs) VULKAN_HPP_NOEXCEPT : DetailedStatistics(*reinterpret_cast<DetailedStatistics const *>(&rhs)) {}

    DetailedStatistics& operator=(DetailedStatistics const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    DetailedStatistics& operator=(VmaDetailedStatistics const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DetailedStatistics const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 DetailedStatistics& setStatistics(Statistics statistics_) VULKAN_HPP_NOEXCEPT {
      statistics = statistics_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DetailedStatistics& setUnusedRangeCount(uint32_t unusedRangeCount_) VULKAN_HPP_NOEXCEPT {
      unusedRangeCount = unusedRangeCount_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DetailedStatistics& setAllocationSizeMin(VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMin_) VULKAN_HPP_NOEXCEPT {
      allocationSizeMin = allocationSizeMin_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DetailedStatistics& setAllocationSizeMax(VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMax_) VULKAN_HPP_NOEXCEPT {
      allocationSizeMax = allocationSizeMax_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DetailedStatistics& setUnusedRangeSizeMin(VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMin_) VULKAN_HPP_NOEXCEPT {
      unusedRangeSizeMin = unusedRangeSizeMin_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DetailedStatistics& setUnusedRangeSizeMax(VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMax_) VULKAN_HPP_NOEXCEPT {
      unusedRangeSizeMax = unusedRangeSizeMax_;
      return *this;
    }
#endif

    operator VmaDetailedStatistics const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDetailedStatistics *>(this);
    }

    operator VmaDetailedStatistics&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDetailedStatistics *>(this);
    }

    operator VmaDetailedStatistics const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaDetailedStatistics *>(this);
    }

    operator VmaDetailedStatistics*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaDetailedStatistics *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<Statistics const &
             , uint32_t const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(statistics
                    , unusedRangeCount
                    , allocationSizeMin
                    , allocationSizeMax
                    , unusedRangeSizeMin
                    , unusedRangeSizeMax);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(DetailedStatistics const &) const = default;
#else
  bool operator==(DetailedStatistics const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return statistics == rhs.statistics
        && unusedRangeCount == rhs.unusedRangeCount
        && allocationSizeMin == rhs.allocationSizeMin
        && allocationSizeMax == rhs.allocationSizeMax
        && unusedRangeSizeMin == rhs.unusedRangeSizeMin
        && unusedRangeSizeMax == rhs.unusedRangeSizeMax;
#endif
  }
  bool operator!=(DetailedStatistics const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    Statistics statistics = {};
    uint32_t unusedRangeCount = {};
    VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMin = {};
    VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMax = {};
    VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMin = {};
    VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMax = {};
  };

  // wrapper struct for struct VmaTotalStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_total_statistics.html
  struct TotalStatistics {
    using NativeType = VmaTotalStatistics;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR_14 TotalStatistics(
        std::array<DetailedStatistics, VULKAN_HPP_NAMESPACE::MaxMemoryTypes> memoryType_ = {}
      , std::array<DetailedStatistics, VULKAN_HPP_NAMESPACE::MaxMemoryHeaps> memoryHeap_ = {}
      , DetailedStatistics total_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : memoryType { memoryType_ }
      , memoryHeap { memoryHeap_ }
      , total { total_ } {}

    VULKAN_HPP_CONSTEXPR_14 TotalStatistics(TotalStatistics const &) VULKAN_HPP_NOEXCEPT = default;
    TotalStatistics(VmaTotalStatistics const & rhs) VULKAN_HPP_NOEXCEPT : TotalStatistics(*reinterpret_cast<TotalStatistics const *>(&rhs)) {}

    TotalStatistics& operator=(TotalStatistics const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    TotalStatistics& operator=(VmaTotalStatistics const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::TotalStatistics const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 TotalStatistics& setMemoryType(std::array<DetailedStatistics, VULKAN_HPP_NAMESPACE::MaxMemoryTypes> memoryType_) VULKAN_HPP_NOEXCEPT {
      memoryType = memoryType_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 TotalStatistics& setMemoryHeap(std::array<DetailedStatistics, VULKAN_HPP_NAMESPACE::MaxMemoryHeaps> memoryHeap_) VULKAN_HPP_NOEXCEPT {
      memoryHeap = memoryHeap_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 TotalStatistics& setTotal(DetailedStatistics total_) VULKAN_HPP_NOEXCEPT {
      total = total_;
      return *this;
    }
#endif

    operator VmaTotalStatistics const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaTotalStatistics *>(this);
    }

    operator VmaTotalStatistics&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaTotalStatistics *>(this);
    }

    operator VmaTotalStatistics const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaTotalStatistics *>(this);
    }

    operator VmaTotalStatistics*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaTotalStatistics *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<VULKAN_HPP_NAMESPACE::ArrayWrapper1D<DetailedStatistics, VULKAN_HPP_NAMESPACE::MaxMemoryTypes> const &
             , VULKAN_HPP_NAMESPACE::ArrayWrapper1D<DetailedStatistics, VULKAN_HPP_NAMESPACE::MaxMemoryHeaps> const &
             , DetailedStatistics const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(memoryType
                    , memoryHeap
                    , total);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(TotalStatistics const &) const = default;
#else
  bool operator==(TotalStatistics const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return memoryType == rhs.memoryType
        && memoryHeap == rhs.memoryHeap
        && total == rhs.total;
#endif
  }
  bool operator!=(TotalStatistics const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    VULKAN_HPP_NAMESPACE::ArrayWrapper1D<DetailedStatistics, VULKAN_HPP_NAMESPACE::MaxMemoryTypes> memoryType = {};
    VULKAN_HPP_NAMESPACE::ArrayWrapper1D<DetailedStatistics, VULKAN_HPP_NAMESPACE::MaxMemoryHeaps> memoryHeap = {};
    DetailedStatistics total = {};
  };

  // wrapper struct for struct VmaBudget, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_budget.html
  struct Budget {
    using NativeType = VmaBudget;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR Budget(
        Statistics statistics_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize usage_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize budget_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : statistics { statistics_ }
      , usage { usage_ }
      , budget { budget_ } {}

    VULKAN_HPP_CONSTEXPR Budget(Budget const &) VULKAN_HPP_NOEXCEPT = default;
    Budget(VmaBudget const & rhs) VULKAN_HPP_NOEXCEPT : Budget(*reinterpret_cast<Budget const *>(&rhs)) {}

    Budget& operator=(Budget const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    Budget& operator=(VmaBudget const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::Budget const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 Budget& setStatistics(Statistics statistics_) VULKAN_HPP_NOEXCEPT {
      statistics = statistics_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 Budget& setUsage(VULKAN_HPP_NAMESPACE::DeviceSize usage_) VULKAN_HPP_NOEXCEPT {
      usage = usage_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 Budget& setBudget(VULKAN_HPP_NAMESPACE::DeviceSize budget_) VULKAN_HPP_NOEXCEPT {
      budget = budget_;
      return *this;
    }
#endif

    operator VmaBudget const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaBudget *>(this);
    }

    operator VmaBudget&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaBudget *>(this);
    }

    operator VmaBudget const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaBudget *>(this);
    }

    operator VmaBudget*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaBudget *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<Statistics const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(statistics
                    , usage
                    , budget);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(Budget const &) const = default;
#else
  bool operator==(Budget const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return statistics == rhs.statistics
        && usage == rhs.usage
        && budget == rhs.budget;
#endif
  }
  bool operator!=(Budget const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    Statistics statistics = {};
    VULKAN_HPP_NAMESPACE::DeviceSize usage = {};
    VULKAN_HPP_NAMESPACE::DeviceSize budget = {};
  };

  // wrapper struct for struct VmaAllocationCreateInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation_create_info.html
  struct AllocationCreateInfo {
    using NativeType = VmaAllocationCreateInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR AllocationCreateInfo(
        AllocationCreateFlags flags_ = {}
      , MemoryUsage usage_ = {}
      , VULKAN_HPP_NAMESPACE::MemoryPropertyFlags requiredFlags_ = {}
      , VULKAN_HPP_NAMESPACE::MemoryPropertyFlags preferredFlags_ = {}
      , uint32_t memoryTypeBits_ = {}
      , Pool pool_ = {}
      , void* pUserData_ = {}
      , float priority_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : flags { flags_ }
      , usage { usage_ }
      , requiredFlags { requiredFlags_ }
      , preferredFlags { preferredFlags_ }
      , memoryTypeBits { memoryTypeBits_ }
      , pool { pool_ }
      , pUserData { pUserData_ }
      , priority { priority_ } {}

    VULKAN_HPP_CONSTEXPR AllocationCreateInfo(AllocationCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocationCreateInfo(VmaAllocationCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : AllocationCreateInfo(*reinterpret_cast<AllocationCreateInfo const *>(&rhs)) {}

    AllocationCreateInfo& operator=(AllocationCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    AllocationCreateInfo& operator=(VmaAllocationCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::AllocationCreateInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 AllocationCreateInfo& setFlags(AllocationCreateFlags flags_) VULKAN_HPP_NOEXCEPT {
      flags = flags_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationCreateInfo& setUsage(MemoryUsage usage_) VULKAN_HPP_NOEXCEPT {
      usage = usage_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationCreateInfo& setRequiredFlags(VULKAN_HPP_NAMESPACE::MemoryPropertyFlags requiredFlags_) VULKAN_HPP_NOEXCEPT {
      requiredFlags = requiredFlags_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationCreateInfo& setPreferredFlags(VULKAN_HPP_NAMESPACE::MemoryPropertyFlags preferredFlags_) VULKAN_HPP_NOEXCEPT {
      preferredFlags = preferredFlags_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationCreateInfo& setMemoryTypeBits(uint32_t memoryTypeBits_) VULKAN_HPP_NOEXCEPT {
      memoryTypeBits = memoryTypeBits_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationCreateInfo& setPool(Pool pool_) VULKAN_HPP_NOEXCEPT {
      pool = pool_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationCreateInfo& setPUserData(void* pUserData_) VULKAN_HPP_NOEXCEPT {
      pUserData = pUserData_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationCreateInfo& setPriority(float priority_) VULKAN_HPP_NOEXCEPT {
      priority = priority_;
      return *this;
    }
#endif

    operator VmaAllocationCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaAllocationCreateInfo *>(this);
    }

    operator VmaAllocationCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaAllocationCreateInfo *>(this);
    }

    operator VmaAllocationCreateInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaAllocationCreateInfo *>(this);
    }

    operator VmaAllocationCreateInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaAllocationCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<AllocationCreateFlags const &
             , MemoryUsage const &
             , VULKAN_HPP_NAMESPACE::MemoryPropertyFlags const &
             , VULKAN_HPP_NAMESPACE::MemoryPropertyFlags const &
             , uint32_t const &
             , Pool const &
             , void* const &
             , float const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(flags
                    , usage
                    , requiredFlags
                    , preferredFlags
                    , memoryTypeBits
                    , pool
                    , pUserData
                    , priority);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(AllocationCreateInfo const &) const = default;
#else
  bool operator==(AllocationCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return flags == rhs.flags
        && usage == rhs.usage
        && requiredFlags == rhs.requiredFlags
        && preferredFlags == rhs.preferredFlags
        && memoryTypeBits == rhs.memoryTypeBits
        && pool == rhs.pool
        && pUserData == rhs.pUserData
        && priority == rhs.priority;
#endif
  }
  bool operator!=(AllocationCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    AllocationCreateFlags flags = {};
    MemoryUsage usage = {};
    VULKAN_HPP_NAMESPACE::MemoryPropertyFlags requiredFlags = {};
    VULKAN_HPP_NAMESPACE::MemoryPropertyFlags preferredFlags = {};
    uint32_t memoryTypeBits = {};
    Pool pool = {};
    void* pUserData = {};
    float priority = {};
  };

  // wrapper struct for struct VmaPoolCreateInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_pool_create_info.html
  struct PoolCreateInfo {
    using NativeType = VmaPoolCreateInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR PoolCreateInfo(
        uint32_t memoryTypeIndex_ = {}
      , PoolCreateFlags flags_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize blockSize_ = {}
      , size_t minBlockCount_ = {}
      , size_t maxBlockCount_ = {}
      , float priority_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize minAllocationAlignment_ = {}
      , void* pMemoryAllocateNext_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : memoryTypeIndex { memoryTypeIndex_ }
      , flags { flags_ }
      , blockSize { blockSize_ }
      , minBlockCount { minBlockCount_ }
      , maxBlockCount { maxBlockCount_ }
      , priority { priority_ }
      , minAllocationAlignment { minAllocationAlignment_ }
      , pMemoryAllocateNext { pMemoryAllocateNext_ } {}

    VULKAN_HPP_CONSTEXPR PoolCreateInfo(PoolCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    PoolCreateInfo(VmaPoolCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : PoolCreateInfo(*reinterpret_cast<PoolCreateInfo const *>(&rhs)) {}

    PoolCreateInfo& operator=(PoolCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    PoolCreateInfo& operator=(VmaPoolCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::PoolCreateInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 PoolCreateInfo& setMemoryTypeIndex(uint32_t memoryTypeIndex_) VULKAN_HPP_NOEXCEPT {
      memoryTypeIndex = memoryTypeIndex_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 PoolCreateInfo& setFlags(PoolCreateFlags flags_) VULKAN_HPP_NOEXCEPT {
      flags = flags_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 PoolCreateInfo& setBlockSize(VULKAN_HPP_NAMESPACE::DeviceSize blockSize_) VULKAN_HPP_NOEXCEPT {
      blockSize = blockSize_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 PoolCreateInfo& setMinBlockCount(size_t minBlockCount_) VULKAN_HPP_NOEXCEPT {
      minBlockCount = minBlockCount_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 PoolCreateInfo& setMaxBlockCount(size_t maxBlockCount_) VULKAN_HPP_NOEXCEPT {
      maxBlockCount = maxBlockCount_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 PoolCreateInfo& setPriority(float priority_) VULKAN_HPP_NOEXCEPT {
      priority = priority_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 PoolCreateInfo& setMinAllocationAlignment(VULKAN_HPP_NAMESPACE::DeviceSize minAllocationAlignment_) VULKAN_HPP_NOEXCEPT {
      minAllocationAlignment = minAllocationAlignment_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 PoolCreateInfo& setPMemoryAllocateNext(void* pMemoryAllocateNext_) VULKAN_HPP_NOEXCEPT {
      pMemoryAllocateNext = pMemoryAllocateNext_;
      return *this;
    }
#endif

    operator VmaPoolCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaPoolCreateInfo *>(this);
    }

    operator VmaPoolCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaPoolCreateInfo *>(this);
    }

    operator VmaPoolCreateInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaPoolCreateInfo *>(this);
    }

    operator VmaPoolCreateInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaPoolCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<uint32_t const &
             , PoolCreateFlags const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , size_t const &
             , size_t const &
             , float const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , void* const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(memoryTypeIndex
                    , flags
                    , blockSize
                    , minBlockCount
                    , maxBlockCount
                    , priority
                    , minAllocationAlignment
                    , pMemoryAllocateNext);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(PoolCreateInfo const &) const = default;
#else
  bool operator==(PoolCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return memoryTypeIndex == rhs.memoryTypeIndex
        && flags == rhs.flags
        && blockSize == rhs.blockSize
        && minBlockCount == rhs.minBlockCount
        && maxBlockCount == rhs.maxBlockCount
        && priority == rhs.priority
        && minAllocationAlignment == rhs.minAllocationAlignment
        && pMemoryAllocateNext == rhs.pMemoryAllocateNext;
#endif
  }
  bool operator!=(PoolCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    uint32_t memoryTypeIndex = {};
    PoolCreateFlags flags = {};
    VULKAN_HPP_NAMESPACE::DeviceSize blockSize = {};
    size_t minBlockCount = {};
    size_t maxBlockCount = {};
    float priority = {};
    VULKAN_HPP_NAMESPACE::DeviceSize minAllocationAlignment = {};
    void* pMemoryAllocateNext = {};
  };

  // wrapper struct for struct VmaAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation_info.html
  struct AllocationInfo {
    using NativeType = VmaAllocationInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR AllocationInfo(
        uint32_t memoryType_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceMemory deviceMemory_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize offset_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize size_ = {}
      , void* pMappedData_ = {}
      , void* pUserData_ = {}
      , const char* pName_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : memoryType { memoryType_ }
      , deviceMemory { deviceMemory_ }
      , offset { offset_ }
      , size { size_ }
      , pMappedData { pMappedData_ }
      , pUserData { pUserData_ }
      , pName { pName_ } {}

    VULKAN_HPP_CONSTEXPR AllocationInfo(AllocationInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocationInfo(VmaAllocationInfo const & rhs) VULKAN_HPP_NOEXCEPT : AllocationInfo(*reinterpret_cast<AllocationInfo const *>(&rhs)) {}

    AllocationInfo& operator=(AllocationInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    AllocationInfo& operator=(VmaAllocationInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::AllocationInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 AllocationInfo& setMemoryType(uint32_t memoryType_) VULKAN_HPP_NOEXCEPT {
      memoryType = memoryType_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationInfo& setDeviceMemory(VULKAN_HPP_NAMESPACE::DeviceMemory deviceMemory_) VULKAN_HPP_NOEXCEPT {
      deviceMemory = deviceMemory_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationInfo& setOffset(VULKAN_HPP_NAMESPACE::DeviceSize offset_) VULKAN_HPP_NOEXCEPT {
      offset = offset_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationInfo& setSize(VULKAN_HPP_NAMESPACE::DeviceSize size_) VULKAN_HPP_NOEXCEPT {
      size = size_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationInfo& setPMappedData(void* pMappedData_) VULKAN_HPP_NOEXCEPT {
      pMappedData = pMappedData_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationInfo& setPUserData(void* pUserData_) VULKAN_HPP_NOEXCEPT {
      pUserData = pUserData_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationInfo& setPName(const char* pName_) VULKAN_HPP_NOEXCEPT {
      pName = pName_;
      return *this;
    }
#endif

    operator VmaAllocationInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaAllocationInfo *>(this);
    }

    operator VmaAllocationInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaAllocationInfo *>(this);
    }

    operator VmaAllocationInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaAllocationInfo *>(this);
    }

    operator VmaAllocationInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaAllocationInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<uint32_t const &
             , VULKAN_HPP_NAMESPACE::DeviceMemory const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , void* const &
             , void* const &
             , const char* const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(memoryType
                    , deviceMemory
                    , offset
                    , size
                    , pMappedData
                    , pUserData
                    , pName);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(AllocationInfo const &) const = default;
#else
  bool operator==(AllocationInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return memoryType == rhs.memoryType
        && deviceMemory == rhs.deviceMemory
        && offset == rhs.offset
        && size == rhs.size
        && pMappedData == rhs.pMappedData
        && pUserData == rhs.pUserData
        && pName == rhs.pName;
#endif
  }
  bool operator!=(AllocationInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    uint32_t memoryType = {};
    VULKAN_HPP_NAMESPACE::DeviceMemory deviceMemory = {};
    VULKAN_HPP_NAMESPACE::DeviceSize offset = {};
    VULKAN_HPP_NAMESPACE::DeviceSize size = {};
    void* pMappedData = {};
    void* pUserData = {};
    const char* pName = {};
  };

  // wrapper struct for struct VmaAllocationInfo2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation_info2.html
  struct AllocationInfo2 {
    using NativeType = VmaAllocationInfo2;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR AllocationInfo2(
        AllocationInfo allocationInfo_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize blockSize_ = {}
      , VULKAN_HPP_NAMESPACE::Bool32 dedicatedMemory_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : allocationInfo { allocationInfo_ }
      , blockSize { blockSize_ }
      , dedicatedMemory { dedicatedMemory_ } {}

    VULKAN_HPP_CONSTEXPR AllocationInfo2(AllocationInfo2 const &) VULKAN_HPP_NOEXCEPT = default;
    AllocationInfo2(VmaAllocationInfo2 const & rhs) VULKAN_HPP_NOEXCEPT : AllocationInfo2(*reinterpret_cast<AllocationInfo2 const *>(&rhs)) {}

    AllocationInfo2& operator=(AllocationInfo2 const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    AllocationInfo2& operator=(VmaAllocationInfo2 const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::AllocationInfo2 const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 AllocationInfo2& setAllocationInfo(AllocationInfo allocationInfo_) VULKAN_HPP_NOEXCEPT {
      allocationInfo = allocationInfo_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationInfo2& setBlockSize(VULKAN_HPP_NAMESPACE::DeviceSize blockSize_) VULKAN_HPP_NOEXCEPT {
      blockSize = blockSize_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 AllocationInfo2& setDedicatedMemory(VULKAN_HPP_NAMESPACE::Bool32 dedicatedMemory_) VULKAN_HPP_NOEXCEPT {
      dedicatedMemory = dedicatedMemory_;
      return *this;
    }
#endif

    operator VmaAllocationInfo2 const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaAllocationInfo2 *>(this);
    }

    operator VmaAllocationInfo2&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaAllocationInfo2 *>(this);
    }

    operator VmaAllocationInfo2 const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaAllocationInfo2 *>(this);
    }

    operator VmaAllocationInfo2*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaAllocationInfo2 *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<AllocationInfo const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::Bool32 const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(allocationInfo
                    , blockSize
                    , dedicatedMemory);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(AllocationInfo2 const &) const = default;
#else
  bool operator==(AllocationInfo2 const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return allocationInfo == rhs.allocationInfo
        && blockSize == rhs.blockSize
        && dedicatedMemory == rhs.dedicatedMemory;
#endif
  }
  bool operator!=(AllocationInfo2 const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    AllocationInfo allocationInfo = {};
    VULKAN_HPP_NAMESPACE::DeviceSize blockSize = {};
    VULKAN_HPP_NAMESPACE::Bool32 dedicatedMemory = {};
  };

  // wrapper struct for struct VmaDefragmentationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_defragmentation_info.html
  struct DefragmentationInfo {
    using NativeType = VmaDefragmentationInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DefragmentationInfo(
        DefragmentationFlags flags_ = {}
      , Pool pool_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize maxBytesPerPass_ = {}
      , uint32_t maxAllocationsPerPass_ = {}
      , PFN_vmaCheckDefragmentationBreakFunction pfnBreakCallback_ = {}
      , void* pBreakCallbackUserData_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : flags { flags_ }
      , pool { pool_ }
      , maxBytesPerPass { maxBytesPerPass_ }
      , maxAllocationsPerPass { maxAllocationsPerPass_ }
      , pfnBreakCallback { pfnBreakCallback_ }
      , pBreakCallbackUserData { pBreakCallbackUserData_ } {}

    VULKAN_HPP_CONSTEXPR DefragmentationInfo(DefragmentationInfo const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationInfo(VmaDefragmentationInfo const & rhs) VULKAN_HPP_NOEXCEPT : DefragmentationInfo(*reinterpret_cast<DefragmentationInfo const *>(&rhs)) {}

    DefragmentationInfo& operator=(DefragmentationInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    DefragmentationInfo& operator=(VmaDefragmentationInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DefragmentationInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 DefragmentationInfo& setFlags(DefragmentationFlags flags_) VULKAN_HPP_NOEXCEPT {
      flags = flags_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationInfo& setPool(Pool pool_) VULKAN_HPP_NOEXCEPT {
      pool = pool_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationInfo& setMaxBytesPerPass(VULKAN_HPP_NAMESPACE::DeviceSize maxBytesPerPass_) VULKAN_HPP_NOEXCEPT {
      maxBytesPerPass = maxBytesPerPass_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationInfo& setMaxAllocationsPerPass(uint32_t maxAllocationsPerPass_) VULKAN_HPP_NOEXCEPT {
      maxAllocationsPerPass = maxAllocationsPerPass_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationInfo& setPfnBreakCallback(PFN_vmaCheckDefragmentationBreakFunction pfnBreakCallback_) VULKAN_HPP_NOEXCEPT {
      pfnBreakCallback = pfnBreakCallback_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationInfo& setPBreakCallbackUserData(void* pBreakCallbackUserData_) VULKAN_HPP_NOEXCEPT {
      pBreakCallbackUserData = pBreakCallbackUserData_;
      return *this;
    }
#endif

    operator VmaDefragmentationInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDefragmentationInfo *>(this);
    }

    operator VmaDefragmentationInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDefragmentationInfo *>(this);
    }

    operator VmaDefragmentationInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaDefragmentationInfo *>(this);
    }

    operator VmaDefragmentationInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaDefragmentationInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<DefragmentationFlags const &
             , Pool const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , uint32_t const &
             , PFN_vmaCheckDefragmentationBreakFunction const &
             , void* const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(flags
                    , pool
                    , maxBytesPerPass
                    , maxAllocationsPerPass
                    , pfnBreakCallback
                    , pBreakCallbackUserData);
    }
#endif

  bool operator==(DefragmentationInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return flags == rhs.flags
        && pool == rhs.pool
        && maxBytesPerPass == rhs.maxBytesPerPass
        && maxAllocationsPerPass == rhs.maxAllocationsPerPass
        && pfnBreakCallback == rhs.pfnBreakCallback
        && pBreakCallbackUserData == rhs.pBreakCallbackUserData;
#endif
  }
  bool operator!=(DefragmentationInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }

  public:
    DefragmentationFlags flags = {};
    Pool pool = {};
    VULKAN_HPP_NAMESPACE::DeviceSize maxBytesPerPass = {};
    uint32_t maxAllocationsPerPass = {};
    PFN_vmaCheckDefragmentationBreakFunction pfnBreakCallback = {};
    void* pBreakCallbackUserData = {};
  };

  // wrapper struct for struct VmaDefragmentationMove, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_defragmentation_move.html
  struct DefragmentationMove {
    using NativeType = VmaDefragmentationMove;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DefragmentationMove(
        DefragmentationMoveOperation operation_ = {}
      , Allocation srcAllocation_ = {}
      , Allocation dstTmpAllocation_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : operation { operation_ }
      , srcAllocation { srcAllocation_ }
      , dstTmpAllocation { dstTmpAllocation_ } {}

    VULKAN_HPP_CONSTEXPR DefragmentationMove(DefragmentationMove const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationMove(VmaDefragmentationMove const & rhs) VULKAN_HPP_NOEXCEPT : DefragmentationMove(*reinterpret_cast<DefragmentationMove const *>(&rhs)) {}

    DefragmentationMove& operator=(DefragmentationMove const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    DefragmentationMove& operator=(VmaDefragmentationMove const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DefragmentationMove const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 DefragmentationMove& setOperation(DefragmentationMoveOperation operation_) VULKAN_HPP_NOEXCEPT {
      operation = operation_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationMove& setSrcAllocation(Allocation srcAllocation_) VULKAN_HPP_NOEXCEPT {
      srcAllocation = srcAllocation_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationMove& setDstTmpAllocation(Allocation dstTmpAllocation_) VULKAN_HPP_NOEXCEPT {
      dstTmpAllocation = dstTmpAllocation_;
      return *this;
    }
#endif

    operator VmaDefragmentationMove const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDefragmentationMove *>(this);
    }

    operator VmaDefragmentationMove&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDefragmentationMove *>(this);
    }

    operator VmaDefragmentationMove const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaDefragmentationMove *>(this);
    }

    operator VmaDefragmentationMove*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaDefragmentationMove *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<DefragmentationMoveOperation const &
             , Allocation const &
             , Allocation const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(operation
                    , srcAllocation
                    , dstTmpAllocation);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(DefragmentationMove const &) const = default;
#else
  bool operator==(DefragmentationMove const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return operation == rhs.operation
        && srcAllocation == rhs.srcAllocation
        && dstTmpAllocation == rhs.dstTmpAllocation;
#endif
  }
  bool operator!=(DefragmentationMove const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    DefragmentationMoveOperation operation = {};
    Allocation srcAllocation = {};
    Allocation dstTmpAllocation = {};
  };

  // wrapper struct for struct VmaDefragmentationPassMoveInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_defragmentation_pass_move_info.html
  struct DefragmentationPassMoveInfo {
    using NativeType = VmaDefragmentationPassMoveInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DefragmentationPassMoveInfo(
        uint32_t moveCount_ = {}
      , DefragmentationMove* pMoves_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : moveCount { moveCount_ }
      , pMoves { pMoves_ } {}

    VULKAN_HPP_CONSTEXPR DefragmentationPassMoveInfo(DefragmentationPassMoveInfo const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationPassMoveInfo(VmaDefragmentationPassMoveInfo const & rhs) VULKAN_HPP_NOEXCEPT : DefragmentationPassMoveInfo(*reinterpret_cast<DefragmentationPassMoveInfo const *>(&rhs)) {}

#if !defined( VULKAN_HPP_DISABLE_ENHANCED_MODE )
    DefragmentationPassMoveInfo(
        VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<DefragmentationMove> const & moves_
    ) VULKAN_HPP_NOEXCEPT
      : moveCount { static_cast<uint32_t>(moves_.size()) }
      , pMoves { moves_.data() } {}
#endif

    DefragmentationPassMoveInfo& operator=(DefragmentationPassMoveInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    DefragmentationPassMoveInfo& operator=(VmaDefragmentationPassMoveInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DefragmentationPassMoveInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 DefragmentationPassMoveInfo& setMoveCount(uint32_t moveCount_) VULKAN_HPP_NOEXCEPT {
      moveCount = moveCount_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationPassMoveInfo& setPMoves(DefragmentationMove* pMoves_) VULKAN_HPP_NOEXCEPT {
      pMoves = pMoves_;
      return *this;
    }

#if !defined( VULKAN_HPP_DISABLE_ENHANCED_MODE )
    DefragmentationPassMoveInfo& setMoves(VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<DefragmentationMove> const & moves_) VULKAN_HPP_NOEXCEPT {
      moveCount = static_cast<uint32_t>(moves_.size());
      pMoves = moves_.data();
      return *this;
    }
#endif
#endif

    operator VmaDefragmentationPassMoveInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDefragmentationPassMoveInfo *>(this);
    }

    operator VmaDefragmentationPassMoveInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDefragmentationPassMoveInfo *>(this);
    }

    operator VmaDefragmentationPassMoveInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaDefragmentationPassMoveInfo *>(this);
    }

    operator VmaDefragmentationPassMoveInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaDefragmentationPassMoveInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<uint32_t const &
             , DefragmentationMove* const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(moveCount
                    , pMoves);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(DefragmentationPassMoveInfo const &) const = default;
#else
  bool operator==(DefragmentationPassMoveInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return moveCount == rhs.moveCount
        && pMoves == rhs.pMoves;
#endif
  }
  bool operator!=(DefragmentationPassMoveInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    uint32_t moveCount = {};
    DefragmentationMove* pMoves = {};
  };

  // wrapper struct for struct VmaDefragmentationStats, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_defragmentation_stats.html
  struct DefragmentationStats {
    using NativeType = VmaDefragmentationStats;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DefragmentationStats(
        VULKAN_HPP_NAMESPACE::DeviceSize bytesMoved_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize bytesFreed_ = {}
      , uint32_t allocationsMoved_ = {}
      , uint32_t deviceMemoryBlocksFreed_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : bytesMoved { bytesMoved_ }
      , bytesFreed { bytesFreed_ }
      , allocationsMoved { allocationsMoved_ }
      , deviceMemoryBlocksFreed { deviceMemoryBlocksFreed_ } {}

    VULKAN_HPP_CONSTEXPR DefragmentationStats(DefragmentationStats const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationStats(VmaDefragmentationStats const & rhs) VULKAN_HPP_NOEXCEPT : DefragmentationStats(*reinterpret_cast<DefragmentationStats const *>(&rhs)) {}

    DefragmentationStats& operator=(DefragmentationStats const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    DefragmentationStats& operator=(VmaDefragmentationStats const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DefragmentationStats const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 DefragmentationStats& setBytesMoved(VULKAN_HPP_NAMESPACE::DeviceSize bytesMoved_) VULKAN_HPP_NOEXCEPT {
      bytesMoved = bytesMoved_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationStats& setBytesFreed(VULKAN_HPP_NAMESPACE::DeviceSize bytesFreed_) VULKAN_HPP_NOEXCEPT {
      bytesFreed = bytesFreed_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationStats& setAllocationsMoved(uint32_t allocationsMoved_) VULKAN_HPP_NOEXCEPT {
      allocationsMoved = allocationsMoved_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationStats& setDeviceMemoryBlocksFreed(uint32_t deviceMemoryBlocksFreed_) VULKAN_HPP_NOEXCEPT {
      deviceMemoryBlocksFreed = deviceMemoryBlocksFreed_;
      return *this;
    }
#endif

    operator VmaDefragmentationStats const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDefragmentationStats *>(this);
    }

    operator VmaDefragmentationStats&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDefragmentationStats *>(this);
    }

    operator VmaDefragmentationStats const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaDefragmentationStats *>(this);
    }

    operator VmaDefragmentationStats*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaDefragmentationStats *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , uint32_t const &
             , uint32_t const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(bytesMoved
                    , bytesFreed
                    , allocationsMoved
                    , deviceMemoryBlocksFreed);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(DefragmentationStats const &) const = default;
#else
  bool operator==(DefragmentationStats const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return bytesMoved == rhs.bytesMoved
        && bytesFreed == rhs.bytesFreed
        && allocationsMoved == rhs.allocationsMoved
        && deviceMemoryBlocksFreed == rhs.deviceMemoryBlocksFreed;
#endif
  }
  bool operator!=(DefragmentationStats const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    VULKAN_HPP_NAMESPACE::DeviceSize bytesMoved = {};
    VULKAN_HPP_NAMESPACE::DeviceSize bytesFreed = {};
    uint32_t allocationsMoved = {};
    uint32_t deviceMemoryBlocksFreed = {};
  };

  // wrapper struct for struct VmaVirtualBlockCreateInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_virtual_block_create_info.html
  struct VirtualBlockCreateInfo {
    using NativeType = VmaVirtualBlockCreateInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR VirtualBlockCreateInfo(
        VULKAN_HPP_NAMESPACE::DeviceSize size_ = {}
      , VirtualBlockCreateFlags flags_ = {}
      , const VULKAN_HPP_NAMESPACE::AllocationCallbacks* pAllocationCallbacks_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : size { size_ }
      , flags { flags_ }
      , pAllocationCallbacks { pAllocationCallbacks_ } {}

    VULKAN_HPP_CONSTEXPR VirtualBlockCreateInfo(VirtualBlockCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    VirtualBlockCreateInfo(VmaVirtualBlockCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : VirtualBlockCreateInfo(*reinterpret_cast<VirtualBlockCreateInfo const *>(&rhs)) {}

    VirtualBlockCreateInfo& operator=(VirtualBlockCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    VirtualBlockCreateInfo& operator=(VmaVirtualBlockCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::VirtualBlockCreateInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 VirtualBlockCreateInfo& setSize(VULKAN_HPP_NAMESPACE::DeviceSize size_) VULKAN_HPP_NOEXCEPT {
      size = size_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VirtualBlockCreateInfo& setFlags(VirtualBlockCreateFlags flags_) VULKAN_HPP_NOEXCEPT {
      flags = flags_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VirtualBlockCreateInfo& setPAllocationCallbacks(const VULKAN_HPP_NAMESPACE::AllocationCallbacks* pAllocationCallbacks_) VULKAN_HPP_NOEXCEPT {
      pAllocationCallbacks = pAllocationCallbacks_;
      return *this;
    }
#endif

    operator VmaVirtualBlockCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaVirtualBlockCreateInfo *>(this);
    }

    operator VmaVirtualBlockCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaVirtualBlockCreateInfo *>(this);
    }

    operator VmaVirtualBlockCreateInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaVirtualBlockCreateInfo *>(this);
    }

    operator VmaVirtualBlockCreateInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaVirtualBlockCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VirtualBlockCreateFlags const &
             , const VULKAN_HPP_NAMESPACE::AllocationCallbacks* const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(size
                    , flags
                    , pAllocationCallbacks);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(VirtualBlockCreateInfo const &) const = default;
#else
  bool operator==(VirtualBlockCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return size == rhs.size
        && flags == rhs.flags
        && pAllocationCallbacks == rhs.pAllocationCallbacks;
#endif
  }
  bool operator!=(VirtualBlockCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    VULKAN_HPP_NAMESPACE::DeviceSize size = {};
    VirtualBlockCreateFlags flags = {};
    const VULKAN_HPP_NAMESPACE::AllocationCallbacks* pAllocationCallbacks = {};
  };

  // wrapper struct for struct VmaVirtualAllocationCreateInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_virtual_allocation_create_info.html
  struct VirtualAllocationCreateInfo {
    using NativeType = VmaVirtualAllocationCreateInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR VirtualAllocationCreateInfo(
        VULKAN_HPP_NAMESPACE::DeviceSize size_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize alignment_ = {}
      , VirtualAllocationCreateFlags flags_ = {}
      , void* pUserData_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : size { size_ }
      , alignment { alignment_ }
      , flags { flags_ }
      , pUserData { pUserData_ } {}

    VULKAN_HPP_CONSTEXPR VirtualAllocationCreateInfo(VirtualAllocationCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    VirtualAllocationCreateInfo(VmaVirtualAllocationCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : VirtualAllocationCreateInfo(*reinterpret_cast<VirtualAllocationCreateInfo const *>(&rhs)) {}

    VirtualAllocationCreateInfo& operator=(VirtualAllocationCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    VirtualAllocationCreateInfo& operator=(VmaVirtualAllocationCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::VirtualAllocationCreateInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 VirtualAllocationCreateInfo& setSize(VULKAN_HPP_NAMESPACE::DeviceSize size_) VULKAN_HPP_NOEXCEPT {
      size = size_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VirtualAllocationCreateInfo& setAlignment(VULKAN_HPP_NAMESPACE::DeviceSize alignment_) VULKAN_HPP_NOEXCEPT {
      alignment = alignment_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VirtualAllocationCreateInfo& setFlags(VirtualAllocationCreateFlags flags_) VULKAN_HPP_NOEXCEPT {
      flags = flags_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VirtualAllocationCreateInfo& setPUserData(void* pUserData_) VULKAN_HPP_NOEXCEPT {
      pUserData = pUserData_;
      return *this;
    }
#endif

    operator VmaVirtualAllocationCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaVirtualAllocationCreateInfo *>(this);
    }

    operator VmaVirtualAllocationCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaVirtualAllocationCreateInfo *>(this);
    }

    operator VmaVirtualAllocationCreateInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaVirtualAllocationCreateInfo *>(this);
    }

    operator VmaVirtualAllocationCreateInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaVirtualAllocationCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VirtualAllocationCreateFlags const &
             , void* const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(size
                    , alignment
                    , flags
                    , pUserData);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(VirtualAllocationCreateInfo const &) const = default;
#else
  bool operator==(VirtualAllocationCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return size == rhs.size
        && alignment == rhs.alignment
        && flags == rhs.flags
        && pUserData == rhs.pUserData;
#endif
  }
  bool operator!=(VirtualAllocationCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    VULKAN_HPP_NAMESPACE::DeviceSize size = {};
    VULKAN_HPP_NAMESPACE::DeviceSize alignment = {};
    VirtualAllocationCreateFlags flags = {};
    void* pUserData = {};
  };

  // wrapper struct for struct VmaVirtualAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_virtual_allocation_info.html
  struct VirtualAllocationInfo {
    using NativeType = VmaVirtualAllocationInfo;

#if !defined( VULKAN_HPP_NO_CONSTRUCTORS ) && !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR VirtualAllocationInfo(
        VULKAN_HPP_NAMESPACE::DeviceSize offset_ = {}
      , VULKAN_HPP_NAMESPACE::DeviceSize size_ = {}
      , void* pUserData_ = {}
    ) VULKAN_HPP_NOEXCEPT
      : offset { offset_ }
      , size { size_ }
      , pUserData { pUserData_ } {}

    VULKAN_HPP_CONSTEXPR VirtualAllocationInfo(VirtualAllocationInfo const &) VULKAN_HPP_NOEXCEPT = default;
    VirtualAllocationInfo(VmaVirtualAllocationInfo const & rhs) VULKAN_HPP_NOEXCEPT : VirtualAllocationInfo(*reinterpret_cast<VirtualAllocationInfo const *>(&rhs)) {}

    VirtualAllocationInfo& operator=(VirtualAllocationInfo const &) VULKAN_HPP_NOEXCEPT = default;
#endif

    VirtualAllocationInfo& operator=(VmaVirtualAllocationInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::VirtualAllocationInfo const *>(&rhs);
      return *this;
    }

#if !defined( VULKAN_HPP_NO_SETTERS ) && !defined( VULKAN_HPP_NO_STRUCT_SETTERS )
    VULKAN_HPP_CONSTEXPR_14 VirtualAllocationInfo& setOffset(VULKAN_HPP_NAMESPACE::DeviceSize offset_) VULKAN_HPP_NOEXCEPT {
      offset = offset_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VirtualAllocationInfo& setSize(VULKAN_HPP_NAMESPACE::DeviceSize size_) VULKAN_HPP_NOEXCEPT {
      size = size_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VirtualAllocationInfo& setPUserData(void* pUserData_) VULKAN_HPP_NOEXCEPT {
      pUserData = pUserData_;
      return *this;
    }
#endif

    operator VmaVirtualAllocationInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaVirtualAllocationInfo *>(this);
    }

    operator VmaVirtualAllocationInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaVirtualAllocationInfo *>(this);
    }

    operator VmaVirtualAllocationInfo const *() const VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<const VmaVirtualAllocationInfo *>(this);
    }

    operator VmaVirtualAllocationInfo*() VULKAN_HPP_NOEXCEPT {
      return reinterpret_cast<VmaVirtualAllocationInfo *>(this);
    }

#if defined( VULKAN_HPP_USE_REFLECT )
    std::tuple<VULKAN_HPP_NAMESPACE::DeviceSize const &
             , VULKAN_HPP_NAMESPACE::DeviceSize const &
             , void* const &>
    reflect() const VULKAN_HPP_NOEXCEPT {
      return std::tie(offset
                    , size
                    , pUserData);
    }
#endif

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
  auto operator<=>(VirtualAllocationInfo const &) const = default;
#else
  bool operator==(VirtualAllocationInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
#if defined( VULKAN_HPP_USE_REFLECT )
    return this->reflect() == rhs.reflect();
#else
    return offset == rhs.offset
        && size == rhs.size
        && pUserData == rhs.pUserData;
#endif
  }
  bool operator!=(VirtualAllocationInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
    return !operator==(rhs);
  }
#endif

  public:
    VULKAN_HPP_NAMESPACE::DeviceSize offset = {};
    VULKAN_HPP_NAMESPACE::DeviceSize size = {};
    void* pUserData = {};
  };
}
#endif