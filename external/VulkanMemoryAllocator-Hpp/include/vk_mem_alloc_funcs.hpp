// Generated from the Vulkan Memory Allocator (vk_mem_alloc.h).
#ifndef VULKAN_MEMORY_ALLOCATOR_FUNCS_HPP
#define VULKAN_MEMORY_ALLOCATOR_FUNCS_HPP

namespace VMA_HPP_NAMESPACE {

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaDestroyAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroy() const VULKAN_HPP_NOEXCEPT {
    vmaDestroyAllocator(m_allocator);
  }
#else
  // wrapper function for command vmaDestroyAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroy() const VULKAN_HPP_NOEXCEPT {
    vmaDestroyAllocator(m_allocator);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetAllocatorInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE AllocatorInfo Allocator::getAllocatorInfo() const VULKAN_HPP_NOEXCEPT {
    AllocatorInfo allocatorInfo;
    vmaGetAllocatorInfo(m_allocator, reinterpret_cast<VmaAllocatorInfo*>(&allocatorInfo));
    return allocatorInfo;
  }
#endif
  // wrapper function for command vmaGetAllocatorInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getAllocatorInfo(AllocatorInfo* pAllocatorInfo) const VULKAN_HPP_NOEXCEPT {
    vmaGetAllocatorInfo(m_allocator, reinterpret_cast<VmaAllocatorInfo*>(pAllocatorInfo));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetPhysicalDeviceProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties* Allocator::getPhysicalDeviceProperties() const VULKAN_HPP_NOEXCEPT {
    const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties* pPhysicalDeviceProperties;
    vmaGetPhysicalDeviceProperties(m_allocator, reinterpret_cast<const VkPhysicalDeviceProperties**>(&pPhysicalDeviceProperties));
    return pPhysicalDeviceProperties;
  }
#endif
  // wrapper function for command vmaGetPhysicalDeviceProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getPhysicalDeviceProperties(const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties** ppPhysicalDeviceProperties) const VULKAN_HPP_NOEXCEPT {
    vmaGetPhysicalDeviceProperties(m_allocator, reinterpret_cast<const VkPhysicalDeviceProperties**>(ppPhysicalDeviceProperties));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties* Allocator::getMemoryProperties() const VULKAN_HPP_NOEXCEPT {
    const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties* pPhysicalDeviceMemoryProperties;
    vmaGetMemoryProperties(m_allocator, reinterpret_cast<const VkPhysicalDeviceMemoryProperties**>(&pPhysicalDeviceMemoryProperties));
    return pPhysicalDeviceMemoryProperties;
  }
#endif
  // wrapper function for command vmaGetMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getMemoryProperties(const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties** ppPhysicalDeviceMemoryProperties) const VULKAN_HPP_NOEXCEPT {
    vmaGetMemoryProperties(m_allocator, reinterpret_cast<const VkPhysicalDeviceMemoryProperties**>(ppPhysicalDeviceMemoryProperties));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetMemoryTypeProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::MemoryPropertyFlags Allocator::getMemoryTypeProperties(uint32_t memoryTypeIndex) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::MemoryPropertyFlags flags;
    vmaGetMemoryTypeProperties(m_allocator, memoryTypeIndex, reinterpret_cast<VkMemoryPropertyFlags*>(&flags));
    return flags;
  }
#endif
  // wrapper function for command vmaGetMemoryTypeProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getMemoryTypeProperties(uint32_t memoryTypeIndex,
                                                            VULKAN_HPP_NAMESPACE::MemoryPropertyFlags* pFlags) const VULKAN_HPP_NOEXCEPT {
    vmaGetMemoryTypeProperties(m_allocator, memoryTypeIndex, reinterpret_cast<VkMemoryPropertyFlags*>(pFlags));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaSetCurrentFrameIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::setCurrentFrameIndex(uint32_t frameIndex) const VULKAN_HPP_NOEXCEPT {
    vmaSetCurrentFrameIndex(m_allocator, frameIndex);
  }
#else
  // wrapper function for command vmaSetCurrentFrameIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::setCurrentFrameIndex(uint32_t frameIndex) const VULKAN_HPP_NOEXCEPT {
    vmaSetCurrentFrameIndex(m_allocator, frameIndex);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCalculateStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE TotalStatistics Allocator::calculateStatistics() const VULKAN_HPP_NOEXCEPT {
    TotalStatistics stats;
    vmaCalculateStatistics(m_allocator, reinterpret_cast<VmaTotalStatistics*>(&stats));
    return stats;
  }
#endif
  // wrapper function for command vmaCalculateStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::calculateStatistics(TotalStatistics* pStats) const VULKAN_HPP_NOEXCEPT {
    vmaCalculateStatistics(m_allocator, reinterpret_cast<VmaTotalStatistics*>(pStats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetHeapBudgets, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  template <typename BudgetAllocator>
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE std::vector<Budget, BudgetAllocator> Allocator::getHeapBudgets() const VULKAN_HPP_NOEXCEPT {
    std::vector<Budget, BudgetAllocator> budgets(getMemoryProperties()->memoryHeapCount);
    vmaGetHeapBudgets(m_allocator, reinterpret_cast<VmaBudget*>(budgets.data()));
    return budgets;
  }
  // wrapper function for command vmaGetHeapBudgets, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  template <typename BudgetAllocator,
            typename std::enable_if<std::is_same<typename BudgetAllocator::value_type, Budget>::value, int>::type>
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE std::vector<Budget, BudgetAllocator> Allocator::getHeapBudgets(BudgetAllocator& budgetAllocator) const VULKAN_HPP_NOEXCEPT {
    std::vector<Budget, BudgetAllocator> budgets(getMemoryProperties()->memoryHeapCount, budgetAllocator);
    vmaGetHeapBudgets(m_allocator, reinterpret_cast<VmaBudget*>(budgets.data()));
    return budgets;
  }
#endif
  // wrapper function for command vmaGetHeapBudgets, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getHeapBudgets(Budget* pBudgets) const VULKAN_HPP_NOEXCEPT {
    vmaGetHeapBudgets(m_allocator, reinterpret_cast<VmaBudget*>(pBudgets));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFindMemoryTypeIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type Allocator::findMemoryTypeIndex(uint32_t memoryTypeBits,
                                                                                                                                       const AllocationCreateInfo& allocationCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    uint32_t memoryTypeIndex;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndex(m_allocator, memoryTypeBits, reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), &memoryTypeIndex) );
    if (result == VULKAN_HPP_NAMESPACE::Result::eErrorFeatureNotPresent) memoryTypeIndex = VULKAN_HPP_NAMESPACE::MaxMemoryTypes;
    else VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::findMemoryTypeIndex");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, memoryTypeIndex);
  }
#endif
  // wrapper function for command vmaFindMemoryTypeIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::findMemoryTypeIndex(uint32_t memoryTypeBits,
                                                                                                     const AllocationCreateInfo* pAllocationCreateInfo,
                                                                                                     uint32_t* pMemoryTypeIndex) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndex(m_allocator, memoryTypeBits, reinterpret_cast<const VmaAllocationCreateInfo*>(pAllocationCreateInfo), pMemoryTypeIndex) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFindMemoryTypeIndexForBufferInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type Allocator::findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                    const AllocationCreateInfo& allocationCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    uint32_t memoryTypeIndex;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndexForBufferInfo(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), &memoryTypeIndex) );
    if (result == VULKAN_HPP_NAMESPACE::Result::eErrorFeatureNotPresent) memoryTypeIndex = VULKAN_HPP_NAMESPACE::MaxMemoryTypes;
    else VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::findMemoryTypeIndexForBufferInfo");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, memoryTypeIndex);
  }
