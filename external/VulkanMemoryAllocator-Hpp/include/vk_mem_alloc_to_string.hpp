// Generated from the Vulkan Memory Allocator (vk_mem_alloc.h).
#ifndef VULKAN_MEMORY_ALLOCATOR_TO_STRING_HPP
#define VULKAN_MEMORY_ALLOCATOR_TO_STRING_HPP

namespace VMA_HPP_NAMESPACE {

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string(AllocatorCreateFlagBits value) {
    if (value == AllocatorCreateFlagBits::eExternallySynchronized) return "ExternallySynchronized";
    if (value == AllocatorCreateFlagBits::eKhrDedicatedAllocation) return "KhrDedicatedAllocation";
    if (value == AllocatorCreateFlagBits::eKhrBindMemory2) return "KhrBindMemory2";
    if (value == AllocatorCreateFlagBits::eExtMemoryBudget) return "ExtMemoryBudget";
    if (value == AllocatorCreateFlagBits::eAmdDeviceCoherentMemory) return "AmdDeviceCoherentMemory";
    if (value == AllocatorCreateFlagBits::eBufferDeviceAddress) return "BufferDeviceAddress";
    if (value == AllocatorCreateFlagBits::eExtMemoryPriority) return "ExtMemoryPriority";
    if (value == AllocatorCreateFlagBits::eKhrMaintenance4) return "KhrMaintenance4";
    if (value == AllocatorCreateFlagBits::eKhrMaintenance5) return "KhrMaintenance5";
    if (value == AllocatorCreateFlagBits::eKhrExternalMemoryWin32) return "KhrExternalMemoryWin32";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }

