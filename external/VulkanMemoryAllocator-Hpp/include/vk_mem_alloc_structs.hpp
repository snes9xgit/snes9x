#ifndef VULKAN_MEMORY_ALLOCATOR_STRUCTS_HPP
#define VULKAN_MEMORY_ALLOCATOR_STRUCTS_HPP

namespace VMA_HPP_NAMESPACE {

  struct DeviceMemoryCallbacks {
    using NativeType = VmaDeviceMemoryCallbacks;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DeviceMemoryCallbacks(
          PFN_vmaAllocateDeviceMemoryFunction pfnAllocate_ = {}
        , PFN_vmaFreeDeviceMemoryFunction pfnFree_ = {}
        , void* pUserData_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : pfnAllocate(pfnAllocate_)
      , pfnFree(pfnFree_)
      , pUserData(pUserData_)
      {}

    VULKAN_HPP_CONSTEXPR DeviceMemoryCallbacks(DeviceMemoryCallbacks const &) VULKAN_HPP_NOEXCEPT = default;
    DeviceMemoryCallbacks(VmaDeviceMemoryCallbacks const & rhs) VULKAN_HPP_NOEXCEPT : DeviceMemoryCallbacks(*reinterpret_cast<DeviceMemoryCallbacks const *>(&rhs)) {}
#endif

    DeviceMemoryCallbacks& operator=(DeviceMemoryCallbacks const &) VULKAN_HPP_NOEXCEPT = default;
    DeviceMemoryCallbacks& operator=(VmaDeviceMemoryCallbacks const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DeviceMemoryCallbacks const *>(&rhs);
      return *this;
    }

    explicit operator VmaDeviceMemoryCallbacks const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDeviceMemoryCallbacks *>(this);
    }

    explicit operator VmaDeviceMemoryCallbacks&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDeviceMemoryCallbacks *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(DeviceMemoryCallbacks const &) const = default;
#else
    bool operator==(DeviceMemoryCallbacks const & rhs) const VULKAN_HPP_NOEXCEPT {
      return pfnAllocate == rhs.pfnAllocate
          && pfnFree == rhs.pfnFree
          && pUserData == rhs.pUserData
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    PFN_vmaAllocateDeviceMemoryFunction pfnAllocate = {};
    PFN_vmaFreeDeviceMemoryFunction pfnFree = {};
    void* pUserData = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(DeviceMemoryCallbacks) == sizeof(VmaDeviceMemoryCallbacks),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<DeviceMemoryCallbacks>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<DeviceMemoryCallbacks>::value,
                           "DeviceMemoryCallbacks is not nothrow_move_constructible!");

  struct VulkanFunctions {
    using NativeType = VmaVulkanFunctions;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
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
#if VMA_VULKAN_VERSION >= 1003000
        , PFN_vkGetDeviceBufferMemoryRequirements vkGetDeviceBufferMemoryRequirements_ = {}
        , PFN_vkGetDeviceImageMemoryRequirements vkGetDeviceImageMemoryRequirements_ = {}
#endif
      ) VULKAN_HPP_NOEXCEPT
      : vkGetInstanceProcAddr(vkGetInstanceProcAddr_)
      , vkGetDeviceProcAddr(vkGetDeviceProcAddr_)
      , vkGetPhysicalDeviceProperties(vkGetPhysicalDeviceProperties_)
      , vkGetPhysicalDeviceMemoryProperties(vkGetPhysicalDeviceMemoryProperties_)
      , vkAllocateMemory(vkAllocateMemory_)
      , vkFreeMemory(vkFreeMemory_)
      , vkMapMemory(vkMapMemory_)
      , vkUnmapMemory(vkUnmapMemory_)
      , vkFlushMappedMemoryRanges(vkFlushMappedMemoryRanges_)
      , vkInvalidateMappedMemoryRanges(vkInvalidateMappedMemoryRanges_)
      , vkBindBufferMemory(vkBindBufferMemory_)
      , vkBindImageMemory(vkBindImageMemory_)
      , vkGetBufferMemoryRequirements(vkGetBufferMemoryRequirements_)
      , vkGetImageMemoryRequirements(vkGetImageMemoryRequirements_)
      , vkCreateBuffer(vkCreateBuffer_)
      , vkDestroyBuffer(vkDestroyBuffer_)
      , vkCreateImage(vkCreateImage_)
      , vkDestroyImage(vkDestroyImage_)
      , vkCmdCopyBuffer(vkCmdCopyBuffer_)
#if VMA_DEDICATED_ALLOCATION || VMA_VULKAN_VERSION >= 1001000
      , vkGetBufferMemoryRequirements2KHR(vkGetBufferMemoryRequirements2KHR_)
      , vkGetImageMemoryRequirements2KHR(vkGetImageMemoryRequirements2KHR_)
#endif
#if VMA_BIND_MEMORY2 || VMA_VULKAN_VERSION >= 1001000
      , vkBindBufferMemory2KHR(vkBindBufferMemory2KHR_)
      , vkBindImageMemory2KHR(vkBindImageMemory2KHR_)
#endif
#if VMA_MEMORY_BUDGET || VMA_VULKAN_VERSION >= 1001000
      , vkGetPhysicalDeviceMemoryProperties2KHR(vkGetPhysicalDeviceMemoryProperties2KHR_)
#endif
#if VMA_VULKAN_VERSION >= 1003000
      , vkGetDeviceBufferMemoryRequirements(vkGetDeviceBufferMemoryRequirements_)
      , vkGetDeviceImageMemoryRequirements(vkGetDeviceImageMemoryRequirements_)
#endif
      {}