#endif
  // wrapper function for command vmaFindMemoryTypeIndexForBufferInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                                                                  const AllocationCreateInfo* pAllocationCreateInfo,
                                                                                                                  uint32_t* pMemoryTypeIndex) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndexForBufferInfo(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(pBufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(pAllocationCreateInfo), pMemoryTypeIndex) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFindMemoryTypeIndexForImageInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type Allocator::findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                                   const AllocationCreateInfo& allocationCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    uint32_t memoryTypeIndex;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndexForImageInfo(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), &memoryTypeIndex) );
    if (result == VULKAN_HPP_NAMESPACE::Result::eErrorFeatureNotPresent) memoryTypeIndex = VULKAN_HPP_NAMESPACE::MaxMemoryTypes;
    else VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::findMemoryTypeIndexForImageInfo");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, memoryTypeIndex);
  }
#endif
  // wrapper function for command vmaFindMemoryTypeIndexForImageInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo* pImageCreateInfo,
                                                                                                                 const AllocationCreateInfo* pAllocationCreateInfo,
                                                                                                                 uint32_t* pMemoryTypeIndex) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndexForImageInfo(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(pImageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(pAllocationCreateInfo), pMemoryTypeIndex) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreatePool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Pool>::type Allocator::createPool(const PoolCreateInfo& createInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Pool pool;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreatePool(m_allocator, reinterpret_cast<const VmaPoolCreateInfo*>(&createInfo), reinterpret_cast<VmaPool*>(&pool)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createPool");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, pool);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreatePool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniquePool>::type Allocator::createPoolUnique(const PoolCreateInfo& createInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Pool pool;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreatePool(m_allocator, reinterpret_cast<const VmaPoolCreateInfo*>(&createInfo), reinterpret_cast<VmaPool*>(&pool)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createPoolUnique");
    UniquePool poolUnique { pool, *this };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(poolUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreatePool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createPool(const PoolCreateInfo* pCreateInfo,
                                                                                            Pool* pPool) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreatePool(m_allocator, reinterpret_cast<const VmaPoolCreateInfo*>(pCreateInfo), reinterpret_cast<VmaPool*>(pPool)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaDestroyPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroyPool(Pool pool) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyPool(m_allocator, static_cast<VmaPool>(pool));
  }
#else
  // wrapper function for command vmaDestroyPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroyPool(Pool pool) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyPool(m_allocator, static_cast<VmaPool>(pool));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaDestroyPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroy(Pool pool) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyPool(m_allocator, static_cast<VmaPool>(pool));
  }
#else
  // wrapper function for command vmaDestroyPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroy(Pool pool) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyPool(m_allocator, static_cast<VmaPool>(pool));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetPoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE Statistics Allocator::getPoolStatistics(Pool pool) const VULKAN_HPP_NOEXCEPT {
    Statistics poolStats;
    vmaGetPoolStatistics(m_allocator, static_cast<VmaPool>(pool), reinterpret_cast<VmaStatistics*>(&poolStats));
    return poolStats;
  }
#endif
  // wrapper function for command vmaGetPoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getPoolStatistics(Pool pool,
                                                      Statistics* pPoolStats) const VULKAN_HPP_NOEXCEPT {
    vmaGetPoolStatistics(m_allocator, static_cast<VmaPool>(pool), reinterpret_cast<VmaStatistics*>(pPoolStats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCalculatePoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE DetailedStatistics Allocator::calculatePoolStatistics(Pool pool) const VULKAN_HPP_NOEXCEPT {
    DetailedStatistics poolStats;
    vmaCalculatePoolStatistics(m_allocator, static_cast<VmaPool>(pool), reinterpret_cast<VmaDetailedStatistics*>(&poolStats));
    return poolStats;
  }
#endif
  // wrapper function for command vmaCalculatePoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::calculatePoolStatistics(Pool pool,
                                                            DetailedStatistics* pPoolStats) const VULKAN_HPP_NOEXCEPT {
    vmaCalculatePoolStatistics(m_allocator, static_cast<VmaPool>(pool), reinterpret_cast<VmaDetailedStatistics*>(pPoolStats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCheckPoolCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::checkPoolCorruption(Pool pool) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCheckPoolCorruption(m_allocator, static_cast<VmaPool>(pool)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::checkPoolCorruption");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaCheckPoolCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::checkPoolCorruption(Pool pool) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCheckPoolCorruption(m_allocator, static_cast<VmaPool>(pool)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE const char* Allocator::getPoolName(Pool pool) const VULKAN_HPP_NOEXCEPT {
    const char* pName;
    vmaGetPoolName(m_allocator, static_cast<VmaPool>(pool), &pName);
    return pName;
  }
#endif
  // wrapper function for command vmaGetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getPoolName(Pool pool,
                                                const char** ppName) const VULKAN_HPP_NOEXCEPT {
    vmaGetPoolName(m_allocator, static_cast<VmaPool>(pool), ppName);
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaSetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::setPoolName(Pool pool,
                                                const char* name) const VULKAN_HPP_NOEXCEPT {
    vmaSetPoolName(m_allocator, static_cast<VmaPool>(pool), name);
  }
#else
  // wrapper function for command vmaSetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::setPoolName(Pool pool,
                                                const char* pName) const VULKAN_HPP_NOEXCEPT {
    vmaSetPoolName(m_allocator, static_cast<VmaPool>(pool), pName);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaAllocateMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type Allocator::allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements& vkMemoryRequirements,
                                                                                                                                    const AllocationCreateInfo& createInfo,
                                                                                                                                    VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemory(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(&vkMemoryRequirements), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemory");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, allocation);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaAllocateMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type Allocator::allocateMemoryUnique(const VULKAN_HPP_NAMESPACE::MemoryRequirements& vkMemoryRequirements,
                                                                                                                                                const AllocationCreateInfo& createInfo,
                                                                                                                                                VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemory(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(&vkMemoryRequirements), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryUnique");
    UniqueAllocation allocationUnique { allocation, *this };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(allocationUnique));
  }
#endif
#endif
  // wrapper function for command vmaAllocateMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements* pVkMemoryRequirements,
                                                                                                const AllocationCreateInfo* pCreateInfo,
                                                                                                Allocation* pAllocation,
                                                                                                AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemory(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(pVkMemoryRequirements), reinterpret_cast<const VmaAllocationCreateInfo*>(pCreateInfo), reinterpret_cast<VmaAllocation*>(pAllocation), reinterpret_cast<VmaAllocationInfo*>(pAllocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  template <typename AllocationAllocator>
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation, AllocationAllocator>>::type Allocator::allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> const & vkMemoryRequirements,
                                                                                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> const & createInfo,
                                                                                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    size_t allocationCount = vkMemoryRequirements.size() | createInfo.size() | allocationInfo.size();
#ifdef VULKAN_HPP_NO_EXCEPTIONS
    VULKAN_HPP_ASSERT(vkMemoryRequirements.size() == allocationCount);
    VULKAN_HPP_ASSERT(createInfo.size() == allocationCount);
    VULKAN_HPP_ASSERT(allocationInfo.empty() || allocationInfo.size() == allocationCount);
#else
    if (vkMemoryRequirements.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages: vkMemoryRequirements.size() != allocationCount");
    if (createInfo.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages: createInfo.size() != allocationCount");
    if (!allocationInfo.empty() && allocationInfo.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages: allocationInfo.size() != allocationCount");
#endif
    std::vector<Allocation, AllocationAllocator> allocations(allocationCount);
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements.data()), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo.data()), allocationCount, reinterpret_cast<VmaAllocation*>(allocations.data()), reinterpret_cast<VmaAllocationInfo*>(allocationInfo.data())) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, allocations);
  }
  // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  template <typename AllocationAllocator,
            typename std::enable_if<std::is_same<typename AllocationAllocator::value_type, Allocation>::value, int>::type>
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation, AllocationAllocator>>::type Allocator::allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> const & vkMemoryRequirements,
                                                                                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> const & createInfo,
                                                                                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> const & allocationInfo,
                                                                                                                                                                           AllocationAllocator& allocationAllocator) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    size_t allocationCount = vkMemoryRequirements.size() | createInfo.size() | allocationInfo.size();
#ifdef VULKAN_HPP_NO_EXCEPTIONS
    VULKAN_HPP_ASSERT(vkMemoryRequirements.size() == allocationCount);
    VULKAN_HPP_ASSERT(createInfo.size() == allocationCount);
    VULKAN_HPP_ASSERT(allocationInfo.empty() || allocationInfo.size() == allocationCount);
#else
    if (vkMemoryRequirements.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages: vkMemoryRequirements.size() != allocationCount");
    if (createInfo.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages: createInfo.size() != allocationCount");
    if (!allocationInfo.empty() && allocationInfo.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages: allocationInfo.size() != allocationCount");
#endif
    std::vector<Allocation, AllocationAllocator> allocations(allocationCount, allocationAllocator);
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements.data()), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo.data()), allocationCount, reinterpret_cast<VmaAllocation*>(allocations.data()), reinterpret_cast<VmaAllocationInfo*>(allocationInfo.data())) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, allocations);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  template <typename AllocationAllocator>
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<UniqueAllocation, AllocationAllocator>>::type Allocator::allocateMemoryPagesUnique(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> const & vkMemoryRequirements,
                                                                                                                                                                                       VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> const & createInfo,
                                                                                                                                                                                       VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    size_t allocationCount = vkMemoryRequirements.size() | createInfo.size() | allocationInfo.size();
#ifdef VULKAN_HPP_NO_EXCEPTIONS
    VULKAN_HPP_ASSERT(vkMemoryRequirements.size() == allocationCount);
    VULKAN_HPP_ASSERT(createInfo.size() == allocationCount);
    VULKAN_HPP_ASSERT(allocationInfo.empty() || allocationInfo.size() == allocationCount);
#else
    if (vkMemoryRequirements.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPagesUnique: vkMemoryRequirements.size() != allocationCount");
    if (createInfo.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPagesUnique: createInfo.size() != allocationCount");
    if (!allocationInfo.empty() && allocationInfo.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPagesUnique: allocationInfo.size() != allocationCount");
#endif
    std::vector<Allocation> allocations(allocationCount);
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements.data()), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo.data()), allocationCount, reinterpret_cast<VmaAllocation*>(allocations.data()), reinterpret_cast<VmaAllocationInfo*>(allocationInfo.data())) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPagesUnique");
    std::vector<UniqueAllocation, AllocationAllocator> allocationsUnique;
    allocationsUnique.reserve(allocations.size());
    for (auto const& t : allocations) allocationsUnique.emplace_back(t, *this);
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(allocationsUnique));
  }
  // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  template <typename AllocationAllocator,
            typename std::enable_if<std::is_same<typename AllocationAllocator::value_type, UniqueAllocation>::value, int>::type>
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<UniqueAllocation, AllocationAllocator>>::type Allocator::allocateMemoryPagesUnique(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> const & vkMemoryRequirements,
                                                                                                                                                                                       VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> const & createInfo,
                                                                                                                                                                                       VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> const & allocationInfo,
                                                                                                                                                                                       AllocationAllocator& allocationAllocator) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    size_t allocationCount = vkMemoryRequirements.size() | createInfo.size() | allocationInfo.size();
#ifdef VULKAN_HPP_NO_EXCEPTIONS
    VULKAN_HPP_ASSERT(vkMemoryRequirements.size() == allocationCount);
    VULKAN_HPP_ASSERT(createInfo.size() == allocationCount);
    VULKAN_HPP_ASSERT(allocationInfo.empty() || allocationInfo.size() == allocationCount);
#else
    if (vkMemoryRequirements.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPagesUnique: vkMemoryRequirements.size() != allocationCount");
    if (createInfo.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPagesUnique: createInfo.size() != allocationCount");
    if (!allocationInfo.empty() && allocationInfo.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPagesUnique: allocationInfo.size() != allocationCount");
#endif
    std::vector<Allocation> allocations(allocationCount);
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements.data()), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo.data()), allocationCount, reinterpret_cast<VmaAllocation*>(allocations.data()), reinterpret_cast<VmaAllocationInfo*>(allocationInfo.data())) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPagesUnique");
    std::vector<UniqueAllocation, AllocationAllocator> allocationsUnique(allocationAllocator);
    allocationsUnique.reserve(allocations.size());
    for (auto const& t : allocations) allocationsUnique.emplace_back(t, *this);
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(allocationsUnique));
  }
