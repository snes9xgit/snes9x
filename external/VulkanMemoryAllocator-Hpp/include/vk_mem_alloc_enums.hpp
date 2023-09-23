#ifndef VULKAN_MEMORY_ALLOCATOR_ENUMS_HPP
#define VULKAN_MEMORY_ALLOCATOR_ENUMS_HPP

namespace VMA_HPP_NAMESPACE {

  enum class AllocatorCreateFlagBits : VmaAllocatorCreateFlags {
    eExternallySynchronized = VMA_ALLOCATOR_CREATE_EXTERNALLY_SYNCHRONIZED_BIT,
    eKhrDedicatedAllocation = VMA_ALLOCATOR_CREATE_KHR_DEDICATED_ALLOCATION_BIT,
    eKhrBindMemory2 = VMA_ALLOCATOR_CREATE_KHR_BIND_MEMORY2_BIT,
    eExtMemoryBudget = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT,
    eAmdDeviceCoherentMemory = VMA_ALLOCATOR_CREATE_AMD_DEVICE_COHERENT_MEMORY_BIT,
    eBufferDeviceAddress = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
    eExtMemoryPriority = VMA_ALLOCATOR_CREATE_EXT_MEMORY_PRIORITY_BIT
  };

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(AllocatorCreateFlagBits value) {
    if (value == AllocatorCreateFlagBits::eExternallySynchronized) return "ExternallySynchronized";
    if (value == AllocatorCreateFlagBits::eKhrDedicatedAllocation) return "KhrDedicatedAllocation";
    if (value == AllocatorCreateFlagBits::eKhrBindMemory2) return "KhrBindMemory2";
    if (value == AllocatorCreateFlagBits::eExtMemoryBudget) return "ExtMemoryBudget";
    if (value == AllocatorCreateFlagBits::eAmdDeviceCoherentMemory) return "AmdDeviceCoherentMemory";
    if (value == AllocatorCreateFlagBits::eBufferDeviceAddress) return "BufferDeviceAddress";
    if (value == AllocatorCreateFlagBits::eExtMemoryPriority) return "ExtMemoryPriority";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }
# endif
}

namespace VULKAN_HPP_NAMESPACE {
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::AllocatorCreateFlagBits> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
    static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<VMA_HPP_NAMESPACE::AllocatorCreateFlagBits> allFlags =
        VMA_HPP_NAMESPACE::AllocatorCreateFlagBits::eExternallySynchronized
      | VMA_HPP_NAMESPACE::AllocatorCreateFlagBits::eKhrDedicatedAllocation
      | VMA_HPP_NAMESPACE::AllocatorCreateFlagBits::eKhrBindMemory2
      | VMA_HPP_NAMESPACE::AllocatorCreateFlagBits::eExtMemoryBudget
      | VMA_HPP_NAMESPACE::AllocatorCreateFlagBits::eAmdDeviceCoherentMemory
      | VMA_HPP_NAMESPACE::AllocatorCreateFlagBits::eBufferDeviceAddress
      | VMA_HPP_NAMESPACE::AllocatorCreateFlagBits::eExtMemoryPriority;
  };
}

namespace VMA_HPP_NAMESPACE {