    VULKAN_HPP_CONSTEXPR VulkanFunctions(VulkanFunctions const &) VULKAN_HPP_NOEXCEPT = default;
    VulkanFunctions(VmaVulkanFunctions const & rhs) VULKAN_HPP_NOEXCEPT : VulkanFunctions(*reinterpret_cast<VulkanFunctions const *>(&rhs)) {}
#endif

    VulkanFunctions& operator=(VulkanFunctions const &) VULKAN_HPP_NOEXCEPT = default;
    VulkanFunctions& operator=(VmaVulkanFunctions const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::VulkanFunctions const *>(&rhs);
      return *this;
    }

    explicit operator VmaVulkanFunctions const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaVulkanFunctions *>(this);
    }

    explicit operator VmaVulkanFunctions&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaVulkanFunctions *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(VulkanFunctions const &) const = default;
#else
    bool operator==(VulkanFunctions const & rhs) const VULKAN_HPP_NOEXCEPT {
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
#if VMA_VULKAN_VERSION >= 1003000
          && vkGetDeviceBufferMemoryRequirements == rhs.vkGetDeviceBufferMemoryRequirements
          && vkGetDeviceImageMemoryRequirements == rhs.vkGetDeviceImageMemoryRequirements
#endif
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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
#if VMA_VULKAN_VERSION >= 1003000

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetDeviceBufferMemoryRequirements(PFN_vkGetDeviceBufferMemoryRequirements vkGetDeviceBufferMemoryRequirements_) VULKAN_HPP_NOEXCEPT {
      vkGetDeviceBufferMemoryRequirements = vkGetDeviceBufferMemoryRequirements_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 VulkanFunctions& setVkGetDeviceImageMemoryRequirements(PFN_vkGetDeviceImageMemoryRequirements vkGetDeviceImageMemoryRequirements_) VULKAN_HPP_NOEXCEPT {
      vkGetDeviceImageMemoryRequirements = vkGetDeviceImageMemoryRequirements_;
      return *this;
    }
#endif
#endif

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
#if VMA_VULKAN_VERSION >= 1003000
    PFN_vkGetDeviceBufferMemoryRequirements vkGetDeviceBufferMemoryRequirements = {};
    PFN_vkGetDeviceImageMemoryRequirements vkGetDeviceImageMemoryRequirements = {};
#endif
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(VulkanFunctions) == sizeof(VmaVulkanFunctions),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<VulkanFunctions>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<VulkanFunctions>::value,
                           "VulkanFunctions is not nothrow_move_constructible!");

  struct AllocatorCreateInfo {
    using NativeType = VmaAllocatorCreateInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
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
      : flags(flags_)
      , physicalDevice(physicalDevice_)
      , device(device_)
      , preferredLargeHeapBlockSize(preferredLargeHeapBlockSize_)
      , pAllocationCallbacks(pAllocationCallbacks_)
      , pDeviceMemoryCallbacks(pDeviceMemoryCallbacks_)
      , pHeapSizeLimit(pHeapSizeLimit_)
      , pVulkanFunctions(pVulkanFunctions_)
      , instance(instance_)
      , vulkanApiVersion(vulkanApiVersion_)
#if VMA_EXTERNAL_MEMORY
      , pTypeExternalMemoryHandleTypes(pTypeExternalMemoryHandleTypes_)
#endif
      {}

    VULKAN_HPP_CONSTEXPR AllocatorCreateInfo(AllocatorCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocatorCreateInfo(VmaAllocatorCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : AllocatorCreateInfo(*reinterpret_cast<AllocatorCreateInfo const *>(&rhs)) {}
#endif

    AllocatorCreateInfo& operator=(AllocatorCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocatorCreateInfo& operator=(VmaAllocatorCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::AllocatorCreateInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaAllocatorCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaAllocatorCreateInfo *>(this);
    }

    explicit operator VmaAllocatorCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaAllocatorCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(AllocatorCreateInfo const &) const = default;
#else
    bool operator==(AllocatorCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
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
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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
  VULKAN_HPP_STATIC_ASSERT(sizeof(AllocatorCreateInfo) == sizeof(VmaAllocatorCreateInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<AllocatorCreateInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<AllocatorCreateInfo>::value,
                           "AllocatorCreateInfo is not nothrow_move_constructible!");

  struct AllocatorInfo {
    using NativeType = VmaAllocatorInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR AllocatorInfo(
          VULKAN_HPP_NAMESPACE::Instance instance_ = {}
        , VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice_ = {}
        , VULKAN_HPP_NAMESPACE::Device device_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : instance(instance_)
      , physicalDevice(physicalDevice_)
      , device(device_)
      {}

    VULKAN_HPP_CONSTEXPR AllocatorInfo(AllocatorInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocatorInfo(VmaAllocatorInfo const & rhs) VULKAN_HPP_NOEXCEPT : AllocatorInfo(*reinterpret_cast<AllocatorInfo const *>(&rhs)) {}
#endif

    AllocatorInfo& operator=(AllocatorInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocatorInfo& operator=(VmaAllocatorInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::AllocatorInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaAllocatorInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaAllocatorInfo *>(this);
    }

    explicit operator VmaAllocatorInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaAllocatorInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(AllocatorInfo const &) const = default;
#else
    bool operator==(AllocatorInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
      return instance == rhs.instance
          && physicalDevice == rhs.physicalDevice
          && device == rhs.device
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    VULKAN_HPP_NAMESPACE::Instance instance = {};
    VULKAN_HPP_NAMESPACE::PhysicalDevice physicalDevice = {};
    VULKAN_HPP_NAMESPACE::Device device = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(AllocatorInfo) == sizeof(VmaAllocatorInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<AllocatorInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<AllocatorInfo>::value,
                           "AllocatorInfo is not nothrow_move_constructible!");

  struct Statistics {
    using NativeType = VmaStatistics;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR Statistics(
          uint32_t blockCount_ = {}
        , uint32_t allocationCount_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize blockBytes_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize allocationBytes_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : blockCount(blockCount_)
      , allocationCount(allocationCount_)
      , blockBytes(blockBytes_)
      , allocationBytes(allocationBytes_)
      {}

    VULKAN_HPP_CONSTEXPR Statistics(Statistics const &) VULKAN_HPP_NOEXCEPT = default;
    Statistics(VmaStatistics const & rhs) VULKAN_HPP_NOEXCEPT : Statistics(*reinterpret_cast<Statistics const *>(&rhs)) {}
#endif

    Statistics& operator=(Statistics const &) VULKAN_HPP_NOEXCEPT = default;
    Statistics& operator=(VmaStatistics const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::Statistics const *>(&rhs);
      return *this;
    }

    explicit operator VmaStatistics const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaStatistics *>(this);
    }

    explicit operator VmaStatistics&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaStatistics *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(Statistics const &) const = default;
#else
    bool operator==(Statistics const & rhs) const VULKAN_HPP_NOEXCEPT {
      return blockCount == rhs.blockCount
          && allocationCount == rhs.allocationCount
          && blockBytes == rhs.blockBytes
          && allocationBytes == rhs.allocationBytes
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    uint32_t blockCount = {};
    uint32_t allocationCount = {};
    VULKAN_HPP_NAMESPACE::DeviceSize blockBytes = {};
    VULKAN_HPP_NAMESPACE::DeviceSize allocationBytes = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(Statistics) == sizeof(VmaStatistics),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<Statistics>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<Statistics>::value,
                           "Statistics is not nothrow_move_constructible!");

  struct DetailedStatistics {
    using NativeType = VmaDetailedStatistics;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DetailedStatistics(
          Statistics statistics_ = {}
        , uint32_t unusedRangeCount_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMin_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMax_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMin_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMax_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : statistics(statistics_)
      , unusedRangeCount(unusedRangeCount_)
      , allocationSizeMin(allocationSizeMin_)
      , allocationSizeMax(allocationSizeMax_)
      , unusedRangeSizeMin(unusedRangeSizeMin_)
      , unusedRangeSizeMax(unusedRangeSizeMax_)
      {}

    VULKAN_HPP_CONSTEXPR DetailedStatistics(DetailedStatistics const &) VULKAN_HPP_NOEXCEPT = default;
    DetailedStatistics(VmaDetailedStatistics const & rhs) VULKAN_HPP_NOEXCEPT : DetailedStatistics(*reinterpret_cast<DetailedStatistics const *>(&rhs)) {}
#endif

    DetailedStatistics& operator=(DetailedStatistics const &) VULKAN_HPP_NOEXCEPT = default;
    DetailedStatistics& operator=(VmaDetailedStatistics const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DetailedStatistics const *>(&rhs);
      return *this;
    }

    explicit operator VmaDetailedStatistics const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDetailedStatistics *>(this);
    }

    explicit operator VmaDetailedStatistics&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDetailedStatistics *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(DetailedStatistics const &) const = default;
#else
    bool operator==(DetailedStatistics const & rhs) const VULKAN_HPP_NOEXCEPT {
      return statistics == rhs.statistics
          && unusedRangeCount == rhs.unusedRangeCount
          && allocationSizeMin == rhs.allocationSizeMin
          && allocationSizeMax == rhs.allocationSizeMax
          && unusedRangeSizeMin == rhs.unusedRangeSizeMin
          && unusedRangeSizeMax == rhs.unusedRangeSizeMax
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    Statistics statistics = {};
    uint32_t unusedRangeCount = {};
    VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMin = {};
    VULKAN_HPP_NAMESPACE::DeviceSize allocationSizeMax = {};
    VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMin = {};
    VULKAN_HPP_NAMESPACE::DeviceSize unusedRangeSizeMax = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(DetailedStatistics) == sizeof(VmaDetailedStatistics),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<DetailedStatistics>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<DetailedStatistics>::value,
                           "DetailedStatistics is not nothrow_move_constructible!");

  struct TotalStatistics {
    using NativeType = VmaTotalStatistics;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR TotalStatistics(
          std::array<DetailedStatistics, VK_MAX_MEMORY_TYPES> memoryType_ = {}
        , std::array<DetailedStatistics, VK_MAX_MEMORY_HEAPS> memoryHeap_ = {}
        , DetailedStatistics total_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : memoryType(memoryType_)
      , memoryHeap(memoryHeap_)
      , total(total_)
      {}

    VULKAN_HPP_CONSTEXPR TotalStatistics(TotalStatistics const &) VULKAN_HPP_NOEXCEPT = default;
    TotalStatistics(VmaTotalStatistics const & rhs) VULKAN_HPP_NOEXCEPT : TotalStatistics(*reinterpret_cast<TotalStatistics const *>(&rhs)) {}
#endif

    TotalStatistics& operator=(TotalStatistics const &) VULKAN_HPP_NOEXCEPT = default;
    TotalStatistics& operator=(VmaTotalStatistics const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::TotalStatistics const *>(&rhs);
      return *this;
    }

    explicit operator VmaTotalStatistics const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaTotalStatistics *>(this);
    }

    explicit operator VmaTotalStatistics&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaTotalStatistics *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(TotalStatistics const &) const = default;
#else
    bool operator==(TotalStatistics const & rhs) const VULKAN_HPP_NOEXCEPT {
      return memoryType == rhs.memoryType
          && memoryHeap == rhs.memoryHeap
          && total == rhs.total
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

    VULKAN_HPP_CONSTEXPR_14 TotalStatistics& setMemoryType(std::array<DetailedStatistics, VK_MAX_MEMORY_TYPES> memoryType_) VULKAN_HPP_NOEXCEPT {
      memoryType = memoryType_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 TotalStatistics& setMemoryHeap(std::array<DetailedStatistics, VK_MAX_MEMORY_HEAPS> memoryHeap_) VULKAN_HPP_NOEXCEPT {
      memoryHeap = memoryHeap_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 TotalStatistics& setTotal(DetailedStatistics total_) VULKAN_HPP_NOEXCEPT {
      total = total_;
      return *this;
    }
#endif

  public:
    std::array<DetailedStatistics, VK_MAX_MEMORY_TYPES> memoryType = {};
    std::array<DetailedStatistics, VK_MAX_MEMORY_HEAPS> memoryHeap = {};
    DetailedStatistics total = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(TotalStatistics) == sizeof(VmaTotalStatistics),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<TotalStatistics>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<TotalStatistics>::value,
                           "TotalStatistics is not nothrow_move_constructible!");

  struct Budget {
    using NativeType = VmaBudget;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR Budget(
          Statistics statistics_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize usage_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize budget_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : statistics(statistics_)
      , usage(usage_)
      , budget(budget_)
      {}

    VULKAN_HPP_CONSTEXPR Budget(Budget const &) VULKAN_HPP_NOEXCEPT = default;
    Budget(VmaBudget const & rhs) VULKAN_HPP_NOEXCEPT : Budget(*reinterpret_cast<Budget const *>(&rhs)) {}
#endif

    Budget& operator=(Budget const &) VULKAN_HPP_NOEXCEPT = default;
    Budget& operator=(VmaBudget const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::Budget const *>(&rhs);
      return *this;
    }

    explicit operator VmaBudget const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaBudget *>(this);
    }

    explicit operator VmaBudget&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaBudget *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(Budget const &) const = default;
#else
    bool operator==(Budget const & rhs) const VULKAN_HPP_NOEXCEPT {
      return statistics == rhs.statistics
          && usage == rhs.usage
          && budget == rhs.budget
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    Statistics statistics = {};
    VULKAN_HPP_NAMESPACE::DeviceSize usage = {};
    VULKAN_HPP_NAMESPACE::DeviceSize budget = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(Budget) == sizeof(VmaBudget),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<Budget>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<Budget>::value,
                           "Budget is not nothrow_move_constructible!");

  struct AllocationCreateInfo {
    using NativeType = VmaAllocationCreateInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
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
      : flags(flags_)
      , usage(usage_)
      , requiredFlags(requiredFlags_)
      , preferredFlags(preferredFlags_)
      , memoryTypeBits(memoryTypeBits_)
      , pool(pool_)
      , pUserData(pUserData_)
      , priority(priority_)
      {}

    VULKAN_HPP_CONSTEXPR AllocationCreateInfo(AllocationCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocationCreateInfo(VmaAllocationCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : AllocationCreateInfo(*reinterpret_cast<AllocationCreateInfo const *>(&rhs)) {}
#endif

    AllocationCreateInfo& operator=(AllocationCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocationCreateInfo& operator=(VmaAllocationCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::AllocationCreateInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaAllocationCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaAllocationCreateInfo *>(this);
    }

    explicit operator VmaAllocationCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaAllocationCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(AllocationCreateInfo const &) const = default;
#else
    bool operator==(AllocationCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
      return flags == rhs.flags
          && usage == rhs.usage
          && requiredFlags == rhs.requiredFlags
          && preferredFlags == rhs.preferredFlags
          && memoryTypeBits == rhs.memoryTypeBits
          && pool == rhs.pool
          && pUserData == rhs.pUserData
          && priority == rhs.priority
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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
  VULKAN_HPP_STATIC_ASSERT(sizeof(AllocationCreateInfo) == sizeof(VmaAllocationCreateInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<AllocationCreateInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<AllocationCreateInfo>::value,
                           "AllocationCreateInfo is not nothrow_move_constructible!");

  struct PoolCreateInfo {
    using NativeType = VmaPoolCreateInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
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
      : memoryTypeIndex(memoryTypeIndex_)
      , flags(flags_)
      , blockSize(blockSize_)
      , minBlockCount(minBlockCount_)
      , maxBlockCount(maxBlockCount_)
      , priority(priority_)
      , minAllocationAlignment(minAllocationAlignment_)
      , pMemoryAllocateNext(pMemoryAllocateNext_)
      {}

    VULKAN_HPP_CONSTEXPR PoolCreateInfo(PoolCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    PoolCreateInfo(VmaPoolCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : PoolCreateInfo(*reinterpret_cast<PoolCreateInfo const *>(&rhs)) {}
#endif

    PoolCreateInfo& operator=(PoolCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    PoolCreateInfo& operator=(VmaPoolCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::PoolCreateInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaPoolCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaPoolCreateInfo *>(this);
    }

    explicit operator VmaPoolCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaPoolCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(PoolCreateInfo const &) const = default;
#else
    bool operator==(PoolCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
      return memoryTypeIndex == rhs.memoryTypeIndex
          && flags == rhs.flags
          && blockSize == rhs.blockSize
          && minBlockCount == rhs.minBlockCount
          && maxBlockCount == rhs.maxBlockCount
          && priority == rhs.priority
          && minAllocationAlignment == rhs.minAllocationAlignment
          && pMemoryAllocateNext == rhs.pMemoryAllocateNext
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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
  VULKAN_HPP_STATIC_ASSERT(sizeof(PoolCreateInfo) == sizeof(VmaPoolCreateInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<PoolCreateInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<PoolCreateInfo>::value,
                           "PoolCreateInfo is not nothrow_move_constructible!");

  struct AllocationInfo {
    using NativeType = VmaAllocationInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR AllocationInfo(
          uint32_t memoryType_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceMemory deviceMemory_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize offset_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize size_ = {}
        , void* pMappedData_ = {}
        , void* pUserData_ = {}
        , const char* pName_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : memoryType(memoryType_)
      , deviceMemory(deviceMemory_)
      , offset(offset_)
      , size(size_)
      , pMappedData(pMappedData_)
      , pUserData(pUserData_)
      , pName(pName_)
      {}

    VULKAN_HPP_CONSTEXPR AllocationInfo(AllocationInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocationInfo(VmaAllocationInfo const & rhs) VULKAN_HPP_NOEXCEPT : AllocationInfo(*reinterpret_cast<AllocationInfo const *>(&rhs)) {}
#endif

    AllocationInfo& operator=(AllocationInfo const &) VULKAN_HPP_NOEXCEPT = default;
    AllocationInfo& operator=(VmaAllocationInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::AllocationInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaAllocationInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaAllocationInfo *>(this);
    }

    explicit operator VmaAllocationInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaAllocationInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(AllocationInfo const &) const = default;
#else
    bool operator==(AllocationInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
      return memoryType == rhs.memoryType
          && deviceMemory == rhs.deviceMemory
          && offset == rhs.offset
          && size == rhs.size
          && pMappedData == rhs.pMappedData
          && pUserData == rhs.pUserData
          && pName == rhs.pName
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    uint32_t memoryType = {};
    VULKAN_HPP_NAMESPACE::DeviceMemory deviceMemory = {};
    VULKAN_HPP_NAMESPACE::DeviceSize offset = {};
    VULKAN_HPP_NAMESPACE::DeviceSize size = {};
    void* pMappedData = {};
    void* pUserData = {};
    const char* pName = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(AllocationInfo) == sizeof(VmaAllocationInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<AllocationInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<AllocationInfo>::value,
                           "AllocationInfo is not nothrow_move_constructible!");

  struct DefragmentationInfo {
    using NativeType = VmaDefragmentationInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DefragmentationInfo(
          DefragmentationFlags flags_ = {}
        , Pool pool_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize maxBytesPerPass_ = {}
        , uint32_t maxAllocationsPerPass_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : flags(flags_)
      , pool(pool_)
      , maxBytesPerPass(maxBytesPerPass_)
      , maxAllocationsPerPass(maxAllocationsPerPass_)
      {}

    VULKAN_HPP_CONSTEXPR DefragmentationInfo(DefragmentationInfo const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationInfo(VmaDefragmentationInfo const & rhs) VULKAN_HPP_NOEXCEPT : DefragmentationInfo(*reinterpret_cast<DefragmentationInfo const *>(&rhs)) {}
#endif

    DefragmentationInfo& operator=(DefragmentationInfo const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationInfo& operator=(VmaDefragmentationInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DefragmentationInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaDefragmentationInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDefragmentationInfo *>(this);
    }

    explicit operator VmaDefragmentationInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDefragmentationInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(DefragmentationInfo const &) const = default;
#else
    bool operator==(DefragmentationInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
      return flags == rhs.flags
          && pool == rhs.pool
          && maxBytesPerPass == rhs.maxBytesPerPass
          && maxAllocationsPerPass == rhs.maxAllocationsPerPass
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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
#endif

  public:
    DefragmentationFlags flags = {};
    Pool pool = {};
    VULKAN_HPP_NAMESPACE::DeviceSize maxBytesPerPass = {};
    uint32_t maxAllocationsPerPass = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(DefragmentationInfo) == sizeof(VmaDefragmentationInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<DefragmentationInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<DefragmentationInfo>::value,
                           "DefragmentationInfo is not nothrow_move_constructible!");

  struct DefragmentationMove {
    using NativeType = VmaDefragmentationMove;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DefragmentationMove(
          DefragmentationMoveOperation operation_ = {}
        , Allocation srcAllocation_ = {}
        , Allocation dstTmpAllocation_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : operation(operation_)
      , srcAllocation(srcAllocation_)
      , dstTmpAllocation(dstTmpAllocation_)
      {}

    VULKAN_HPP_CONSTEXPR DefragmentationMove(DefragmentationMove const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationMove(VmaDefragmentationMove const & rhs) VULKAN_HPP_NOEXCEPT : DefragmentationMove(*reinterpret_cast<DefragmentationMove const *>(&rhs)) {}
#endif

    DefragmentationMove& operator=(DefragmentationMove const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationMove& operator=(VmaDefragmentationMove const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DefragmentationMove const *>(&rhs);
      return *this;
    }

    explicit operator VmaDefragmentationMove const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDefragmentationMove *>(this);
    }

    explicit operator VmaDefragmentationMove&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDefragmentationMove *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(DefragmentationMove const &) const = default;
#else
    bool operator==(DefragmentationMove const & rhs) const VULKAN_HPP_NOEXCEPT {
      return operation == rhs.operation
          && srcAllocation == rhs.srcAllocation
          && dstTmpAllocation == rhs.dstTmpAllocation
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    DefragmentationMoveOperation operation = {};
    Allocation srcAllocation = {};
    Allocation dstTmpAllocation = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(DefragmentationMove) == sizeof(VmaDefragmentationMove),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<DefragmentationMove>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<DefragmentationMove>::value,
                           "DefragmentationMove is not nothrow_move_constructible!");

  struct DefragmentationPassMoveInfo {
    using NativeType = VmaDefragmentationPassMoveInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DefragmentationPassMoveInfo(
          uint32_t moveCount_ = {}
        , DefragmentationMove* pMoves_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : moveCount(moveCount_)
      , pMoves(pMoves_)
      {}

    VULKAN_HPP_CONSTEXPR DefragmentationPassMoveInfo(DefragmentationPassMoveInfo const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationPassMoveInfo(VmaDefragmentationPassMoveInfo const & rhs) VULKAN_HPP_NOEXCEPT : DefragmentationPassMoveInfo(*reinterpret_cast<DefragmentationPassMoveInfo const *>(&rhs)) {}
#endif

    DefragmentationPassMoveInfo& operator=(DefragmentationPassMoveInfo const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationPassMoveInfo& operator=(VmaDefragmentationPassMoveInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DefragmentationPassMoveInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaDefragmentationPassMoveInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDefragmentationPassMoveInfo *>(this);
    }

    explicit operator VmaDefragmentationPassMoveInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDefragmentationPassMoveInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(DefragmentationPassMoveInfo const &) const = default;
#else
    bool operator==(DefragmentationPassMoveInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
      return moveCount == rhs.moveCount
          && pMoves == rhs.pMoves
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

    VULKAN_HPP_CONSTEXPR_14 DefragmentationPassMoveInfo& setMoveCount(uint32_t moveCount_) VULKAN_HPP_NOEXCEPT {
      moveCount = moveCount_;
      return *this;
    }

    VULKAN_HPP_CONSTEXPR_14 DefragmentationPassMoveInfo& setPMoves(DefragmentationMove* pMoves_) VULKAN_HPP_NOEXCEPT {
      pMoves = pMoves_;
      return *this;
    }
#endif

  public:
    uint32_t moveCount = {};
    DefragmentationMove* pMoves = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(DefragmentationPassMoveInfo) == sizeof(VmaDefragmentationPassMoveInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<DefragmentationPassMoveInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<DefragmentationPassMoveInfo>::value,
                           "DefragmentationPassMoveInfo is not nothrow_move_constructible!");

  struct DefragmentationStats {
    using NativeType = VmaDefragmentationStats;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR DefragmentationStats(
          VULKAN_HPP_NAMESPACE::DeviceSize bytesMoved_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize bytesFreed_ = {}
        , uint32_t allocationsMoved_ = {}
        , uint32_t deviceMemoryBlocksFreed_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : bytesMoved(bytesMoved_)
      , bytesFreed(bytesFreed_)
      , allocationsMoved(allocationsMoved_)
      , deviceMemoryBlocksFreed(deviceMemoryBlocksFreed_)
      {}

    VULKAN_HPP_CONSTEXPR DefragmentationStats(DefragmentationStats const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationStats(VmaDefragmentationStats const & rhs) VULKAN_HPP_NOEXCEPT : DefragmentationStats(*reinterpret_cast<DefragmentationStats const *>(&rhs)) {}
#endif

    DefragmentationStats& operator=(DefragmentationStats const &) VULKAN_HPP_NOEXCEPT = default;
    DefragmentationStats& operator=(VmaDefragmentationStats const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::DefragmentationStats const *>(&rhs);
      return *this;
    }

    explicit operator VmaDefragmentationStats const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaDefragmentationStats *>(this);
    }

    explicit operator VmaDefragmentationStats&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaDefragmentationStats *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(DefragmentationStats const &) const = default;
#else
    bool operator==(DefragmentationStats const & rhs) const VULKAN_HPP_NOEXCEPT {
      return bytesMoved == rhs.bytesMoved
          && bytesFreed == rhs.bytesFreed
          && allocationsMoved == rhs.allocationsMoved
          && deviceMemoryBlocksFreed == rhs.deviceMemoryBlocksFreed
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    VULKAN_HPP_NAMESPACE::DeviceSize bytesMoved = {};
    VULKAN_HPP_NAMESPACE::DeviceSize bytesFreed = {};
    uint32_t allocationsMoved = {};
    uint32_t deviceMemoryBlocksFreed = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(DefragmentationStats) == sizeof(VmaDefragmentationStats),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<DefragmentationStats>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<DefragmentationStats>::value,
                           "DefragmentationStats is not nothrow_move_constructible!");

  struct VirtualBlockCreateInfo {
    using NativeType = VmaVirtualBlockCreateInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR VirtualBlockCreateInfo(
          VULKAN_HPP_NAMESPACE::DeviceSize size_ = {}
        , VirtualBlockCreateFlags flags_ = {}
        , const VULKAN_HPP_NAMESPACE::AllocationCallbacks* pAllocationCallbacks_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : size(size_)
      , flags(flags_)
      , pAllocationCallbacks(pAllocationCallbacks_)
      {}

    VULKAN_HPP_CONSTEXPR VirtualBlockCreateInfo(VirtualBlockCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    VirtualBlockCreateInfo(VmaVirtualBlockCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : VirtualBlockCreateInfo(*reinterpret_cast<VirtualBlockCreateInfo const *>(&rhs)) {}
#endif

    VirtualBlockCreateInfo& operator=(VirtualBlockCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    VirtualBlockCreateInfo& operator=(VmaVirtualBlockCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::VirtualBlockCreateInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaVirtualBlockCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaVirtualBlockCreateInfo *>(this);
    }

    explicit operator VmaVirtualBlockCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaVirtualBlockCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(VirtualBlockCreateInfo const &) const = default;
#else
    bool operator==(VirtualBlockCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
      return size == rhs.size
          && flags == rhs.flags
          && pAllocationCallbacks == rhs.pAllocationCallbacks
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    VULKAN_HPP_NAMESPACE::DeviceSize size = {};
    VirtualBlockCreateFlags flags = {};
    const VULKAN_HPP_NAMESPACE::AllocationCallbacks* pAllocationCallbacks = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(VirtualBlockCreateInfo) == sizeof(VmaVirtualBlockCreateInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<VirtualBlockCreateInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<VirtualBlockCreateInfo>::value,
                           "VirtualBlockCreateInfo is not nothrow_move_constructible!");

  struct VirtualAllocationCreateInfo {
    using NativeType = VmaVirtualAllocationCreateInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR VirtualAllocationCreateInfo(
          VULKAN_HPP_NAMESPACE::DeviceSize size_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize alignment_ = {}
        , VirtualAllocationCreateFlags flags_ = {}
        , void* pUserData_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : size(size_)
      , alignment(alignment_)
      , flags(flags_)
      , pUserData(pUserData_)
      {}

    VULKAN_HPP_CONSTEXPR VirtualAllocationCreateInfo(VirtualAllocationCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    VirtualAllocationCreateInfo(VmaVirtualAllocationCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT : VirtualAllocationCreateInfo(*reinterpret_cast<VirtualAllocationCreateInfo const *>(&rhs)) {}
#endif

    VirtualAllocationCreateInfo& operator=(VirtualAllocationCreateInfo const &) VULKAN_HPP_NOEXCEPT = default;
    VirtualAllocationCreateInfo& operator=(VmaVirtualAllocationCreateInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::VirtualAllocationCreateInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaVirtualAllocationCreateInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaVirtualAllocationCreateInfo *>(this);
    }

    explicit operator VmaVirtualAllocationCreateInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaVirtualAllocationCreateInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(VirtualAllocationCreateInfo const &) const = default;
#else
    bool operator==(VirtualAllocationCreateInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
      return size == rhs.size
          && alignment == rhs.alignment
          && flags == rhs.flags
          && pUserData == rhs.pUserData
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    VULKAN_HPP_NAMESPACE::DeviceSize size = {};
    VULKAN_HPP_NAMESPACE::DeviceSize alignment = {};
    VirtualAllocationCreateFlags flags = {};
    void* pUserData = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(VirtualAllocationCreateInfo) == sizeof(VmaVirtualAllocationCreateInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<VirtualAllocationCreateInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<VirtualAllocationCreateInfo>::value,
                           "VirtualAllocationCreateInfo is not nothrow_move_constructible!");

  struct VirtualAllocationInfo {
    using NativeType = VmaVirtualAllocationInfo;

#if !defined( VULKAN_HPP_NO_STRUCT_CONSTRUCTORS )
    VULKAN_HPP_CONSTEXPR VirtualAllocationInfo(
          VULKAN_HPP_NAMESPACE::DeviceSize offset_ = {}
        , VULKAN_HPP_NAMESPACE::DeviceSize size_ = {}
        , void* pUserData_ = {}
      ) VULKAN_HPP_NOEXCEPT
      : offset(offset_)
      , size(size_)
      , pUserData(pUserData_)
      {}

    VULKAN_HPP_CONSTEXPR VirtualAllocationInfo(VirtualAllocationInfo const &) VULKAN_HPP_NOEXCEPT = default;
    VirtualAllocationInfo(VmaVirtualAllocationInfo const & rhs) VULKAN_HPP_NOEXCEPT : VirtualAllocationInfo(*reinterpret_cast<VirtualAllocationInfo const *>(&rhs)) {}
#endif

    VirtualAllocationInfo& operator=(VirtualAllocationInfo const &) VULKAN_HPP_NOEXCEPT = default;
    VirtualAllocationInfo& operator=(VmaVirtualAllocationInfo const & rhs) VULKAN_HPP_NOEXCEPT {
      *this = *reinterpret_cast<VMA_HPP_NAMESPACE::VirtualAllocationInfo const *>(&rhs);
      return *this;
    }

    explicit operator VmaVirtualAllocationInfo const &() const VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<const VmaVirtualAllocationInfo *>(this);
    }

    explicit operator VmaVirtualAllocationInfo&() VULKAN_HPP_NOEXCEPT {
      return *reinterpret_cast<VmaVirtualAllocationInfo *>(this);
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    bool operator==(VirtualAllocationInfo const &) const = default;
#else
    bool operator==(VirtualAllocationInfo const & rhs) const VULKAN_HPP_NOEXCEPT {
      return offset == rhs.offset
          && size == rhs.size
          && pUserData == rhs.pUserData
      ;
    }
#endif

#if !defined( VULKAN_HPP_NO_STRUCT_SETTERS )

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

  public:
    VULKAN_HPP_NAMESPACE::DeviceSize offset = {};
    VULKAN_HPP_NAMESPACE::DeviceSize size = {};
    void* pUserData = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(VirtualAllocationInfo) == sizeof(VmaVirtualAllocationInfo),
                           "struct and wrapper have different size!");
  VULKAN_HPP_STATIC_ASSERT(std::is_standard_layout<VirtualAllocationInfo>::value,
                           "struct wrapper is not a standard layout!");
  VULKAN_HPP_STATIC_ASSERT(std::is_nothrow_move_constructible<VirtualAllocationInfo>::value,
                           "VirtualAllocationInfo is not nothrow_move_constructible!");
}
#endif
