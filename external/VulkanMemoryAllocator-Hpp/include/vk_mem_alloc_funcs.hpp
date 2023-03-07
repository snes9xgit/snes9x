#ifndef VULKAN_MEMORY_ALLOCATOR_FUNCS_HPP
#define VULKAN_MEMORY_ALLOCATOR_FUNCS_HPP

namespace VMA_HPP_NAMESPACE {

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::destroy() const {
    vmaDestroyAllocator(m_allocator);
  }
#else
  VULKAN_HPP_INLINE void Allocator::destroy() const {
    vmaDestroyAllocator(m_allocator);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE AllocatorInfo Allocator::getAllocatorInfo() const {
    AllocatorInfo allocatorInfo;
    vmaGetAllocatorInfo(m_allocator, reinterpret_cast<VmaAllocatorInfo*>(&allocatorInfo));
    return allocatorInfo;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::getAllocatorInfo(AllocatorInfo* allocatorInfo) const {
    vmaGetAllocatorInfo(m_allocator, reinterpret_cast<VmaAllocatorInfo*>(allocatorInfo));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties* Allocator::getPhysicalDeviceProperties() const {
    const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties* physicalDeviceProperties;
    vmaGetPhysicalDeviceProperties(m_allocator, reinterpret_cast<const VkPhysicalDeviceProperties**>(&physicalDeviceProperties));
    return physicalDeviceProperties;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::getPhysicalDeviceProperties(const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties** physicalDeviceProperties) const {
    vmaGetPhysicalDeviceProperties(m_allocator, reinterpret_cast<const VkPhysicalDeviceProperties**>(physicalDeviceProperties));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties* Allocator::getMemoryProperties() const {
    const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties* physicalDeviceMemoryProperties;
    vmaGetMemoryProperties(m_allocator, reinterpret_cast<const VkPhysicalDeviceMemoryProperties**>(&physicalDeviceMemoryProperties));
    return physicalDeviceMemoryProperties;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::getMemoryProperties(const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties** physicalDeviceMemoryProperties) const {
    vmaGetMemoryProperties(m_allocator, reinterpret_cast<const VkPhysicalDeviceMemoryProperties**>(physicalDeviceMemoryProperties));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::MemoryPropertyFlags Allocator::getMemoryTypeProperties(uint32_t memoryTypeIndex) const {
    VULKAN_HPP_NAMESPACE::MemoryPropertyFlags flags;
    vmaGetMemoryTypeProperties(m_allocator, memoryTypeIndex, reinterpret_cast<VkMemoryPropertyFlags*>(&flags));
    return flags;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::getMemoryTypeProperties(uint32_t memoryTypeIndex,
                                                            VULKAN_HPP_NAMESPACE::MemoryPropertyFlags* flags) const {
    vmaGetMemoryTypeProperties(m_allocator, memoryTypeIndex, reinterpret_cast<VkMemoryPropertyFlags*>(flags));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::setCurrentFrameIndex(uint32_t frameIndex) const {
    vmaSetCurrentFrameIndex(m_allocator, frameIndex);
  }
#else
  VULKAN_HPP_INLINE void Allocator::setCurrentFrameIndex(uint32_t frameIndex) const {
    vmaSetCurrentFrameIndex(m_allocator, frameIndex);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE TotalStatistics Allocator::calculateStatistics() const {
    TotalStatistics stats;
    vmaCalculateStatistics(m_allocator, reinterpret_cast<VmaTotalStatistics*>(&stats));
    return stats;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::calculateStatistics(TotalStatistics* stats) const {
    vmaCalculateStatistics(m_allocator, reinterpret_cast<VmaTotalStatistics*>(stats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  template<typename VectorAllocator,
           typename B,
           typename std::enable_if<std::is_same<typename B::value_type, Budget>::value, int>::type>
  VULKAN_HPP_INLINE std::vector<Budget, VectorAllocator> Allocator::getHeapBudgets(VectorAllocator& vectorAllocator) const {
    std::vector<Budget, VectorAllocator> budgets(getMemoryProperties()->memoryHeapCount, vectorAllocator);
    vmaGetHeapBudgets(m_allocator, reinterpret_cast<VmaBudget*>(budgets.data()));
    return budgets;
  }

  template<typename VectorAllocator>
  VULKAN_HPP_INLINE std::vector<Budget, VectorAllocator> Allocator::getHeapBudgets() const {
    std::vector<Budget, VectorAllocator> budgets(getMemoryProperties()->memoryHeapCount);
    vmaGetHeapBudgets(m_allocator, reinterpret_cast<VmaBudget*>(budgets.data()));
    return budgets;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::getHeapBudgets(Budget* budgets) const {
    vmaGetHeapBudgets(m_allocator, reinterpret_cast<VmaBudget*>(budgets));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type Allocator::findMemoryTypeIndex(uint32_t memoryTypeBits,
                                                                                                                  const AllocationCreateInfo& allocationCreateInfo) const {
    uint32_t memoryTypeIndex;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndex(m_allocator, memoryTypeBits, reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), &memoryTypeIndex) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::findMemoryTypeIndex");
    return createResultValueType(result, memoryTypeIndex);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::findMemoryTypeIndex(uint32_t memoryTypeBits,
                                                                                const AllocationCreateInfo* allocationCreateInfo,
                                                                                uint32_t* memoryTypeIndex) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndex(m_allocator, memoryTypeBits, reinterpret_cast<const VmaAllocationCreateInfo*>(allocationCreateInfo), memoryTypeIndex) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type Allocator::findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                               const AllocationCreateInfo& allocationCreateInfo) const {
    uint32_t memoryTypeIndex;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndexForBufferInfo(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), &memoryTypeIndex) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::findMemoryTypeIndexForBufferInfo");
    return createResultValueType(result, memoryTypeIndex);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* bufferCreateInfo,
                                                                                             const AllocationCreateInfo* allocationCreateInfo,
                                                                                             uint32_t* memoryTypeIndex) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndexForBufferInfo(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(allocationCreateInfo), memoryTypeIndex) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type Allocator::findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                              const AllocationCreateInfo& allocationCreateInfo) const {
    uint32_t memoryTypeIndex;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndexForImageInfo(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), &memoryTypeIndex) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::findMemoryTypeIndexForImageInfo");
    return createResultValueType(result, memoryTypeIndex);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo* imageCreateInfo,
                                                                                            const AllocationCreateInfo* allocationCreateInfo,
                                                                                            uint32_t* memoryTypeIndex) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFindMemoryTypeIndexForImageInfo(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(imageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(allocationCreateInfo), memoryTypeIndex) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Pool>::type Allocator::createPool(const PoolCreateInfo& createInfo) const {
    Pool pool;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreatePool(m_allocator, reinterpret_cast<const VmaPoolCreateInfo*>(&createInfo), reinterpret_cast<VmaPool*>(&pool)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createPool");
    return createResultValueType(result, pool);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniquePool>::type Allocator::createPoolUnique(const PoolCreateInfo& createInfo) const {
    Pool pool;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreatePool(m_allocator, reinterpret_cast<const VmaPoolCreateInfo*>(&createInfo), reinterpret_cast<VmaPool*>(&pool)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createPool");
    return createResultValueType(result, createUniqueHandle(pool, this));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createPool(const PoolCreateInfo* createInfo,
                                                                       Pool* pool) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreatePool(m_allocator, reinterpret_cast<const VmaPoolCreateInfo*>(createInfo), reinterpret_cast<VmaPool*>(pool)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::destroyPool(Pool pool) const {
    vmaDestroyPool(m_allocator, static_cast<VmaPool>(pool));
  }
#else
  VULKAN_HPP_INLINE void Allocator::destroyPool(Pool pool) const {
    vmaDestroyPool(m_allocator, static_cast<VmaPool>(pool));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE Statistics Allocator::getPoolStatistics(Pool pool) const {
    Statistics poolStats;
    vmaGetPoolStatistics(m_allocator, static_cast<VmaPool>(pool), reinterpret_cast<VmaStatistics*>(&poolStats));
    return poolStats;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::getPoolStatistics(Pool pool,
                                                      Statistics* poolStats) const {
    vmaGetPoolStatistics(m_allocator, static_cast<VmaPool>(pool), reinterpret_cast<VmaStatistics*>(poolStats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE DetailedStatistics Allocator::calculatePoolStatistics(Pool pool) const {
    DetailedStatistics poolStats;
    vmaCalculatePoolStatistics(m_allocator, static_cast<VmaPool>(pool), reinterpret_cast<VmaDetailedStatistics*>(&poolStats));
    return poolStats;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::calculatePoolStatistics(Pool pool,
                                                            DetailedStatistics* poolStats) const {
    vmaCalculatePoolStatistics(m_allocator, static_cast<VmaPool>(pool), reinterpret_cast<VmaDetailedStatistics*>(poolStats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::checkPoolCorruption(Pool pool) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCheckPoolCorruption(m_allocator, static_cast<VmaPool>(pool)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::checkPoolCorruption");
    return createResultValueType(result);
  }
#else
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::checkPoolCorruption(Pool pool) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCheckPoolCorruption(m_allocator, static_cast<VmaPool>(pool)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE const char* Allocator::getPoolName(Pool pool) const {
    const char* name;
    vmaGetPoolName(m_allocator, static_cast<VmaPool>(pool), &name);
    return name;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::getPoolName(Pool pool,
                                                const char** name) const {
    vmaGetPoolName(m_allocator, static_cast<VmaPool>(pool), name);
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::setPoolName(Pool pool,
                                                const char* name) const {
    vmaSetPoolName(m_allocator, static_cast<VmaPool>(pool), name);
  }
#else
  VULKAN_HPP_INLINE void Allocator::setPoolName(Pool pool,
                                                const char* name) const {
    vmaSetPoolName(m_allocator, static_cast<VmaPool>(pool), name);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type Allocator::allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements& vkMemoryRequirements,
                                                                                                               const AllocationCreateInfo& createInfo,
                                                                                                               VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemory(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(&vkMemoryRequirements), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemory");
    return createResultValueType(result, allocation);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type Allocator::allocateMemoryUnique(const VULKAN_HPP_NAMESPACE::MemoryRequirements& vkMemoryRequirements,
                                                                                                                           const AllocationCreateInfo& createInfo,
                                                                                                                           VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemory(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(&vkMemoryRequirements), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemory");
    return createResultValueType(result, createUniqueHandle(allocation, this));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements* vkMemoryRequirements,
                                                                           const AllocationCreateInfo* createInfo,
                                                                           Allocation* allocation,
                                                                           AllocationInfo* allocationInfo) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemory(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo), reinterpret_cast<VmaAllocation*>(allocation), reinterpret_cast<VmaAllocationInfo*>(allocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  template<typename VectorAllocator,
           typename B,
           typename std::enable_if<std::is_same<typename B::value_type, Allocation>::value, int>::type>
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation, VectorAllocator>>::type Allocator::allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                                                                                                                                                  VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> createInfo,
                                                                                                                                                  VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> allocationInfo,
                                                                                                                                                  VectorAllocator& vectorAllocator) const {
    size_t allocationCount = vkMemoryRequirements.size();
    std::vector<Allocation, VectorAllocator> allocations(allocationCount, vectorAllocator);
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements.data()), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo.data()), allocationCount, reinterpret_cast<VmaAllocation*>(allocations.data()), reinterpret_cast<VmaAllocationInfo*>(allocationInfo.data())) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages");
    return createResultValueType(result, allocations);
  }

  template<typename VectorAllocator>
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation, VectorAllocator>>::type Allocator::allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                                                                                                                                                  VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> createInfo,
                                                                                                                                                  VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> allocationInfo) const {
    size_t allocationCount = vkMemoryRequirements.size();
    std::vector<Allocation, VectorAllocator> allocations(allocationCount);
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements.data()), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo.data()), allocationCount, reinterpret_cast<VmaAllocation*>(allocations.data()), reinterpret_cast<VmaAllocationInfo*>(allocationInfo.data())) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages");
    return createResultValueType(result, allocations);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  template<typename VectorAllocator,
           typename B,
           typename std::enable_if<std::is_same<typename B::value_type, UniqueAllocation>::value, int>::type>
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<UniqueAllocation, VectorAllocator>>::type Allocator::allocateMemoryPagesUnique(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                                                                                                                                                              VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> createInfo,
                                                                                                                                                              VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> allocationInfo,
                                                                                                                                                              VectorAllocator& vectorAllocator) const {
    size_t allocationCount = vkMemoryRequirements.size();
    std::vector<Allocation> allocations(allocationCount);
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements.data()), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo.data()), allocationCount, reinterpret_cast<VmaAllocation*>(allocations.data()), reinterpret_cast<VmaAllocationInfo*>(allocationInfo.data())) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages");
    return createResultValueType(result, createUniqueHandleVector(allocations, this, vectorAllocator));
  }

  template<typename VectorAllocator>
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<UniqueAllocation, VectorAllocator>>::type Allocator::allocateMemoryPagesUnique(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                                                                                                                                                              VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> createInfo,
                                                                                                                                                              VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> allocationInfo) const {
    size_t allocationCount = vkMemoryRequirements.size();
    std::vector<Allocation> allocations(allocationCount);
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements.data()), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo.data()), allocationCount, reinterpret_cast<VmaAllocation*>(allocations.data()), reinterpret_cast<VmaAllocationInfo*>(allocationInfo.data())) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryPages");
    return createResultValueType(result, createUniqueHandleVector(allocations, this, VectorAllocator()));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::allocateMemoryPages(const VULKAN_HPP_NAMESPACE::MemoryRequirements* vkMemoryRequirements,
                                                                                const AllocationCreateInfo* createInfo,
                                                                                size_t allocationCount,
                                                                                Allocation* allocations,
                                                                                AllocationInfo* allocationInfo) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryPages(m_allocator, reinterpret_cast<const VkMemoryRequirements*>(vkMemoryRequirements), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo), allocationCount, reinterpret_cast<VmaAllocation*>(allocations), reinterpret_cast<VmaAllocationInfo*>(allocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type Allocator::allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                        const AllocationCreateInfo& createInfo,
                                                                                                                        VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForBuffer(m_allocator, static_cast<VkBuffer>(buffer), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryForBuffer");
    return createResultValueType(result, allocation);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type Allocator::allocateMemoryForBufferUnique(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                                    const AllocationCreateInfo& createInfo,
                                                                                                                                    VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForBuffer(m_allocator, static_cast<VkBuffer>(buffer), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryForBuffer");
    return createResultValueType(result, createUniqueHandle(allocation, this));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                    const AllocationCreateInfo* createInfo,
                                                                                    Allocation* allocation,
                                                                                    AllocationInfo* allocationInfo) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForBuffer(m_allocator, static_cast<VkBuffer>(buffer), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo), reinterpret_cast<VmaAllocation*>(allocation), reinterpret_cast<VmaAllocationInfo*>(allocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type Allocator::allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                       const AllocationCreateInfo& createInfo,
                                                                                                                       VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForImage(m_allocator, static_cast<VkImage>(image), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryForImage");
    return createResultValueType(result, allocation);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type Allocator::allocateMemoryForImageUnique(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                                   const AllocationCreateInfo& createInfo,
                                                                                                                                   VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    Allocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForImage(m_allocator, static_cast<VkImage>(image), reinterpret_cast<const VmaAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::allocateMemoryForImage");
    return createResultValueType(result, createUniqueHandle(allocation, this));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                                                                                   const AllocationCreateInfo* createInfo,
                                                                                   Allocation* allocation,
                                                                                   AllocationInfo* allocationInfo) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaAllocateMemoryForImage(m_allocator, static_cast<VkImage>(image), reinterpret_cast<const VmaAllocationCreateInfo*>(createInfo), reinterpret_cast<VmaAllocation*>(allocation), reinterpret_cast<VmaAllocationInfo*>(allocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::freeMemory(const Allocation allocation) const {
    vmaFreeMemory(m_allocator, static_cast<const VmaAllocation>(allocation));
  }
#else
  VULKAN_HPP_INLINE void Allocator::freeMemory(const Allocation allocation) const {
    vmaFreeMemory(m_allocator, static_cast<const VmaAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::freeMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> allocations) const {
    size_t allocationCount = allocations.size();
    vmaFreeMemoryPages(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations.data()));
  }
#endif
  VULKAN_HPP_INLINE void Allocator::freeMemoryPages(size_t allocationCount,
                                                    const Allocation* allocations) const {
    vmaFreeMemoryPages(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE AllocationInfo Allocator::getAllocationInfo(Allocation allocation) const {
    AllocationInfo allocationInfo;
    vmaGetAllocationInfo(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VmaAllocationInfo*>(&allocationInfo));
    return allocationInfo;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::getAllocationInfo(Allocation allocation,
                                                      AllocationInfo* allocationInfo) const {
    vmaGetAllocationInfo(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VmaAllocationInfo*>(allocationInfo));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::setAllocationUserData(Allocation allocation,
                                                          void* userData) const {
    vmaSetAllocationUserData(m_allocator, static_cast<VmaAllocation>(allocation), userData);
  }
#else
  VULKAN_HPP_INLINE void Allocator::setAllocationUserData(Allocation allocation,
                                                          void* userData) const {
    vmaSetAllocationUserData(m_allocator, static_cast<VmaAllocation>(allocation), userData);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::setAllocationName(Allocation allocation,
                                                      const char* name) const {
    vmaSetAllocationName(m_allocator, static_cast<VmaAllocation>(allocation), name);
  }
#else
  VULKAN_HPP_INLINE void Allocator::setAllocationName(Allocation allocation,
                                                      const char* name) const {
    vmaSetAllocationName(m_allocator, static_cast<VmaAllocation>(allocation), name);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::MemoryPropertyFlags Allocator::getAllocationMemoryProperties(Allocation allocation) const {
    VULKAN_HPP_NAMESPACE::MemoryPropertyFlags flags;
    vmaGetAllocationMemoryProperties(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VkMemoryPropertyFlags*>(&flags));
    return flags;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::getAllocationMemoryProperties(Allocation allocation,
                                                                  VULKAN_HPP_NAMESPACE::MemoryPropertyFlags* flags) const {
    vmaGetAllocationMemoryProperties(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<VkMemoryPropertyFlags*>(flags));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void*>::type Allocator::mapMemory(Allocation allocation) const {
    void* data;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaMapMemory(m_allocator, static_cast<VmaAllocation>(allocation), &data) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::mapMemory");
    return createResultValueType(result, data);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::mapMemory(Allocation allocation,
                                                                      void** data) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaMapMemory(m_allocator, static_cast<VmaAllocation>(allocation), data) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::unmapMemory(Allocation allocation) const {
    vmaUnmapMemory(m_allocator, static_cast<VmaAllocation>(allocation));
  }
#else
  VULKAN_HPP_INLINE void Allocator::unmapMemory(Allocation allocation) const {
    vmaUnmapMemory(m_allocator, static_cast<VmaAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::flushAllocation(Allocation allocation,
                                                                                                          VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                          VULKAN_HPP_NAMESPACE::DeviceSize size) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFlushAllocation(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::flushAllocation");
    return createResultValueType(result);
  }
#else
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::flushAllocation(Allocation allocation,
                                                                            VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                            VULKAN_HPP_NAMESPACE::DeviceSize size) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFlushAllocation(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::invalidateAllocation(Allocation allocation,
                                                                                                               VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                               VULKAN_HPP_NAMESPACE::DeviceSize size) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaInvalidateAllocation(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::invalidateAllocation");
    return createResultValueType(result);
  }
#else
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::invalidateAllocation(Allocation allocation,
                                                                                 VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                 VULKAN_HPP_NAMESPACE::DeviceSize size) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaInvalidateAllocation(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(offset), static_cast<VkDeviceSize>(size)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::flushAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> allocations,
                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> offsets,
                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> sizes) const {
    uint32_t allocationCount = allocations.size();
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFlushAllocations(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations.data()), reinterpret_cast<const VkDeviceSize*>(offsets.data()), reinterpret_cast<const VkDeviceSize*>(sizes.data())) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::flushAllocations");
    return createResultValueType(result);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::flushAllocations(uint32_t allocationCount,
                                                                             const Allocation* allocations,
                                                                             const VULKAN_HPP_NAMESPACE::DeviceSize* offsets,
                                                                             const VULKAN_HPP_NAMESPACE::DeviceSize* sizes) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaFlushAllocations(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations), reinterpret_cast<const VkDeviceSize*>(offsets), reinterpret_cast<const VkDeviceSize*>(sizes)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::invalidateAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> allocations,
                                                                                                                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> offsets,
                                                                                                                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> sizes) const {
    uint32_t allocationCount = allocations.size();
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaInvalidateAllocations(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations.data()), reinterpret_cast<const VkDeviceSize*>(offsets.data()), reinterpret_cast<const VkDeviceSize*>(sizes.data())) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::invalidateAllocations");
    return createResultValueType(result);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::invalidateAllocations(uint32_t allocationCount,
                                                                                  const Allocation* allocations,
                                                                                  const VULKAN_HPP_NAMESPACE::DeviceSize* offsets,
                                                                                  const VULKAN_HPP_NAMESPACE::DeviceSize* sizes) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaInvalidateAllocations(m_allocator, allocationCount, reinterpret_cast<const VmaAllocation*>(allocations), reinterpret_cast<const VkDeviceSize*>(offsets), reinterpret_cast<const VkDeviceSize*>(sizes)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::checkCorruption(uint32_t memoryTypeBits) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCheckCorruption(m_allocator, memoryTypeBits) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::checkCorruption");
    return createResultValueType(result);
  }
#else
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::checkCorruption(uint32_t memoryTypeBits) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCheckCorruption(m_allocator, memoryTypeBits) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationContext>::type Allocator::beginDefragmentation(const DefragmentationInfo& info) const {
    DefragmentationContext context;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBeginDefragmentation(m_allocator, reinterpret_cast<const VmaDefragmentationInfo*>(&info), reinterpret_cast<VmaDefragmentationContext*>(&context)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::beginDefragmentation");
    return createResultValueType(result, context);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::beginDefragmentation(const DefragmentationInfo* info,
                                                                                 DefragmentationContext* context) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBeginDefragmentation(m_allocator, reinterpret_cast<const VmaDefragmentationInfo*>(info), reinterpret_cast<VmaDefragmentationContext*>(context)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::endDefragmentation(DefragmentationContext context,
                                                       VULKAN_HPP_NAMESPACE::Optional<DefragmentationStats> stats) const {
    vmaEndDefragmentation(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationStats*>(static_cast<DefragmentationStats*>(stats)));
  }
#endif
  VULKAN_HPP_INLINE void Allocator::endDefragmentation(DefragmentationContext context,
                                                       DefragmentationStats* stats) const {
    vmaEndDefragmentation(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationStats*>(stats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationPassMoveInfo>::type Allocator::beginDefragmentationPass(DefragmentationContext context) const {
    DefragmentationPassMoveInfo passInfo;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBeginDefragmentationPass(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationPassMoveInfo*>(&passInfo)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::beginDefragmentationPass");
    return createResultValueType(result, passInfo);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::beginDefragmentationPass(DefragmentationContext context,
                                                                                     DefragmentationPassMoveInfo* passInfo) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBeginDefragmentationPass(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationPassMoveInfo*>(passInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationPassMoveInfo>::type Allocator::endDefragmentationPass(DefragmentationContext context) const {
    DefragmentationPassMoveInfo passInfo;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaEndDefragmentationPass(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationPassMoveInfo*>(&passInfo)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::endDefragmentationPass");
    return createResultValueType(result, passInfo);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::endDefragmentationPass(DefragmentationContext context,
                                                                                   DefragmentationPassMoveInfo* passInfo) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaEndDefragmentationPass(m_allocator, static_cast<VmaDefragmentationContext>(context), reinterpret_cast<VmaDefragmentationPassMoveInfo*>(passInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::bindBufferMemory(Allocation allocation,
                                                                                                           VULKAN_HPP_NAMESPACE::Buffer buffer) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindBufferMemory(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkBuffer>(buffer)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::bindBufferMemory");
    return createResultValueType(result);
  }
#else
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::bindBufferMemory(Allocation allocation,
                                                                             VULKAN_HPP_NAMESPACE::Buffer buffer) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindBufferMemory(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkBuffer>(buffer)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::bindBufferMemory2(Allocation allocation,
                                                                                                            VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                            VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                            const void* next) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindBufferMemory2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), static_cast<VkBuffer>(buffer), next) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::bindBufferMemory2");
    return createResultValueType(result);
  }
#else
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::bindBufferMemory2(Allocation allocation,
                                                                              VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                              VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                              const void* next) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindBufferMemory2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), static_cast<VkBuffer>(buffer), next) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::bindImageMemory(Allocation allocation,
                                                                                                          VULKAN_HPP_NAMESPACE::Image image) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindImageMemory(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkImage>(image)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::bindImageMemory");
    return createResultValueType(result);
  }
#else
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::bindImageMemory(Allocation allocation,
                                                                            VULKAN_HPP_NAMESPACE::Image image) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindImageMemory(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkImage>(image)) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type Allocator::bindImageMemory2(Allocation allocation,
                                                                                                           VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                           VULKAN_HPP_NAMESPACE::Image image,
                                                                                                           const void* next) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindImageMemory2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), static_cast<VkImage>(image), next) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::bindImageMemory2");
    return createResultValueType(result);
  }
#else
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::bindImageMemory2(Allocation allocation,
                                                                             VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                             VULKAN_HPP_NAMESPACE::Image image,
                                                                             const void* next) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaBindImageMemory2(m_allocator, static_cast<VmaAllocation>(allocation), static_cast<VkDeviceSize>(allocationLocalOffset), static_cast<VkImage>(image), next) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<VULKAN_HPP_NAMESPACE::Buffer, Allocation>>::type Allocator::createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                      const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                      VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    std::pair<VULKAN_HPP_NAMESPACE::Buffer, Allocation> pair;
    VULKAN_HPP_NAMESPACE::Buffer& buffer = pair.first;
    Allocation& allocation = pair.second;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBuffer(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), reinterpret_cast<VkBuffer*>(&buffer), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createBuffer");
    return createResultValueType(result, pair);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueBuffer, UniqueAllocation>>::type Allocator::createBufferUnique(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                  const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                  VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    std::pair<VULKAN_HPP_NAMESPACE::Buffer, Allocation> pair;
    VULKAN_HPP_NAMESPACE::Buffer& buffer = pair.first;
    Allocation& allocation = pair.second;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBuffer(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), reinterpret_cast<VkBuffer*>(&buffer), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createBuffer");
    return createResultValueType(result, createUniqueHandle(pair, this));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* bufferCreateInfo,
                                                                         const AllocationCreateInfo* allocationCreateInfo,
                                                                         VULKAN_HPP_NAMESPACE::Buffer* buffer,
                                                                         Allocation* allocation,
                                                                         AllocationInfo* allocationInfo) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBuffer(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(allocationCreateInfo), reinterpret_cast<VkBuffer*>(buffer), reinterpret_cast<VmaAllocation*>(allocation), reinterpret_cast<VmaAllocationInfo*>(allocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<VULKAN_HPP_NAMESPACE::Buffer, Allocation>>::type Allocator::createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                                   const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                   VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                                                                                   VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    std::pair<VULKAN_HPP_NAMESPACE::Buffer, Allocation> pair;
    VULKAN_HPP_NAMESPACE::Buffer& buffer = pair.first;
    Allocation& allocation = pair.second;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBufferWithAlignment(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), static_cast<VkDeviceSize>(minAlignment), reinterpret_cast<VkBuffer*>(&buffer), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createBufferWithAlignment");
    return createResultValueType(result, pair);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueBuffer, UniqueAllocation>>::type Allocator::createBufferWithAlignmentUnique(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                               const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                               VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                                                                               VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    std::pair<VULKAN_HPP_NAMESPACE::Buffer, Allocation> pair;
    VULKAN_HPP_NAMESPACE::Buffer& buffer = pair.first;
    Allocation& allocation = pair.second;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBufferWithAlignment(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), static_cast<VkDeviceSize>(minAlignment), reinterpret_cast<VkBuffer*>(&buffer), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createBufferWithAlignment");
    return createResultValueType(result, createUniqueHandle(pair, this));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* bufferCreateInfo,
                                                                                      const AllocationCreateInfo* allocationCreateInfo,
                                                                                      VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                      VULKAN_HPP_NAMESPACE::Buffer* buffer,
                                                                                      Allocation* allocation,
                                                                                      AllocationInfo* allocationInfo) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateBufferWithAlignment(m_allocator, reinterpret_cast<const VkBufferCreateInfo*>(bufferCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(allocationCreateInfo), static_cast<VkDeviceSize>(minAlignment), reinterpret_cast<VkBuffer*>(buffer), reinterpret_cast<VmaAllocation*>(allocation), reinterpret_cast<VmaAllocationInfo*>(allocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Buffer>::type Allocator::createAliasingBuffer(Allocation allocation,
                                                                                                                                       const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const {
    VULKAN_HPP_NAMESPACE::Buffer buffer;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingBuffer(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkBufferCreateInfo*>(&bufferCreateInfo), reinterpret_cast<VkBuffer*>(&buffer)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingBuffer");
    return createResultValueType(result, buffer);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createAliasingBuffer(Allocation allocation,
                                                                                 const VULKAN_HPP_NAMESPACE::BufferCreateInfo* bufferCreateInfo,
                                                                                 VULKAN_HPP_NAMESPACE::Buffer* buffer) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingBuffer(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkBufferCreateInfo*>(bufferCreateInfo), reinterpret_cast<VkBuffer*>(buffer)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::destroyBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                  Allocation allocation) const {
    vmaDestroyBuffer(m_allocator, static_cast<VkBuffer>(buffer), static_cast<VmaAllocation>(allocation));
  }
#else
  VULKAN_HPP_INLINE void Allocator::destroyBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                  Allocation allocation) const {
    vmaDestroyBuffer(m_allocator, static_cast<VkBuffer>(buffer), static_cast<VmaAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<VULKAN_HPP_NAMESPACE::Image, Allocation>>::type Allocator::createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                                    const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                    VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    std::pair<VULKAN_HPP_NAMESPACE::Image, Allocation> pair;
    VULKAN_HPP_NAMESPACE::Image& image = pair.first;
    Allocation& allocation = pair.second;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), reinterpret_cast<VkImage*>(&image), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createImage");
    return createResultValueType(result, pair);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueImage, UniqueAllocation>>::type Allocator::createImageUnique(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                                const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo) const {
    std::pair<VULKAN_HPP_NAMESPACE::Image, Allocation> pair;
    VULKAN_HPP_NAMESPACE::Image& image = pair.first;
    Allocation& allocation = pair.second;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(&allocationCreateInfo), reinterpret_cast<VkImage*>(&image), reinterpret_cast<VmaAllocation*>(&allocation), reinterpret_cast<VmaAllocationInfo*>(static_cast<AllocationInfo*>(allocationInfo))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createImage");
    return createResultValueType(result, createUniqueHandle(pair, this));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo* imageCreateInfo,
                                                                        const AllocationCreateInfo* allocationCreateInfo,
                                                                        VULKAN_HPP_NAMESPACE::Image* image,
                                                                        Allocation* allocation,
                                                                        AllocationInfo* allocationInfo) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateImage(m_allocator, reinterpret_cast<const VkImageCreateInfo*>(imageCreateInfo), reinterpret_cast<const VmaAllocationCreateInfo*>(allocationCreateInfo), reinterpret_cast<VkImage*>(image), reinterpret_cast<VmaAllocation*>(allocation), reinterpret_cast<VmaAllocationInfo*>(allocationInfo)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Image>::type Allocator::createAliasingImage(Allocation allocation,
                                                                                                                                     const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const {
    VULKAN_HPP_NAMESPACE::Image image;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingImage(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkImageCreateInfo*>(&imageCreateInfo), reinterpret_cast<VkImage*>(&image)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::Allocator::createAliasingImage");
    return createResultValueType(result, image);
  }
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result Allocator::createAliasingImage(Allocation allocation,
                                                                                const VULKAN_HPP_NAMESPACE::ImageCreateInfo* imageCreateInfo,
                                                                                VULKAN_HPP_NAMESPACE::Image* image) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAliasingImage(m_allocator, static_cast<VmaAllocation>(allocation), reinterpret_cast<const VkImageCreateInfo*>(imageCreateInfo), reinterpret_cast<VkImage*>(image)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::destroyImage(VULKAN_HPP_NAMESPACE::Image image,
                                                 Allocation allocation) const {
    vmaDestroyImage(m_allocator, static_cast<VkImage>(image), static_cast<VmaAllocation>(allocation));
  }
#else
  VULKAN_HPP_INLINE void Allocator::destroyImage(VULKAN_HPP_NAMESPACE::Image image,
                                                 Allocation allocation) const {
    vmaDestroyImage(m_allocator, static_cast<VkImage>(image), static_cast<VmaAllocation>(allocation));
  }
#endif

#if VMA_STATS_STRING_ENABLED
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE char* Allocator::buildStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const {
    char* statsString;
    vmaBuildStatsString(m_allocator, &statsString, static_cast<VkBool32>(detailedMap));
    return statsString;
  }
#endif
  VULKAN_HPP_INLINE void Allocator::buildStatsString(char** statsString,
                                                     VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const {
    vmaBuildStatsString(m_allocator, statsString, static_cast<VkBool32>(detailedMap));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void Allocator::freeStatsString(char* statsString) const {
    vmaFreeStatsString(m_allocator, statsString);
  }
#else
  VULKAN_HPP_INLINE void Allocator::freeStatsString(char* statsString) const {
    vmaFreeStatsString(m_allocator, statsString);
  }
#endif

#endif
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void VirtualBlock::destroy() const {
    vmaDestroyVirtualBlock(m_virtualBlock);
  }
#else
  VULKAN_HPP_INLINE void VirtualBlock::destroy() const {
    vmaDestroyVirtualBlock(m_virtualBlock);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Bool32 VirtualBlock::isVirtualBlockEmpty() const {
    VULKAN_HPP_NAMESPACE::Bool32 result = static_cast<VULKAN_HPP_NAMESPACE::Bool32>( vmaIsVirtualBlockEmpty(m_virtualBlock) );
    return result;
  }
#else
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Bool32 VirtualBlock::isVirtualBlockEmpty() const {
    VULKAN_HPP_NAMESPACE::Bool32 result = static_cast<VULKAN_HPP_NAMESPACE::Bool32>( vmaIsVirtualBlockEmpty(m_virtualBlock) );
    return result;
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE VirtualAllocationInfo VirtualBlock::getVirtualAllocationInfo(VirtualAllocation allocation) const {
    VirtualAllocationInfo virtualAllocInfo;
    vmaGetVirtualAllocationInfo(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation), reinterpret_cast<VmaVirtualAllocationInfo*>(&virtualAllocInfo));
    return virtualAllocInfo;
  }
#endif
  VULKAN_HPP_INLINE void VirtualBlock::getVirtualAllocationInfo(VirtualAllocation allocation,
                                                                VirtualAllocationInfo* virtualAllocInfo) const {
    vmaGetVirtualAllocationInfo(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation), reinterpret_cast<VmaVirtualAllocationInfo*>(virtualAllocInfo));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualAllocation>::type VirtualBlock::virtualAllocate(const VirtualAllocationCreateInfo& createInfo,
                                                                                                                          VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> offset) const {
    VirtualAllocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaVirtualAllocate(m_virtualBlock, reinterpret_cast<const VmaVirtualAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaVirtualAllocation*>(&allocation), reinterpret_cast<VkDeviceSize*>(static_cast<VULKAN_HPP_NAMESPACE::DeviceSize*>(offset))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::VirtualBlock::virtualAllocate");
    return createResultValueType(result, allocation);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueVirtualAllocation>::type VirtualBlock::virtualAllocateUnique(const VirtualAllocationCreateInfo& createInfo,
                                                                                                                                      VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> offset) const {
    VirtualAllocation allocation;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaVirtualAllocate(m_virtualBlock, reinterpret_cast<const VmaVirtualAllocationCreateInfo*>(&createInfo), reinterpret_cast<VmaVirtualAllocation*>(&allocation), reinterpret_cast<VkDeviceSize*>(static_cast<VULKAN_HPP_NAMESPACE::DeviceSize*>(offset))) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::VirtualBlock::virtualAllocate");
    return createResultValueType(result, createUniqueHandle(allocation, this));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result VirtualBlock::virtualAllocate(const VirtualAllocationCreateInfo* createInfo,
                                                                               VirtualAllocation* allocation,
                                                                               VULKAN_HPP_NAMESPACE::DeviceSize* offset) const {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaVirtualAllocate(m_virtualBlock, reinterpret_cast<const VmaVirtualAllocationCreateInfo*>(createInfo), reinterpret_cast<VmaVirtualAllocation*>(allocation), reinterpret_cast<VkDeviceSize*>(offset)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void VirtualBlock::virtualFree(VirtualAllocation allocation) const {
    vmaVirtualFree(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation));
  }
#else
  VULKAN_HPP_INLINE void VirtualBlock::virtualFree(VirtualAllocation allocation) const {
    vmaVirtualFree(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation));
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void VirtualBlock::clearVirtualBlock() const {
    vmaClearVirtualBlock(m_virtualBlock);
  }
#else
  VULKAN_HPP_INLINE void VirtualBlock::clearVirtualBlock() const {
    vmaClearVirtualBlock(m_virtualBlock);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void VirtualBlock::setVirtualAllocationUserData(VirtualAllocation allocation,
                                                                    void* userData) const {
    vmaSetVirtualAllocationUserData(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation), userData);
  }
#else
  VULKAN_HPP_INLINE void VirtualBlock::setVirtualAllocationUserData(VirtualAllocation allocation,
                                                                    void* userData) const {
    vmaSetVirtualAllocationUserData(m_virtualBlock, static_cast<VmaVirtualAllocation>(allocation), userData);
  }
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE Statistics VirtualBlock::getVirtualBlockStatistics() const {
    Statistics stats;
    vmaGetVirtualBlockStatistics(m_virtualBlock, reinterpret_cast<VmaStatistics*>(&stats));
    return stats;
  }
#endif
  VULKAN_HPP_INLINE void VirtualBlock::getVirtualBlockStatistics(Statistics* stats) const {
    vmaGetVirtualBlockStatistics(m_virtualBlock, reinterpret_cast<VmaStatistics*>(stats));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE DetailedStatistics VirtualBlock::calculateVirtualBlockStatistics() const {
    DetailedStatistics stats;
    vmaCalculateVirtualBlockStatistics(m_virtualBlock, reinterpret_cast<VmaDetailedStatistics*>(&stats));
    return stats;
  }
#endif
  VULKAN_HPP_INLINE void VirtualBlock::calculateVirtualBlockStatistics(DetailedStatistics* stats) const {
    vmaCalculateVirtualBlockStatistics(m_virtualBlock, reinterpret_cast<VmaDetailedStatistics*>(stats));
  }

#if VMA_STATS_STRING_ENABLED
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE char* VirtualBlock::buildVirtualBlockStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const {
    char* statsString;
    vmaBuildVirtualBlockStatsString(m_virtualBlock, &statsString, static_cast<VkBool32>(detailedMap));
    return statsString;
  }
#endif
  VULKAN_HPP_INLINE void VirtualBlock::buildVirtualBlockStatsString(char** statsString,
                                                                    VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const {
    vmaBuildVirtualBlockStatsString(m_virtualBlock, statsString, static_cast<VkBool32>(detailedMap));
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE void VirtualBlock::freeVirtualBlockStatsString(char* statsString) const {
    vmaFreeVirtualBlockStatsString(m_virtualBlock, statsString);
  }
#else
  VULKAN_HPP_INLINE void VirtualBlock::freeVirtualBlockStatsString(char* statsString) const {
    vmaFreeVirtualBlockStatsString(m_virtualBlock, statsString);
  }
#endif

#endif
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocator>::type createAllocator(const AllocatorCreateInfo& createInfo) {
    Allocator allocator;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAllocator(reinterpret_cast<const VmaAllocatorCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocator*>(&allocator)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::createAllocator");
    return createResultValueType(result, allocator);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocator>::type createAllocatorUnique(const AllocatorCreateInfo& createInfo) {
    Allocator allocator;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAllocator(reinterpret_cast<const VmaAllocatorCreateInfo*>(&createInfo), reinterpret_cast<VmaAllocator*>(&allocator)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::createAllocator");
    return createResultValueType(result, createUniqueHandle(allocator));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result createAllocator(const AllocatorCreateInfo* createInfo,
                                                                 Allocator* allocator) {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateAllocator(reinterpret_cast<const VmaAllocatorCreateInfo*>(createInfo), reinterpret_cast<VmaAllocator*>(allocator)) );
    return result;
  }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualBlock>::type createVirtualBlock(const VirtualBlockCreateInfo& createInfo) {
    VirtualBlock virtualBlock;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateVirtualBlock(reinterpret_cast<const VmaVirtualBlockCreateInfo*>(&createInfo), reinterpret_cast<VmaVirtualBlock*>(&virtualBlock)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::createVirtualBlock");
    return createResultValueType(result, virtualBlock);
  }
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_INLINE typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueVirtualBlock>::type createVirtualBlockUnique(const VirtualBlockCreateInfo& createInfo) {
    VirtualBlock virtualBlock;
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateVirtualBlock(reinterpret_cast<const VmaVirtualBlockCreateInfo*>(&createInfo), reinterpret_cast<VmaVirtualBlock*>(&virtualBlock)) );
    resultCheck(result, VMA_HPP_NAMESPACE_STRING "::createVirtualBlock");
    return createResultValueType(result, createUniqueHandle(virtualBlock));
  }
#endif
#endif
  VULKAN_HPP_INLINE VULKAN_HPP_NAMESPACE::Result createVirtualBlock(const VirtualBlockCreateInfo* createInfo,
                                                                    VirtualBlock* virtualBlock) {
    VULKAN_HPP_NAMESPACE::Result result = static_cast<VULKAN_HPP_NAMESPACE::Result>( vmaCreateVirtualBlock(reinterpret_cast<const VmaVirtualBlockCreateInfo*>(createInfo), reinterpret_cast<VmaVirtualBlock*>(virtualBlock)) );
    return result;
  }
}
#endif