#endif
#endif
  // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::allocateMemoryPages(const VULKAN_HPP_NAMESPACE::MemoryRequirements* pVkMemoryRequirements,
                                                                                                     const AllocationCreateInfo* pCreateInfo,
                                                                                                     size_t allocationCount,
                                                                                                     Allocation* pAllocations,
                                                                                                     AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(pVkMemoryRequirements), reinterpret_cast<const VmaAllocationCreateInfo*>(pCreateInfo), allocationCount, reinterpret_cast<VmaAllocation*>(pAllocations), reinterpret_cast<VmaAllocationInfo*>(pAllocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaAllocateMemoryForBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type Allocator::allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                                             const AllocationCreateInfo& createInfo,
                                                                                                                                             VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForBuffer(m_allocator, static_cast<VkBuffer>(buffer), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryForBuffer");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, allocation);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaAllocateMemoryForBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type Allocator::allocateMemoryForBufferUnique(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                                                         const AllocationCreateInfo& createInfo,
                                                                                                                                                         VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForBuffer(m_allocator, static_cast<VkBuffer>(buffer), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryForBufferUnique");
    UniqueAllocation allocationUnique { allocation, *this };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(allocationUnique));
  }
#endif
#endif
  // wrapper function for command vmaAllocateMemoryForBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                         const AllocationCreateInfo* pCreateInfo,
                                                                                                         Allocation* pAllocation,
                                                                                                         AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForBuffer(m_allocator, static_cast<VkBuffer>(buffer), reinterpret_cast<const VmaAllocationCreateInfo*>(pCreateInfo), reinterpret_cast<VmaAllocation*>(pAllocation), reinterpret_cast<VmaAllocationInfo*>(pAllocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaAllocateMemoryForImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type Allocator::allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                                            const AllocationCreateInfo& createInfo,
                                                                                                                                            VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForImage(m_allocator, static_cast<VkImage>(image), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryForImage");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, allocation);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaAllocateMemoryForImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type Allocator::allocateMemoryForImageUnique(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                                                        const AllocationCreateInfo& createInfo,
                                                                                                                                                        VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForImage(m_allocator, static_cast<VkImage>(image), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryForImageUnique");
    UniqueAllocation allocationUnique { allocation, *this };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(allocationUnique));
  }
#endif
#endif
  // wrapper function for command vmaAllocateMemoryForImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                        const AllocationCreateInfo* pCreateInfo,
                                                                                                        Allocation* pAllocation,
                                                                                                        AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForImage(m_allocator, static_cast<VkImage>(image), reinterpret_cast<const VmaAllocationCreateInfo*>(pCreateInfo), reinterpret_cast<VmaAllocation*>(pAllocation), reinterpret_cast<VmaAllocationInfo*>(pAllocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFreeMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::freeMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaFreeMemory(m_allocator, static_cast<VmaAllocation>(allocation));
  }
#else
  // wrapper function for command vmaFreeMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::freeMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaFreeMemory(m_allocator, static_cast<VmaAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFreeMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (Allocator::free)(Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaFreeMemory(m_allocator, static_cast<VmaAllocation>(allocation));
  }
#else
  // wrapper function for command vmaFreeMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (Allocator::free)(Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaFreeMemory(m_allocator, static_cast<VmaAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFreeMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::freeMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations) const VULKAN_HPP_NOEXCEPT {
    size_t allocationCount = allocations.size();
    vmaFreeMemoryPages(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations.data()));
  }
#endif
  // wrapper function for command vmaFreeMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::freeMemoryPages(size_t allocationCount,
                                                    const Allocation* pAllocations) const VULKAN_HPP_NOEXCEPT {
    vmaFreeMemoryPages(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(pAllocations));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFreeMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (Allocator::free)(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations) const VULKAN_HPP_NOEXCEPT {
    size_t allocationCount = allocations.size();
    vmaFreeMemoryPages(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations.data()));
  }