  VULKAN_HPP_INLINE std::string to_string(AllocatorCreateFlags value) {
    if (!value) return "{}";
    std::string result = "{";
    if (value & AllocatorCreateFlagBits::eExternallySynchronized) result += " ExternallySynchronized |";
    if (value & AllocatorCreateFlagBits::eKhrDedicatedAllocation) result += " KhrDedicatedAllocation |";
    if (value & AllocatorCreateFlagBits::eKhrBindMemory2) result += " KhrBindMemory2 |";
    if (value & AllocatorCreateFlagBits::eExtMemoryBudget) result += " ExtMemoryBudget |";
    if (value & AllocatorCreateFlagBits::eAmdDeviceCoherentMemory) result += " AmdDeviceCoherentMemory |";
    if (value & AllocatorCreateFlagBits::eBufferDeviceAddress) result += " BufferDeviceAddress |";
    if (value & AllocatorCreateFlagBits::eExtMemoryPriority) result += " ExtMemoryPriority |";
    if (value & AllocatorCreateFlagBits::eKhrMaintenance4) result += " KhrMaintenance4 |";
    if (value & AllocatorCreateFlagBits::eKhrMaintenance5) result += " KhrMaintenance5 |";
    if (value & AllocatorCreateFlagBits::eKhrExternalMemoryWin32) result += " KhrExternalMemoryWin32 |";
    if (result.size() > 1) result.back() = '}';
    else result = "{}";
    return result;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string(MemoryUsage value) {
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

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string(AllocationCreateFlagBits value) {
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
    if (value == AllocationCreateFlagBits::eStrategyMask) return "StrategyMask";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }

  VULKAN_HPP_INLINE std::string to_string(AllocationCreateFlags value) {
    if (!value) return "{}";
    std::string result = "{";
    if (value & AllocationCreateFlagBits::eDedicatedMemory) result += " DedicatedMemory |";
    if (value & AllocationCreateFlagBits::eNeverAllocate) result += " NeverAllocate |";
    if (value & AllocationCreateFlagBits::eMapped) result += " Mapped |";
    if (value & AllocationCreateFlagBits::eUserDataCopyString) result += " UserDataCopyString |";
    if (value & AllocationCreateFlagBits::eUpperAddress) result += " UpperAddress |";
    if (value & AllocationCreateFlagBits::eDontBind) result += " DontBind |";
    if (value & AllocationCreateFlagBits::eWithinBudget) result += " WithinBudget |";
    if (value & AllocationCreateFlagBits::eCanAlias) result += " CanAlias |";
    if (value & AllocationCreateFlagBits::eHostAccessSequentialWrite) result += " HostAccessSequentialWrite |";
    if (value & AllocationCreateFlagBits::eHostAccessRandom) result += " HostAccessRandom |";
    if (value & AllocationCreateFlagBits::eHostAccessAllowTransferInstead) result += " HostAccessAllowTransferInstead |";
    if (value & AllocationCreateFlagBits::eStrategyMinMemory) result += " StrategyMinMemory |";
    if (value & AllocationCreateFlagBits::eStrategyMinTime) result += " StrategyMinTime |";
    if (value & AllocationCreateFlagBits::eStrategyMinOffset) result += " StrategyMinOffset |";
    if (value & AllocationCreateFlagBits::eStrategyBestFit) result += " StrategyBestFit |";
    if (value & AllocationCreateFlagBits::eStrategyFirstFit) result += " StrategyFirstFit |";
    if (result.size() > 1) result.back() = '}';
    else result = "{}";
    return result;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string(PoolCreateFlagBits value) {
    if (value == PoolCreateFlagBits::eIgnoreBufferImageGranularity) return "IgnoreBufferImageGranularity";
    if (value == PoolCreateFlagBits::eLinearAlgorithm) return "LinearAlgorithm";
    if (value == PoolCreateFlagBits::eAlgorithmMask) return "AlgorithmMask";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }

  VULKAN_HPP_INLINE std::string to_string(PoolCreateFlags value) {
    if (!value) return "{}";
    std::string result = "{";
    if (value & PoolCreateFlagBits::eIgnoreBufferImageGranularity) result += " IgnoreBufferImageGranularity |";
    if (value & PoolCreateFlagBits::eLinearAlgorithm) result += " LinearAlgorithm |";
    if (result.size() > 1) result.back() = '}';
    else result = "{}";
    return result;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string(DefragmentationFlagBits value) {
    if (value == DefragmentationFlagBits::eFlagAlgorithmFast) return "FlagAlgorithmFast";
    if (value == DefragmentationFlagBits::eFlagAlgorithmBalanced) return "FlagAlgorithmBalanced";
    if (value == DefragmentationFlagBits::eFlagAlgorithmFull) return "FlagAlgorithmFull";
    if (value == DefragmentationFlagBits::eFlagAlgorithmExtensive) return "FlagAlgorithmExtensive";
    if (value == DefragmentationFlagBits::eFlagAlgorithmMask) return "FlagAlgorithmMask";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }

  VULKAN_HPP_INLINE std::string to_string(DefragmentationFlags value) {
    if (!value) return "{}";
    std::string result = "{";
    if (value & DefragmentationFlagBits::eFlagAlgorithmFast) result += " FlagAlgorithmFast |";
    if (value & DefragmentationFlagBits::eFlagAlgorithmBalanced) result += " FlagAlgorithmBalanced |";
    if (value & DefragmentationFlagBits::eFlagAlgorithmFull) result += " FlagAlgorithmFull |";
    if (value & DefragmentationFlagBits::eFlagAlgorithmExtensive) result += " FlagAlgorithmExtensive |";
    if (result.size() > 1) result.back() = '}';
    else result = "{}";
    return result;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string(DefragmentationMoveOperation value) {
    if (value == DefragmentationMoveOperation::eCopy) return "Copy";
    if (value == DefragmentationMoveOperation::eIgnore) return "Ignore";
    if (value == DefragmentationMoveOperation::eDestroy) return "Destroy";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string(VirtualBlockCreateFlagBits value) {
    if (value == VirtualBlockCreateFlagBits::eLinearAlgorithm) return "LinearAlgorithm";
    if (value == VirtualBlockCreateFlagBits::eAlgorithmMask) return "AlgorithmMask";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }

  VULKAN_HPP_INLINE std::string to_string(VirtualBlockCreateFlags value) {
    if (!value) return "{}";
    std::string result = "{";
    if (value & VirtualBlockCreateFlagBits::eLinearAlgorithm) result += " LinearAlgorithm |";
    if (result.size() > 1) result.back() = '}';
    else result = "{}";
    return result;
  }

  VULKAN_HPP_INLINE VULKAN_HPP_CONSTEXPR_20 std::string to_string(VirtualAllocationCreateFlagBits value) {
    if (value == VirtualAllocationCreateFlagBits::eUpperAddress) return "UpperAddress";
    if (value == VirtualAllocationCreateFlagBits::eStrategyMinMemory) return "StrategyMinMemory";
    if (value == VirtualAllocationCreateFlagBits::eStrategyMinTime) return "StrategyMinTime";
    if (value == VirtualAllocationCreateFlagBits::eStrategyMinOffset) return "StrategyMinOffset";
    if (value == VirtualAllocationCreateFlagBits::eStrategyMask) return "StrategyMask";
    return "invalid ( " + VULKAN_HPP_NAMESPACE::toHexString(static_cast<uint32_t>(value)) + " )";
  }

  VULKAN_HPP_INLINE std::string to_string(VirtualAllocationCreateFlags value) {
    if (!value) return "{}";
    std::string result = "{";
    if (value & VirtualAllocationCreateFlagBits::eUpperAddress) result += " UpperAddress |";
    if (value & VirtualAllocationCreateFlagBits::eStrategyMinMemory) result += " StrategyMinMemory |";
    if (value & VirtualAllocationCreateFlagBits::eStrategyMinTime) result += " StrategyMinTime |";
    if (value & VirtualAllocationCreateFlagBits::eStrategyMinOffset) result += " StrategyMinOffset |";
    if (result.size() > 1) result.back() = '}';
    else result = "{}";
    return result;
  }
}
#endif