  using AllocatorCreateFlags = VULKAN_HPP_NAMESPACE::Flags<AllocatorCreateFlagBits>;

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR AllocatorCreateFlags operator|(AllocatorCreateFlagBits bit0, AllocatorCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return AllocatorCreateFlags(bit0) | bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR AllocatorCreateFlags operator&(AllocatorCreateFlagBits bit0, AllocatorCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return AllocatorCreateFlags(bit0) & bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR AllocatorCreateFlags operator^(AllocatorCreateFlagBits bit0, AllocatorCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return AllocatorCreateFlags(bit0) ^ bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR AllocatorCreateFlags operator~(AllocatorCreateFlagBits bits) VULKAN_HPP_NOEXCEPT {
    return ~(AllocatorCreateFlags(bits));
  }

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(AllocatorCreateFlags value) {
    if (!value) return "{}";
    std::string result;
    if (value & AllocatorCreateFlagBits::eExternallySynchronized) result += "ExternallySynchronized | ";
    if (value & AllocatorCreateFlagBits::eKhrDedicatedAllocation) result += "KhrDedicatedAllocation | ";
    if (value & AllocatorCreateFlagBits::eKhrBindMemory2) result += "KhrBindMemory2 | ";
    if (value & AllocatorCreateFlagBits::eExtMemoryBudget) result += "ExtMemoryBudget | ";
    if (value & AllocatorCreateFlagBits::eAmdDeviceCoherentMemory) result += "AmdDeviceCoherentMemory | ";
    if (value & AllocatorCreateFlagBits::eBufferDeviceAddress) result += "BufferDeviceAddress | ";
    if (value & AllocatorCreateFlagBits::eExtMemoryPriority) result += "ExtMemoryPriority | ";
    return "{ " + result.substr( 0, result.size() - 3 ) + " }";
  }
# endif
}

namespace VMA_HPP_NAMESPACE {

  enum class MemoryUsage {
    eUnknown = VMA_MEMORY_USAGE_UNKNOWN,
    eGpuOnly = VMA_MEMORY_USAGE_GPU_ONLY,
    eCpuOnly = VMA_MEMORY_USAGE_CPU_ONLY,
    eCpuToGpu = VMA_MEMORY_USAGE_CPU_TO_GPU,
    eGpuToCpu = VMA_MEMORY_USAGE_GPU_TO_CPU,
    eCpuCopy = VMA_MEMORY_USAGE_CPU_COPY,
    eGpuLazilyAllocated = VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED,
    eAuto = VMA_MEMORY_USAGE_AUTO,
    eAutoPreferDevice = VMA_MEMORY_USAGE_AUTO_PREFER_DEVICE,
    eAutoPreferHost = VMA_MEMORY_USAGE_AUTO_PREFER_HOST
  };

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(MemoryUsage value) {
    if (value == MemoryUsage::eUnknown) return "Unknown";
    if (value == MemoryUsage::eGpuOnly) return "GpuOnly";
    if (value == MemoryUsage::eCpuOnly) return "CpuOnly";
    if (value == MemoryUsage::eCpuToGpu) return "CpuToGpu";
    if (value == MemoryUsage::eGpuToCpu) return "GpuToCpu";
    if (value == MemoryUsage::eCpuCopy) return "CpuCopy";
    if (value == MemoryUsage::eGpuLazilyAllocated) return "GpuLazilyAllocated";
    if (value == MemoryUsage::eAuto) return "Auto";
    if (value == MemoryUsage::eAutoPreferDevice) return "AutoPreferDevice";
    if (value == MemoryUsage::eAutoPreferHost) return "AutoPreferHost";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }
# endif
}

namespace VMA_HPP_NAMESPACE {

  enum class AllocationCreateFlagBits : VmaAllocationCreateFlags {
    eDedicatedMemory = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
    eNeverAllocate = VMA_ALLOCATION_CREATE_NEVER_ALLOCATE_BIT,
    eMapped = VMA_ALLOCATION_CREATE_MAPPED_BIT,
    eUserDataCopyString = VMA_ALLOCATION_CREATE_USER_DATA_COPY_STRING_BIT,
    eUpperAddress = VMA_ALLOCATION_CREATE_UPPER_ADDRESS_BIT,
    eDontBind = VMA_ALLOCATION_CREATE_DONT_BIND_BIT,
    eWithinBudget = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT,
    eCanAlias = VMA_ALLOCATION_CREATE_CAN_ALIAS_BIT,
    eHostAccessSequentialWrite = VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT,
    eHostAccessRandom = VMA_ALLOCATION_CREATE_HOST_ACCESS_RANDOM_BIT,
    eHostAccessAllowTransferInstead = VMA_ALLOCATION_CREATE_HOST_ACCESS_ALLOW_TRANSFER_INSTEAD_BIT,
    eStrategyMinMemory = VMA_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT,
    eStrategyMinTime = VMA_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
    eStrategyMinOffset = VMA_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT,
    eStrategyBestFit = VMA_ALLOCATION_CREATE_STRATEGY_BEST_FIT_BIT,
    eStrategyFirstFit = VMA_ALLOCATION_CREATE_STRATEGY_FIRST_FIT_BIT
  };

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(AllocationCreateFlagBits value) {
    if (value == AllocationCreateFlagBits::eDedicatedMemory) return "DedicatedMemory";
    if (value == AllocationCreateFlagBits::eNeverAllocate) return "NeverAllocate";
    if (value == AllocationCreateFlagBits::eMapped) return "Mapped";
    if (value == AllocationCreateFlagBits::eUserDataCopyString) return "UserDataCopyString";
    if (value == AllocationCreateFlagBits::eUpperAddress) return "UpperAddress";
    if (value == AllocationCreateFlagBits::eDontBind) return "DontBind";
    if (value == AllocationCreateFlagBits::eWithinBudget) return "WithinBudget";
    if (value == AllocationCreateFlagBits::eCanAlias) return "CanAlias";
    if (value == AllocationCreateFlagBits::eHostAccessSequentialWrite) return "HostAccessSequentialWrite";
    if (value == AllocationCreateFlagBits::eHostAccessRandom) return "HostAccessRandom";
    if (value == AllocationCreateFlagBits::eHostAccessAllowTransferInstead) return "HostAccessAllowTransferInstead";
    if (value == AllocationCreateFlagBits::eStrategyMinMemory) return "StrategyMinMemory";
    if (value == AllocationCreateFlagBits::eStrategyMinTime) return "StrategyMinTime";
    if (value == AllocationCreateFlagBits::eStrategyMinOffset) return "StrategyMinOffset";
    if (value == AllocationCreateFlagBits::eStrategyBestFit) return "StrategyBestFit";
    if (value == AllocationCreateFlagBits::eStrategyFirstFit) return "StrategyFirstFit";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }
# endif
}

namespace VULKAN_HPP_NAMESPACE {
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::AllocationCreateFlagBits> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
    static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<VMA_HPP_NAMESPACE::AllocationCreateFlagBits> allFlags =
        VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eDedicatedMemory
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eNeverAllocate
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eMapped
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eUserDataCopyString
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eUpperAddress
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eDontBind
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eWithinBudget
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eCanAlias
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessSequentialWrite
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessRandom
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eHostAccessAllowTransferInstead
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eStrategyMinMemory
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eStrategyMinTime
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eStrategyMinOffset
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eStrategyBestFit
      | VMA_HPP_NAMESPACE::AllocationCreateFlagBits::eStrategyFirstFit;
  };
}

namespace VMA_HPP_NAMESPACE {

  using AllocationCreateFlags = VULKAN_HPP_NAMESPACE::Flags<AllocationCreateFlagBits>;

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR AllocationCreateFlags operator|(AllocationCreateFlagBits bit0, AllocationCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return AllocationCreateFlags(bit0) | bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR AllocationCreateFlags operator&(AllocationCreateFlagBits bit0, AllocationCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return AllocationCreateFlags(bit0) & bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR AllocationCreateFlags operator^(AllocationCreateFlagBits bit0, AllocationCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return AllocationCreateFlags(bit0) ^ bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR AllocationCreateFlags operator~(AllocationCreateFlagBits bits) VULKAN_HPP_NOEXCEPT {
    return ~(AllocationCreateFlags(bits));
  }

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(AllocationCreateFlags value) {
    if (!value) return "{}";
    std::string result;
    if (value & AllocationCreateFlagBits::eDedicatedMemory) result += "DedicatedMemory | ";
    if (value & AllocationCreateFlagBits::eNeverAllocate) result += "NeverAllocate | ";
    if (value & AllocationCreateFlagBits::eMapped) result += "Mapped | ";
    if (value & AllocationCreateFlagBits::eUserDataCopyString) result += "UserDataCopyString | ";
    if (value & AllocationCreateFlagBits::eUpperAddress) result += "UpperAddress | ";
    if (value & AllocationCreateFlagBits::eDontBind) result += "DontBind | ";
    if (value & AllocationCreateFlagBits::eWithinBudget) result += "WithinBudget | ";
    if (value & AllocationCreateFlagBits::eCanAlias) result += "CanAlias | ";
    if (value & AllocationCreateFlagBits::eHostAccessSequentialWrite) result += "HostAccessSequentialWrite | ";
    if (value & AllocationCreateFlagBits::eHostAccessRandom) result += "HostAccessRandom | ";
    if (value & AllocationCreateFlagBits::eHostAccessAllowTransferInstead) result += "HostAccessAllowTransferInstead | ";
    if (value & AllocationCreateFlagBits::eStrategyMinMemory) result += "StrategyMinMemory | ";
    if (value & AllocationCreateFlagBits::eStrategyMinTime) result += "StrategyMinTime | ";
    if (value & AllocationCreateFlagBits::eStrategyMinOffset) result += "StrategyMinOffset | ";
    if (value & AllocationCreateFlagBits::eStrategyBestFit) result += "StrategyBestFit | ";
    if (value & AllocationCreateFlagBits::eStrategyFirstFit) result += "StrategyFirstFit | ";
    return "{ " + result.substr( 0, result.size() - 3 ) + " }";
  }
# endif
}

namespace VMA_HPP_NAMESPACE {

  enum class PoolCreateFlagBits : VmaPoolCreateFlags {
    eIgnoreBufferImageGranularity = VMA_POOL_CREATE_IGNORE_BUFFER_IMAGE_GRANULARITY_BIT,
    eLinearAlgorithm = VMA_POOL_CREATE_LINEAR_ALGORITHM_BIT
  };

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(PoolCreateFlagBits value) {
    if (value == PoolCreateFlagBits::eIgnoreBufferImageGranularity) return "IgnoreBufferImageGranularity";
    if (value == PoolCreateFlagBits::eLinearAlgorithm) return "LinearAlgorithm";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }
# endif
}

namespace VULKAN_HPP_NAMESPACE {
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::PoolCreateFlagBits> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
    static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<VMA_HPP_NAMESPACE::PoolCreateFlagBits> allFlags =
        VMA_HPP_NAMESPACE::PoolCreateFlagBits::eIgnoreBufferImageGranularity
      | VMA_HPP_NAMESPACE::PoolCreateFlagBits::eLinearAlgorithm;
  };
}

namespace VMA_HPP_NAMESPACE {

  using PoolCreateFlags = VULKAN_HPP_NAMESPACE::Flags<PoolCreateFlagBits>;

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR PoolCreateFlags operator|(PoolCreateFlagBits bit0, PoolCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return PoolCreateFlags(bit0) | bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR PoolCreateFlags operator&(PoolCreateFlagBits bit0, PoolCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return PoolCreateFlags(bit0) & bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR PoolCreateFlags operator^(PoolCreateFlagBits bit0, PoolCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return PoolCreateFlags(bit0) ^ bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR PoolCreateFlags operator~(PoolCreateFlagBits bits) VULKAN_HPP_NOEXCEPT {
    return ~(PoolCreateFlags(bits));
  }

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(PoolCreateFlags value) {
    if (!value) return "{}";
    std::string result;
    if (value & PoolCreateFlagBits::eIgnoreBufferImageGranularity) result += "IgnoreBufferImageGranularity | ";
    if (value & PoolCreateFlagBits::eLinearAlgorithm) result += "LinearAlgorithm | ";
    return "{ " + result.substr( 0, result.size() - 3 ) + " }";
  }
# endif
}

namespace VMA_HPP_NAMESPACE {

  enum class DefragmentationFlagBits : VmaDefragmentationFlags {
    eFlagAlgorithmFast = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_FAST_BIT,
    eFlagAlgorithmBalanced = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_BALANCED_BIT,
    eFlagAlgorithmFull = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_FULL_BIT,
    eFlagAlgorithmExtensive = VMA_DEFRAGMENTATION_FLAG_ALGORITHM_EXTENSIVE_BIT
  };

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(DefragmentationFlagBits value) {
    if (value == DefragmentationFlagBits::eFlagAlgorithmFast) return "FlagAlgorithmFast";
    if (value == DefragmentationFlagBits::eFlagAlgorithmBalanced) return "FlagAlgorithmBalanced";
    if (value == DefragmentationFlagBits::eFlagAlgorithmFull) return "FlagAlgorithmFull";
    if (value == DefragmentationFlagBits::eFlagAlgorithmExtensive) return "FlagAlgorithmExtensive";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }
# endif
}

namespace VULKAN_HPP_NAMESPACE {
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::DefragmentationFlagBits> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
    static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<VMA_HPP_NAMESPACE::DefragmentationFlagBits> allFlags =
        VMA_HPP_NAMESPACE::DefragmentationFlagBits::eFlagAlgorithmFast
      | VMA_HPP_NAMESPACE::DefragmentationFlagBits::eFlagAlgorithmBalanced
      | VMA_HPP_NAMESPACE::DefragmentationFlagBits::eFlagAlgorithmFull
      | VMA_HPP_NAMESPACE::DefragmentationFlagBits::eFlagAlgorithmExtensive;
  };
}

namespace VMA_HPP_NAMESPACE {

  using DefragmentationFlags = VULKAN_HPP_NAMESPACE::Flags<DefragmentationFlagBits>;

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR DefragmentationFlags operator|(DefragmentationFlagBits bit0, DefragmentationFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return DefragmentationFlags(bit0) | bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR DefragmentationFlags operator&(DefragmentationFlagBits bit0, DefragmentationFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return DefragmentationFlags(bit0) & bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR DefragmentationFlags operator^(DefragmentationFlagBits bit0, DefragmentationFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return DefragmentationFlags(bit0) ^ bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR DefragmentationFlags operator~(DefragmentationFlagBits bits) VULKAN_HPP_NOEXCEPT {
    return ~(DefragmentationFlags(bits));
  }

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(DefragmentationFlags value) {
    if (!value) return "{}";
    std::string result;
    if (value & DefragmentationFlagBits::eFlagAlgorithmFast) result += "FlagAlgorithmFast | ";
    if (value & DefragmentationFlagBits::eFlagAlgorithmBalanced) result += "FlagAlgorithmBalanced | ";
    if (value & DefragmentationFlagBits::eFlagAlgorithmFull) result += "FlagAlgorithmFull | ";
    if (value & DefragmentationFlagBits::eFlagAlgorithmExtensive) result += "FlagAlgorithmExtensive | ";
    return "{ " + result.substr( 0, result.size() - 3 ) + " }";
  }
# endif
}

namespace VMA_HPP_NAMESPACE {

  enum class DefragmentationMoveOperation {
    eCopy = VMA_DEFRAGMENTATION_MOVE_OPERATION_COPY,
    eIgnore = VMA_DEFRAGMENTATION_MOVE_OPERATION_IGNORE,
    eDestroy = VMA_DEFRAGMENTATION_MOVE_OPERATION_DESTROY
  };

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(DefragmentationMoveOperation value) {
    if (value == DefragmentationMoveOperation::eCopy) return "Copy";
    if (value == DefragmentationMoveOperation::eIgnore) return "Ignore";
    if (value == DefragmentationMoveOperation::eDestroy) return "Destroy";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }
# endif
}

namespace VMA_HPP_NAMESPACE {

  enum class VirtualBlockCreateFlagBits : VmaVirtualBlockCreateFlags {
    eLinearAlgorithm = VMA_VIRTUAL_BLOCK_CREATE_LINEAR_ALGORITHM_BIT
  };

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(VirtualBlockCreateFlagBits value) {
    if (value == VirtualBlockCreateFlagBits::eLinearAlgorithm) return "LinearAlgorithm";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }
# endif
}

namespace VULKAN_HPP_NAMESPACE {
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::VirtualBlockCreateFlagBits> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
    static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<VMA_HPP_NAMESPACE::VirtualBlockCreateFlagBits> allFlags =
        VMA_HPP_NAMESPACE::VirtualBlockCreateFlagBits::eLinearAlgorithm;
  };
}

namespace VMA_HPP_NAMESPACE {

  using VirtualBlockCreateFlags = VULKAN_HPP_NAMESPACE::Flags<VirtualBlockCreateFlagBits>;

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR VirtualBlockCreateFlags operator|(VirtualBlockCreateFlagBits bit0, VirtualBlockCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return VirtualBlockCreateFlags(bit0) | bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR VirtualBlockCreateFlags operator&(VirtualBlockCreateFlagBits bit0, VirtualBlockCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return VirtualBlockCreateFlags(bit0) & bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR VirtualBlockCreateFlags operator^(VirtualBlockCreateFlagBits bit0, VirtualBlockCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return VirtualBlockCreateFlags(bit0) ^ bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR VirtualBlockCreateFlags operator~(VirtualBlockCreateFlagBits bits) VULKAN_HPP_NOEXCEPT {
    return ~(VirtualBlockCreateFlags(bits));
  }

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(VirtualBlockCreateFlags value) {
    if (!value) return "{}";
    std::string result;
    if (value & VirtualBlockCreateFlagBits::eLinearAlgorithm) result += "LinearAlgorithm | ";
    return "{ " + result.substr( 0, result.size() - 3 ) + " }";
  }
# endif
}

namespace VMA_HPP_NAMESPACE {

  enum class VirtualAllocationCreateFlagBits : VmaVirtualAllocationCreateFlags {
    eUpperAddress = VMA_VIRTUAL_ALLOCATION_CREATE_UPPER_ADDRESS_BIT,
    eStrategyMinMemory = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_MEMORY_BIT,
    eStrategyMinTime = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_TIME_BIT,
    eStrategyMinOffset = VMA_VIRTUAL_ALLOCATION_CREATE_STRATEGY_MIN_OFFSET_BIT
  };

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(VirtualAllocationCreateFlagBits value) {
    if (value == VirtualAllocationCreateFlagBits::eUpperAddress) return "UpperAddress";
    if (value == VirtualAllocationCreateFlagBits::eStrategyMinMemory) return "StrategyMinMemory";
    if (value == VirtualAllocationCreateFlagBits::eStrategyMinTime) return "StrategyMinTime";
    if (value == VirtualAllocationCreateFlagBits::eStrategyMinOffset) return "StrategyMinOffset";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }
# endif
}

namespace VULKAN_HPP_NAMESPACE {
  template<> struct FlagTraits<VMA_HPP_NAMESPACE::VirtualAllocationCreateFlagBits> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool isBitmask = true;
    static VULKAN_HPP_CONST_OR_CONSTEXPR Flags<VMA_HPP_NAMESPACE::VirtualAllocationCreateFlagBits> allFlags =
        VMA_HPP_NAMESPACE::VirtualAllocationCreateFlagBits::eUpperAddress
      | VMA_HPP_NAMESPACE::VirtualAllocationCreateFlagBits::eStrategyMinMemory
      | VMA_HPP_NAMESPACE::VirtualAllocationCreateFlagBits::eStrategyMinTime
      | VMA_HPP_NAMESPACE::VirtualAllocationCreateFlagBits::eStrategyMinOffset;
  };
}

namespace VMA_HPP_NAMESPACE {

  using VirtualAllocationCreateFlags = VULKAN_HPP_NAMESPACE::Flags<VirtualAllocationCreateFlagBits>;

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR VirtualAllocationCreateFlags operator|(VirtualAllocationCreateFlagBits bit0, VirtualAllocationCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return VirtualAllocationCreateFlags(bit0) | bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR VirtualAllocationCreateFlags operator&(VirtualAllocationCreateFlagBits bit0, VirtualAllocationCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return VirtualAllocationCreateFlags(bit0) & bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR VirtualAllocationCreateFlags operator^(VirtualAllocationCreateFlagBits bit0, VirtualAllocationCreateFlagBits bit1) VULKAN_HPP_NOEXCEPT {
    return VirtualAllocationCreateFlags(bit0) ^ bit1;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR VirtualAllocationCreateFlags operator~(VirtualAllocationCreateFlagBits bits) VULKAN_HPP_NOEXCEPT {
    return ~(VirtualAllocationCreateFlags(bits));
  }

# if !defined( VULKAN_HPP_NO_TO_STRING )
  VULKAN_HPP_INLINE std::string to_string(VirtualAllocationCreateFlags value) {
    if (!value) return "{}";
    std::string result;
    if (value & VirtualAllocationCreateFlagBits::eUpperAddress) result += "UpperAddress | ";
    if (value & VirtualAllocationCreateFlagBits::eStrategyMinMemory) result += "StrategyMinMemory | ";
    if (value & VirtualAllocationCreateFlagBits::eStrategyMinTime) result += "StrategyMinTime | ";
    if (value & VirtualAllocationCreateFlagBits::eStrategyMinOffset) result += "StrategyMinOffset | ";
    return "{ " + result.substr( 0, result.size() - 3 ) + " }";
  }
# endif
}

#endif