#endif
  // wrapper function for command vmaFreeMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (Allocator::free)(size_t allocationCount,
                                           const Allocation* pAllocations) const VULKAN_HPP_NOEXCEPT {
    vmaFreeMemoryPages(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(pAllocations));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE AllocationInfo Allocator::getAllocationInfo(Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    AllocationInfo allocationInfo;
    vmaGetAllocationInfo(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VmaAllocationInfo*>(&allocationInfo));
    return allocationInfo;
  }
#endif
  // wrapper function for command vmaGetAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getAllocationInfo(Allocation allocation,
                                                      AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT {
    vmaGetAllocationInfo(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VmaAllocationInfo*>(pAllocationInfo));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetAllocationInfo2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE AllocationInfo2 Allocator::getAllocationInfo2(Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    AllocationInfo2 allocationInfo;
    vmaGetAllocationInfo2(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VmaAllocationInfo2*>(&allocationInfo));
    return allocationInfo;
  }
#endif
  // wrapper function for command vmaGetAllocationInfo2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getAllocationInfo2(Allocation allocation,
                                                       AllocationInfo2* pAllocationInfo) const VULKAN_HPP_NOEXCEPT {
    vmaGetAllocationInfo2(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VmaAllocationInfo2*>(pAllocationInfo));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaSetAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::setAllocationUserData(Allocation allocation,
                                                          void* userData) const VULKAN_HPP_NOEXCEPT {
    vmaSetAllocationUserData(m_allocator, static_cast<VmaAllocation>(allocation), userData);
  }
#else
  // wrapper function for command vmaSetAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::setAllocationUserData(Allocation allocation,
                                                          void* pUserData) const VULKAN_HPP_NOEXCEPT {
    vmaSetAllocationUserData(m_allocator, static_cast<VmaAllocation>(allocation), pUserData);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaSetAllocationName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::setAllocationName(Allocation allocation,
                                                      const char* name) const VULKAN_HPP_NOEXCEPT {
    vmaSetAllocationName(m_allocator, static_cast<VmaAllocation>(allocation), name);
  }
#else
  // wrapper function for command vmaSetAllocationName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::setAllocationName(Allocation allocation,
                                                      const char* pName) const VULKAN_HPP_NOEXCEPT {
    vmaSetAllocationName(m_allocator, static_cast<VmaAllocation>(allocation), pName);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetAllocationMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::MemoryPropertyFlags Allocator::getAllocationMemoryProperties(Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::MemoryPropertyFlags flags;
    vmaGetAllocationMemoryProperties(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VkMemoryPropertyFlags*>(&flags));
    return flags;
  }
#endif
  // wrapper function for command vmaGetAllocationMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::getAllocationMemoryProperties(Allocation allocation,
                                                                  VULKAN_HPP_NAMESPACE::MemoryPropertyFlags* pFlags) const VULKAN_HPP_NOEXCEPT {
    vmaGetAllocationMemoryProperties(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VkMemoryPropertyFlags*>(pFlags));
  }

#if VMA_EXTERNAL_MEMORY_WIN32
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetMemoryWin32Handle, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<HANDLE>::type Allocator::getMemoryWin32Handle(Allocation allocation,
                                                                                                                                      HANDLE hTargetProcess) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    HANDLE handle;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaGetMemoryWin32Handle(m_allocator, static_cast<VmaAllocation>(allocation), hTargetProcess, &handle) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::getMemoryWin32Handle");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, handle);
  }
#endif
  // wrapper function for command vmaGetMemoryWin32Handle, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::getMemoryWin32Handle(Allocation allocation,
                                                                                                      HANDLE hTargetProcess,
                                                                                                      HANDLE* pHandle) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaGetMemoryWin32Handle(m_allocator, static_cast<VmaAllocation>(allocation), hTargetProcess, pHandle) );
    return result;
  }

#endif 
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaMapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void*>::type Allocator::mapMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    void* pData;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaMapMemory(m_allocator, static_cast<VmaAllocation>(allocation), &pData) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::mapMemory");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, pData);
  }
#endif
  // wrapper function for command vmaMapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::mapMemory(Allocation allocation,
                                                                                           void** ppData) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaMapMemory(m_allocator, static_cast<VmaAllocation>(allocation), ppData) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaUnmapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::unmapMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaUnmapMemory(m_allocator, static_cast<VmaAllocation>(allocation));
  }
#else
  // wrapper function for command vmaUnmapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::unmapMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaUnmapMemory(m_allocator, static_cast<VmaAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFlushAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::flushAllocation(Allocation allocation,
                                                                                                                                                  VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                                                                  VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFlushAllocation(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::flushAllocation");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaFlushAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::flushAllocation(Allocation allocation,
                                                                                                 VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                 VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFlushAllocation(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaInvalidateAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::invalidateAllocation(Allocation allocation,
                                                                                                                                                       VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                                                                       VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaInvalidateAllocation(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::invalidateAllocation");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaInvalidateAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::invalidateAllocation(Allocation allocation,
                                                                                                      VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                      VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaInvalidateAllocation(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFlushAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::flushAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations,
                                                                                                                                                   VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & offsets,
                                                                                                                                                   VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & sizes) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    uint32_t allocationCount = allocations.size() | offsets.size() | sizes.size();
#ifdef VULKAN_HPP_NO_EXCEPTIONS
    VULKAN_HPP_ASSERT(allocations.empty() || allocations.size() == allocationCount);
    VULKAN_HPP_ASSERT(offsets.empty() || offsets.size() == allocationCount);
    VULKAN_HPP_ASSERT(sizes.empty() || sizes.size() == allocationCount);
#else
    if (!allocations.empty() && allocations.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::flushAllocations: allocations.size() != allocationCount");
    if (!offsets.empty() && offsets.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::flushAllocations: offsets.size() != allocationCount");
    if (!sizes.empty() && sizes.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::flushAllocations: sizes.size() != allocationCount");
#endif
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFlushAllocations(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations.data()), reinterpret_cast<const VkDeviceSize*>(offsets.data()), reinterpret_cast<const VkDeviceSize*>(sizes.data())) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::flushAllocations");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#endif
  // wrapper function for command vmaFlushAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::flushAllocations(uint32_t allocationCount,
                                                                                                  const Allocation* allocations,
                                                                                                  const VULKAN_HPP_NAMESPACE::DeviceSize* offsets,
                                                                                                  const VULKAN_HPP_NAMESPACE::DeviceSize* sizes) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFlushAllocations(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations), reinterpret_cast<const VkDeviceSize*>(offsets), reinterpret_cast<const VkDeviceSize*>(sizes)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaInvalidateAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::invalidateAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations,
                                                                                                                                                        VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & offsets,
                                                                                                                                                        VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & sizes) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    uint32_t allocationCount = allocations.size() | offsets.size() | sizes.size();
#ifdef VULKAN_HPP_NO_EXCEPTIONS
    VULKAN_HPP_ASSERT(allocations.empty() || allocations.size() == allocationCount);
    VULKAN_HPP_ASSERT(offsets.empty() || offsets.size() == allocationCount);
    VULKAN_HPP_ASSERT(sizes.empty() || sizes.size() == allocationCount);
#else
    if (!allocations.empty() && allocations.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::invalidateAllocations: allocations.size() != allocationCount");
    if (!offsets.empty() && offsets.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::invalidateAllocations: offsets.size() != allocationCount");
    if (!sizes.empty() && sizes.size() != allocationCount) throw VULKAN_HPP_NAMESPACE::LogicError(VMA_HPP_NAMESPACE_STRING "::Allocator::invalidateAllocations: sizes.size() != allocationCount");
#endif
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaInvalidateAllocations(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations.data()), reinterpret_cast<const VkDeviceSize*>(offsets.data()), reinterpret_cast<const VkDeviceSize*>(sizes.data())) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::invalidateAllocations");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#endif
  // wrapper function for command vmaInvalidateAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::invalidateAllocations(uint32_t allocationCount,
                                                                                                       const Allocation* allocations,
                                                                                                       const VULKAN_HPP_NAMESPACE::DeviceSize* offsets,
                                                                                                       const VULKAN_HPP_NAMESPACE::DeviceSize* sizes) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaInvalidateAllocations(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations), reinterpret_cast<const VkDeviceSize*>(offsets), reinterpret_cast<const VkDeviceSize*>(sizes)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCopyMemoryToAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::copyMemoryToAllocation(const void* srcHostPointer,
                                                                                                                                                         Allocation dstAllocation,
                                                                                                                                                         VULKAN_HPP_NAMESPACE::DeviceSize dstAllocationLocalOffset,
                                                                                                                                                         VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCopyMemoryToAllocation(m_allocator, srcHostPointer, static_cast<VmaAllocation>(dstAllocation), static_cast<VkDeviceSize>(dstAllocationLocalOffset), static_cast<VkDeviceSize>(size)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::copyMemoryToAllocation");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaCopyMemoryToAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::copyMemoryToAllocation(const void* pSrcHostPointer,
                                                                                                        Allocation dstAllocation,
                                                                                                        VULKAN_HPP_NAMESPACE::DeviceSize dstAllocationLocalOffset,
                                                                                                        VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCopyMemoryToAllocation(m_allocator, pSrcHostPointer, static_cast<VmaAllocation>(dstAllocation), static_cast<VkDeviceSize>(dstAllocationLocalOffset), static_cast<VkDeviceSize>(size)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCopyAllocationToMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::copyAllocationToMemory(Allocation srcAllocation,
                                                                                                                                                         VULKAN_HPP_NAMESPACE::DeviceSize srcAllocationLocalOffset,
                                                                                                                                                         void* dstHostPointer,
                                                                                                                                                         VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCopyAllocationToMemory(m_allocator, static_cast<VmaAllocation>(srcAllocation), static_cast<VkDeviceSize>(srcAllocationLocalOffset), dstHostPointer, static_cast<VkDeviceSize>(size)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::copyAllocationToMemory");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaCopyAllocationToMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::copyAllocationToMemory(Allocation srcAllocation,
                                                                                                        VULKAN_HPP_NAMESPACE::DeviceSize srcAllocationLocalOffset,
                                                                                                        void* pDstHostPointer,
                                                                                                        VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCopyAllocationToMemory(m_allocator, static_cast<VmaAllocation>(srcAllocation), static_cast<VkDeviceSize>(srcAllocationLocalOffset), pDstHostPointer, static_cast<VkDeviceSize>(size)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCheckCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::checkCorruption(uint32_t memoryTypeBits) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCheckCorruption(m_allocator, memoryTypeBits) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::checkCorruption");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaCheckCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::checkCorruption(uint32_t memoryTypeBits) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCheckCorruption(m_allocator, memoryTypeBits) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaBeginDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationContext>::type Allocator::beginDefragmentation(const DefragmentationInfo& info) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    DefragmentationContext context;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBeginDefragmentation(m_allocator, reinterpret_cast<const VmaDefragmentationInfo*>(&info), reinterpret_cast<VmaDefragmentationContext*>(&context)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::beginDefragmentation");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, context);
  }
#endif
  // wrapper function for command vmaBeginDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::beginDefragmentation(const DefragmentationInfo* pInfo,
                                                                                                      DefragmentationContext* pContext) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBeginDefragmentation(m_allocator, reinterpret_cast<const VmaDefragmentationInfo*>(pInfo), reinterpret_cast<VmaDefragmentationContext*>(pContext)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaEndDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::endDefragmentation(DefragmentationContext context,
                                                       VULKAN_HPP_NAMESPACE::Optional<DefragmentationStats> const & stats) const VULKAN_HPP_NOEXCEPT {
    vmaEndDefragmentation(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationStats*>(static_cast<DefragmentationStats*>(stats)));
  }
#endif
  // wrapper function for command vmaEndDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::endDefragmentation(DefragmentationContext context,
                                                       DefragmentationStats* pStats) const VULKAN_HPP_NOEXCEPT {
    vmaEndDefragmentation(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationStats*>(pStats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaBeginDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValue<DefragmentationPassMoveInfo> Allocator::beginDefragmentationPass(DefragmentationContext context) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    DefragmentationPassMoveInfo passInfo;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBeginDefragmentationPass(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationPassMoveInfo*>(&passInfo)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::beginDefragmentationPass", { VULKAN_HPP_NAMESPACE::Result::eSuccess, VULKAN_HPP_NAMESPACE::Result::eIncomplete });
    return VULKAN_HPP_NAMESPACE::ResultValue<DefragmentationPassMoveInfo>(result, passInfo);
  }
#endif
  // wrapper function for command vmaBeginDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::beginDefragmentationPass(DefragmentationContext context,
                                                                                                          DefragmentationPassMoveInfo* pPassInfo) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBeginDefragmentationPass(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationPassMoveInfo*>(pPassInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaEndDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::ResultValue<DefragmentationPassMoveInfo> Allocator::endDefragmentationPass(DefragmentationContext context) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    DefragmentationPassMoveInfo passInfo;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaEndDefragmentationPass(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationPassMoveInfo*>(&passInfo)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::endDefragmentationPass", { VULKAN_HPP_NAMESPACE::Result::eSuccess, VULKAN_HPP_NAMESPACE::Result::eIncomplete });
    return VULKAN_HPP_NAMESPACE::ResultValue<DefragmentationPassMoveInfo>(result, passInfo);
  }
#endif
  // wrapper function for command vmaEndDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::endDefragmentationPass(DefragmentationContext context,
                                                                                                        DefragmentationPassMoveInfo* pPassInfo) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaEndDefragmentationPass(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationPassMoveInfo*>(pPassInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaBindBufferMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::bindBufferMemory(Allocation allocation,
                                                                                                                                                   VULKAN_HPP_NAMESPACE::Buffer buffer) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindBufferMemory(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkBuffer>(buffer)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::bindBufferMemory");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaBindBufferMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::bindBufferMemory(Allocation allocation,
                                                                                                  VULKAN_HPP_NAMESPACE::Buffer buffer) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindBufferMemory(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkBuffer>(buffer)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaBindBufferMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::bindBufferMemory2(Allocation allocation,
                                                                                                                                                    VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                                    VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                                                    const void* next) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindBufferMemory2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), static_cast<VkBuffer>(buffer), next) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::bindBufferMemory2");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaBindBufferMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::bindBufferMemory2(Allocation allocation,
                                                                                                   VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                   VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                   const void* pNext) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindBufferMemory2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), static_cast<VkBuffer>(buffer), pNext) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaBindImageMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::bindImageMemory(Allocation allocation,
                                                                                                                                                  VULKAN_HPP_NAMESPACE::Image image) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindImageMemory(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkImage>(image)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::bindImageMemory");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaBindImageMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::bindImageMemory(Allocation allocation,
                                                                                                 VULKAN_HPP_NAMESPACE::Image image) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindImageMemory(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkImage>(image)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaBindImageMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::bindImageMemory2(Allocation allocation,
                                                                                                                                                   VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                                   VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                                                   const void* next) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindImageMemory2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), static_cast<VkImage>(image), next) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::bindImageMemory2");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result);
  }
#else
  // wrapper function for command vmaBindImageMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::bindImageMemory2(Allocation allocation,
                                                                                                  VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                  VULKAN_HPP_NAMESPACE::Image image,
                                                                                                  const void* pNext) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindImageMemory2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), static_cast<VkImage>(image), pNext) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<Allocation, VULKAN_HPP_NAMESPACE::Buffer>>::type Allocator::createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                                           const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                           VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    std::pair<Allocation, VULKAN_HPP_NAMESPACE::Buffer> pair;
    VULKAN_HPP_NAMESPACE::Buffer& buffer = pair.second;
    Allocation& allocation = pair.first;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBuffer(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), reinterpret_cast<VkBuffer*>(&buffer), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createBuffer");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, pair);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueAllocation, UniqueBuffer>>::type Allocator::createBufferUnique(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                                       const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                       VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    std::pair<Allocation, VULKAN_HPP_NAMESPACE::Buffer> pair;
    VULKAN_HPP_NAMESPACE::Buffer& buffer = pair.second;
    Allocation& allocation = pair.first;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBuffer(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), reinterpret_cast<VkBuffer*>(&buffer), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createBufferUnique");
    std::pair<UniqueAllocation, UniqueBuffer> pairUnique {
      UniqueAllocation(allocation, *this),
      UniqueBuffer(buffer, *this)
    };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(pairUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreateBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                                              const AllocationCreateInfo* pAllocationCreateInfo,
                                                                                              VULKAN_HPP_NAMESPACE::Buffer* pBuffer,
                                                                                              Allocation* pAllocation,
                                                                                              AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBuffer(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(pBufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(pAllocationCreateInfo), reinterpret_cast<VkBuffer*>(pBuffer), reinterpret_cast<VmaAllocation*>(pAllocation), reinterpret_cast<VmaAllocationInfo*>(pAllocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateBufferWithAlignment, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<Allocation, VULKAN_HPP_NAMESPACE::Buffer>>::type Allocator::createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                                                        const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                                        VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                                                                                                        VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    std::pair<Allocation, VULKAN_HPP_NAMESPACE::Buffer> pair;
    VULKAN_HPP_NAMESPACE::Buffer& buffer = pair.second;
    Allocation& allocation = pair.first;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBufferWithAlignment(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), static_cast<VkDeviceSize>(minAlignment), reinterpret_cast<VkBuffer*>(&buffer), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createBufferWithAlignment");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, pair);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateBufferWithAlignment, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueAllocation, UniqueBuffer>>::type Allocator::createBufferWithAlignmentUnique(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                                                    const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                                    VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                                                                                                    VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    std::pair<Allocation, VULKAN_HPP_NAMESPACE::Buffer> pair;
    VULKAN_HPP_NAMESPACE::Buffer& buffer = pair.second;
    Allocation& allocation = pair.first;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBufferWithAlignment(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), static_cast<VkDeviceSize>(minAlignment), reinterpret_cast<VkBuffer*>(&buffer), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createBufferWithAlignmentUnique");
    std::pair<UniqueAllocation, UniqueBuffer> pairUnique {
      UniqueAllocation(allocation, *this),
      UniqueBuffer(buffer, *this)
    };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(pairUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreateBufferWithAlignment, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                                                           const AllocationCreateInfo* pAllocationCreateInfo,
                                                                                                           VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                           VULKAN_HPP_NAMESPACE::Buffer* pBuffer,
                                                                                                           Allocation* pAllocation,
                                                                                                           AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBufferWithAlignment(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(pBufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(pAllocationCreateInfo), static_cast<VkDeviceSize>(minAlignment), reinterpret_cast<VkBuffer*>(pBuffer), reinterpret_cast<VmaAllocation*>(pAllocation), reinterpret_cast<VmaAllocationInfo*>(pAllocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateAliasingBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Buffer>::type Allocator::createAliasingBuffer(Allocation allocation,
                                                                                                                                                            const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Buffer buffer;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingBuffer(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<VkBuffer*>(&buffer)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingBuffer");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, buffer);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateAliasingBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueBuffer>::type Allocator::createAliasingBufferUnique(Allocation allocation,
                                                                                                                                                  const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Buffer buffer;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingBuffer(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<VkBuffer*>(&buffer)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingBufferUnique");
    UniqueBuffer bufferUnique { buffer, *this };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(bufferUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreateAliasingBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createAliasingBuffer(Allocation allocation,
                                                                                                      const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                                                      VULKAN_HPP_NAMESPACE::Buffer* pBuffer) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingBuffer(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkBufferCreateInfo*>(pBufferCreateInfo), reinterpret_cast<VkBuffer*>(pBuffer)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateAliasingBuffer2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Buffer>::type Allocator::createAliasingBuffer2(Allocation allocation,
                                                                                                                                                             VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                                             const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Buffer buffer;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingBuffer2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<VkBuffer*>(&buffer)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingBuffer2");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, buffer);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateAliasingBuffer2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueBuffer>::type Allocator::createAliasingBuffer2Unique(Allocation allocation,
                                                                                                                                                   VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                                   const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Buffer buffer;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingBuffer2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<VkBuffer*>(&buffer)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingBuffer2Unique");
    UniqueBuffer bufferUnique { buffer, *this };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(bufferUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreateAliasingBuffer2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createAliasingBuffer2(Allocation allocation,
                                                                                                       VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                       const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                                                       VULKAN_HPP_NAMESPACE::Buffer* pBuffer) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingBuffer2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), reinterpret_cast<const VkBufferCreateInfo*>(pBufferCreateInfo), reinterpret_cast<VkBuffer*>(pBuffer)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaDestroyBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroyBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                  Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyBuffer(m_allocator, static_cast<VkBuffer>(buffer), static_cast<VmaAllocation>(allocation));
  }
#else
  // wrapper function for command vmaDestroyBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroyBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                  Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyBuffer(m_allocator, static_cast<VkBuffer>(buffer), static_cast<VmaAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaDestroyBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroy(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                            Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyBuffer(m_allocator, static_cast<VkBuffer>(buffer), static_cast<VmaAllocation>(allocation));
  }
#else
  // wrapper function for command vmaDestroyBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroy(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                            Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyBuffer(m_allocator, static_cast<VkBuffer>(buffer), static_cast<VmaAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<Allocation, VULKAN_HPP_NAMESPACE::Image>>::type Allocator::createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                                                         const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                         VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    std::pair<Allocation, VULKAN_HPP_NAMESPACE::Image> pair;
    VULKAN_HPP_NAMESPACE::Image& image = pair.second;
    Allocation& allocation = pair.first;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), reinterpret_cast<VkImage*>(&image), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createImage");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, pair);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueAllocation, UniqueImage>>::type Allocator::createImageUnique(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                                                     const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                     VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    std::pair<Allocation, VULKAN_HPP_NAMESPACE::Image> pair;
    VULKAN_HPP_NAMESPACE::Image& image = pair.second;
    Allocation& allocation = pair.first;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), reinterpret_cast<VkImage*>(&image), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createImageUnique");
    std::pair<UniqueAllocation, UniqueImage> pairUnique {
      UniqueAllocation(allocation, *this),
      UniqueImage(image, *this)
    };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(pairUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreateImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo* pImageCreateInfo,
                                                                                             const AllocationCreateInfo* pAllocationCreateInfo,
                                                                                             VULKAN_HPP_NAMESPACE::Image* pImage,
                                                                                             Allocation* pAllocation,
                                                                                             AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(pImageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(pAllocationCreateInfo), reinterpret_cast<VkImage*>(pImage), reinterpret_cast<VmaAllocation*>(pAllocation), reinterpret_cast<VmaAllocationInfo*>(pAllocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateAliasingImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Image>::type Allocator::createAliasingImage(Allocation allocation,
                                                                                                                                                          const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Image image;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingImage(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<VkImage*>(&image)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingImage");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, image);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateAliasingImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueImage>::type Allocator::createAliasingImageUnique(Allocation allocation,
                                                                                                                                                const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Image image;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingImage(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<VkImage*>(&image)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingImageUnique");
    UniqueImage imageUnique { image, *this };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(imageUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreateAliasingImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createAliasingImage(Allocation allocation,
                                                                                                     const VULKAN_HPP_NAMESPACE::ImageCreateInfo* pImageCreateInfo,
                                                                                                     VULKAN_HPP_NAMESPACE::Image* pImage) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingImage(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkImageCreateInfo*>(pImageCreateInfo), reinterpret_cast<VkImage*>(pImage)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateAliasingImage2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Image>::type Allocator::createAliasingImage2(Allocation allocation,
                                                                                                                                                           VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                                           const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Image image;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingImage2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<VkImage*>(&image)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingImage2");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, image);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateAliasingImage2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueImage>::type Allocator::createAliasingImage2Unique(Allocation allocation,
                                                                                                                                                 VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                                 const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VULKAN_HPP_NAMESPACE::Image image;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingImage2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<VkImage*>(&image)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingImage2Unique");
    UniqueImage imageUnique { image, *this };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(imageUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreateAliasingImage2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createAliasingImage2(Allocation allocation,
                                                                                                      VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                      const VULKAN_HPP_NAMESPACE::ImageCreateInfo* pImageCreateInfo,
                                                                                                      VULKAN_HPP_NAMESPACE::Image* pImage) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingImage2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), reinterpret_cast<const VkImageCreateInfo*>(pImageCreateInfo), reinterpret_cast<VkImage*>(pImage)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaDestroyImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroyImage(VULKAN_HPP_NAMESPACE::Image image,
                                                 Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyImage(m_allocator, static_cast<VkImage>(image), static_cast<VmaAllocation>(allocation));
  }
#else
  // wrapper function for command vmaDestroyImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroyImage(VULKAN_HPP_NAMESPACE::Image image,
                                                 Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyImage(m_allocator, static_cast<VkImage>(image), static_cast<VmaAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaDestroyImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroy(VULKAN_HPP_NAMESPACE::Image image,
                                            Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyImage(m_allocator, static_cast<VkImage>(image), static_cast<VmaAllocation>(allocation));
  }
#else
  // wrapper function for command vmaDestroyImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::destroy(VULKAN_HPP_NAMESPACE::Image image,
                                            Allocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaDestroyImage(m_allocator, static_cast<VkImage>(image), static_cast<VmaAllocation>(allocation));
  }
#endif

#if VMA_STATS_STRING_ENABLED
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaBuildStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE char* Allocator::buildStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT {
    char* pStatsString;
    vmaBuildStatsString(m_allocator, &pStatsString, static_cast<VkBool32>(detailedMap));
    return pStatsString;
  }
#endif
  // wrapper function for command vmaBuildStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::buildStatsString(char** ppStatsString,
                                                     VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT {
    vmaBuildStatsString(m_allocator, ppStatsString, static_cast<VkBool32>(detailedMap));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFreeStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::freeStatsString(char* statsString) const VULKAN_HPP_NOEXCEPT {
    vmaFreeStatsString(m_allocator, statsString);
  }
#else
  // wrapper function for command vmaFreeStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void Allocator::freeStatsString(char* pStatsString) const VULKAN_HPP_NOEXCEPT {
    vmaFreeStatsString(m_allocator, pStatsString);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFreeStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (Allocator::free)(char* statsString) const VULKAN_HPP_NOEXCEPT {
    vmaFreeStatsString(m_allocator, statsString);
  }
#else
  // wrapper function for command vmaFreeStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (Allocator::free)(char* pStatsString) const VULKAN_HPP_NOEXCEPT {
    vmaFreeStatsString(m_allocator, pStatsString);
  }
#endif
#endif 

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaDestroyVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::destroy() const VULKAN_HPP_NOEXCEPT {
    vmaDestroyVirtualBlock(m_virtualBlock);
  }
#else
  // wrapper function for command vmaDestroyVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::destroy() const VULKAN_HPP_NOEXCEPT {
    vmaDestroyVirtualBlock(m_virtualBlock);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaIsVirtualBlockEmpty, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Bool32 VirtualBlock::isVirtualBlockEmpty() const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Bool32 result = static_cast<VULKAN_HPP_NAMESPACE::Bool32>( vmaIsVirtualBlockEmpty(m_virtualBlock) );
    return result;
  }
#else
  // wrapper function for command vmaIsVirtualBlockEmpty, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Bool32 VirtualBlock::isVirtualBlockEmpty() const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Bool32 result = static_cast<VULKAN_HPP_NAMESPACE::Bool32>( vmaIsVirtualBlockEmpty(m_virtualBlock) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetVirtualAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VirtualAllocationInfo VirtualBlock::getVirtualAllocationInfo(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT {
    VirtualAllocationInfo virtualAllocInfo;
    vmaGetVirtualAllocationInfo(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation), reinterpret_cast<VmaVirtualAllocationInfo*>(&virtualAllocInfo));
    return virtualAllocInfo;
  }
#endif
  // wrapper function for command vmaGetVirtualAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::getVirtualAllocationInfo(VirtualAllocation allocation,
                                                                VirtualAllocationInfo* pVirtualAllocInfo) const VULKAN_HPP_NOEXCEPT {
    vmaGetVirtualAllocationInfo(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation), reinterpret_cast<VmaVirtualAllocationInfo*>(pVirtualAllocInfo));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaVirtualAllocate, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualAllocation>::type VirtualBlock::virtualAllocate(const VirtualAllocationCreateInfo& createInfo,
                                                                                                                                               VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> const & offset) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VirtualAllocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaVirtualAllocate(m_virtualBlock, reinterpret_cast<const VmaVirtualAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaVirtualAllocation*>(&allocation), reinterpret_cast<VkDeviceSize*>(static_cast<VULKAN_HPP_NAMESPACE::DeviceSize*>(offset))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::VirtualBlock::virtualAllocate");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, allocation);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaVirtualAllocate, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueVirtualAllocation>::type VirtualBlock::virtualAllocateUnique(const VirtualAllocationCreateInfo& createInfo,
                                                                                                                                                           VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> const & offset) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VirtualAllocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaVirtualAllocate(m_virtualBlock, reinterpret_cast<const VmaVirtualAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaVirtualAllocation*>(&allocation), reinterpret_cast<VkDeviceSize*>(static_cast<VULKAN_HPP_NAMESPACE::DeviceSize*>(offset))) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::VirtualBlock::virtualAllocateUnique");
    UniqueVirtualAllocation allocationUnique { allocation, *this };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(allocationUnique));
  }
#endif
#endif
  // wrapper function for command vmaVirtualAllocate, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result VirtualBlock::virtualAllocate(const VirtualAllocationCreateInfo* pCreateInfo,
                                                                                                    VirtualAllocation* pAllocation,
                                                                                                    VULKAN_HPP_NAMESPACE::DeviceSize* pOffset) const VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaVirtualAllocate(m_virtualBlock, reinterpret_cast<const VmaVirtualAllocationCreateInfo*>(pCreateInfo), reinterpret_cast<VmaVirtualAllocation*>(pAllocation), reinterpret_cast<VkDeviceSize*>(pOffset)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaVirtualFree, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::virtualFree(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaVirtualFree(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation));
  }
#else
  // wrapper function for command vmaVirtualFree, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::virtualFree(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaVirtualFree(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaVirtualFree, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (VirtualBlock::free)(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaVirtualFree(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation));
  }
#else
  // wrapper function for command vmaVirtualFree, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (VirtualBlock::free)(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT {
    vmaVirtualFree(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaClearVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::clearVirtualBlock() const VULKAN_HPP_NOEXCEPT {
    vmaClearVirtualBlock(m_virtualBlock);
  }
#else
  // wrapper function for command vmaClearVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::clearVirtualBlock() const VULKAN_HPP_NOEXCEPT {
    vmaClearVirtualBlock(m_virtualBlock);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaSetVirtualAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::setVirtualAllocationUserData(VirtualAllocation allocation,
                                                                    void* userData) const VULKAN_HPP_NOEXCEPT {
    vmaSetVirtualAllocationUserData(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation), userData);
  }
#else
  // wrapper function for command vmaSetVirtualAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::setVirtualAllocationUserData(VirtualAllocation allocation,
                                                                    void* pUserData) const VULKAN_HPP_NOEXCEPT {
    vmaSetVirtualAllocationUserData(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation), pUserData);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaGetVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE Statistics VirtualBlock::getVirtualBlockStatistics() const VULKAN_HPP_NOEXCEPT {
    Statistics stats;
    vmaGetVirtualBlockStatistics(m_virtualBlock, reinterpret_cast<VmaStatistics*>(&stats));
    return stats;
  }
#endif
  // wrapper function for command vmaGetVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::getVirtualBlockStatistics(Statistics* pStats) const VULKAN_HPP_NOEXCEPT {
    vmaGetVirtualBlockStatistics(m_virtualBlock, reinterpret_cast<VmaStatistics*>(pStats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCalculateVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE DetailedStatistics VirtualBlock::calculateVirtualBlockStatistics() const VULKAN_HPP_NOEXCEPT {
    DetailedStatistics stats;
    vmaCalculateVirtualBlockStatistics(m_virtualBlock, reinterpret_cast<VmaDetailedStatistics*>(&stats));
    return stats;
  }
#endif
  // wrapper function for command vmaCalculateVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::calculateVirtualBlockStatistics(DetailedStatistics* pStats) const VULKAN_HPP_NOEXCEPT {
    vmaCalculateVirtualBlockStatistics(m_virtualBlock, reinterpret_cast<VmaDetailedStatistics*>(pStats));
  }

#if VMA_STATS_STRING_ENABLED
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaBuildVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE char* VirtualBlock::buildVirtualBlockStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT {
    char* pStatsString;
    vmaBuildVirtualBlockStatsString(m_virtualBlock, &pStatsString, static_cast<VkBool32>(detailedMap));
    return pStatsString;
  }
#endif
  // wrapper function for command vmaBuildVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::buildVirtualBlockStatsString(char** ppStatsString,
                                                                    VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT {
    vmaBuildVirtualBlockStatsString(m_virtualBlock, ppStatsString, static_cast<VkBool32>(detailedMap));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFreeVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::freeVirtualBlockStatsString(char* statsString) const VULKAN_HPP_NOEXCEPT {
    vmaFreeVirtualBlockStatsString(m_virtualBlock, statsString);
  }
#else
  // wrapper function for command vmaFreeVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void VirtualBlock::freeVirtualBlockStatsString(char* pStatsString) const VULKAN_HPP_NOEXCEPT {
    vmaFreeVirtualBlockStatsString(m_virtualBlock, pStatsString);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaFreeVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (VirtualBlock::free)(char* statsString) const VULKAN_HPP_NOEXCEPT {
    vmaFreeVirtualBlockStatsString(m_virtualBlock, statsString);
  }
#else
  // wrapper function for command vmaFreeVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_INLINE void (VirtualBlock::free)(char* pStatsString) const VULKAN_HPP_NOEXCEPT {
    vmaFreeVirtualBlockStatsString(m_virtualBlock, pStatsString);
  }
#endif
#endif 

#ifdef VOLK_HEADER_VERSION
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaImportVulkanFunctionsFromVolk, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VulkanFunctions>::type importVulkanFunctionsFromVolk(const AllocatorCreateInfo& allocatorCreateInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VulkanFunctions dstVulkanFunctions;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaImportVulkanFunctionsFromVolk(reinterpret_cast<const VmaAllocatorCreateInfo*>(&allocatorCreateInfo), reinterpret_cast<VmaVulkanFunctions*>(&dstVulkanFunctions)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::importVulkanFunctionsFromVolk");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, dstVulkanFunctions);
  }
#endif
  // wrapper function for command vmaImportVulkanFunctionsFromVolk, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result importVulkanFunctionsFromVolk(const AllocatorCreateInfo* pAllocatorCreateInfo,
                                                                                                    VulkanFunctions* pDstVulkanFunctions) VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaImportVulkanFunctionsFromVolk(reinterpret_cast<const VmaAllocatorCreateInfo*>(pAllocatorCreateInfo), reinterpret_cast<VmaVulkanFunctions*>(pDstVulkanFunctions)) );
    return result;
  }

#endif
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocator>::type createAllocator(const AllocatorCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Allocator allocator;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAllocator(reinterpret_cast<const VmaAllocatorCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocator*>(&allocator)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::createAllocator");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, allocator);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocator>::type createAllocatorUnique(const AllocatorCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    Allocator allocator;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAllocator(reinterpret_cast<const VmaAllocatorCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocator*>(&allocator)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::createAllocatorUnique");
    UniqueAllocator allocatorUnique { allocator, {} };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(allocatorUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreateAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result createAllocator(const AllocatorCreateInfo* pCreateInfo,
                                                                                      Allocator* pAllocator) VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAllocator(reinterpret_cast<const VmaAllocatorCreateInfo*>(pCreateInfo), reinterpret_cast<VmaAllocator*>(pAllocator)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualBlock>::type createVirtualBlock(const VirtualBlockCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VirtualBlock virtualBlock;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateVirtualBlock(reinterpret_cast<const VmaVirtualBlockCreateInfo*>(&createInfo), reinterpret_cast<VmaVirtualBlock*>(&virtualBlock)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::createVirtualBlock");
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, virtualBlock);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueVirtualBlock>::type createVirtualBlockUnique(const VirtualBlockCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS {
    VirtualBlock virtualBlock;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateVirtualBlock(reinterpret_cast<const VmaVirtualBlockCreateInfo*>(&createInfo), reinterpret_cast<VmaVirtualBlock*>(&virtualBlock)) );
    VULKAN_HPP_NAMESPACE::detail::resultCheck(result, VMA_HPP_NAMESPACE_STRING "::createVirtualBlockUnique");
    UniqueVirtualBlock virtualBlockUnique { virtualBlock, {} };
    return VULKAN_HPP_NAMESPACE::detail::createResultValueType(result, std::move(virtualBlockUnique));
  }
#endif
#endif
  // wrapper function for command vmaCreateVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result createVirtualBlock(const VirtualBlockCreateInfo* pCreateInfo,
                                                                                         VirtualBlock* pVirtualBlock) VULKAN_HPP_NOEXCEPT {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateVirtualBlock(reinterpret_cast<const VmaVirtualBlockCreateInfo*>(pCreateInfo), reinterpret_cast<VmaVirtualBlock*>(pVirtualBlock)) );
    return result;
  }
}
#endif