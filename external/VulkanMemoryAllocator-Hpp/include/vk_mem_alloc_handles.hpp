// Generated from the Vulkan Memory Allocator (vk_mem_alloc.h).
#ifndef VULKAN_MEMORY_ALLOCATOR_HANDLES_HPP
#define VULKAN_MEMORY_ALLOCATOR_HANDLES_HPP

namespace VMA_HPP_NAMESPACE {
  struct DeviceMemoryCallbacks;
  struct VulkanFunctions;
  struct AllocatorCreateInfo;
  struct AllocatorInfo;
  struct Statistics;
  struct DetailedStatistics;
  struct TotalStatistics;
  struct Budget;
  struct AllocationCreateInfo;
  struct PoolCreateInfo;
  struct AllocationInfo;
  struct AllocationInfo2;
  struct DefragmentationInfo;
  struct DefragmentationMove;
  struct DefragmentationPassMoveInfo;
  struct DefragmentationStats;
  struct VirtualBlockCreateInfo;
  struct VirtualAllocationCreateInfo;
  struct VirtualAllocationInfo;
  class Allocator;
  class Pool;
  class Allocation;
  class DefragmentationContext;
  class VirtualAllocation;
  class VirtualBlock;

  namespace detail { class Dispatcher; } // VMA dispatcher is a no-op.
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  using UniqueBuffer = VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Buffer, detail::Dispatcher>;
  using UniqueImage = VULKAN_HPP_NAMESPACE::UniqueHandle<VULKAN_HPP_NAMESPACE::Image, detail::Dispatcher>;
  using UniqueAllocator = VULKAN_HPP_NAMESPACE::UniqueHandle<Allocator, detail::Dispatcher>;
  using UniquePool = VULKAN_HPP_NAMESPACE::UniqueHandle<Pool, detail::Dispatcher>;
  using UniqueAllocation = VULKAN_HPP_NAMESPACE::UniqueHandle<Allocation, detail::Dispatcher>;
  using UniqueVirtualAllocation = VULKAN_HPP_NAMESPACE::UniqueHandle<VirtualAllocation, detail::Dispatcher>;
  using UniqueVirtualBlock = VULKAN_HPP_NAMESPACE::UniqueHandle<VirtualBlock, detail::Dispatcher>;
#endif
  using VULKAN_HPP_NAMESPACE::exchange;

  // wrapper class for handle VmaAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocator.html
  class Allocator {
  public:
    using CType      = VmaAllocator;
    using NativeType = VmaAllocator;

  public:
    VULKAN_HPP_CONSTEXPR Allocator() VULKAN_HPP_NOEXCEPT = default;

    Allocator(Allocator const & rhs)             = default;
    Allocator & operator=(Allocator const & rhs) = default;

#if !defined( VULKAN_HPP_HANDLES_MOVE_EXCHANGE )
    Allocator(Allocator && rhs)             = default;
    Allocator & operator=(Allocator && rhs) = default;
#else
    Allocator(Allocator && rhs) VULKAN_HPP_NOEXCEPT : m_allocator(exchange(rhs.m_allocator, {})) {}
    Allocator & operator=(Allocator && rhs) VULKAN_HPP_NOEXCEPT {
      m_allocator = exchange(rhs.m_allocator, {});
      return *this;
    }
#endif

    VULKAN_HPP_CONSTEXPR         Allocator(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT Allocator(VmaAllocator allocator) VULKAN_HPP_NOEXCEPT : m_allocator(allocator) {}

#if (VULKAN_HPP_TYPESAFE_CONVERSION == 1)
    Allocator& operator=(VmaAllocator allocator) VULKAN_HPP_NOEXCEPT {
      m_allocator = allocator;
      return *this;
    }
#endif

    Allocator& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_allocator = {};
      return *this;
    }

    VULKAN_HPP_TYPESAFE_EXPLICIT operator VmaAllocator() const VULKAN_HPP_NOEXCEPT {
      return m_allocator;
    }

    explicit operator bool() const VULKAN_HPP_NOEXCEPT {
      return m_allocator != VK_NULL_HANDLE;
    }

    bool operator!() const VULKAN_HPP_NOEXCEPT {
      return m_allocator == VK_NULL_HANDLE;
    }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaDestroyAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy() const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaDestroyAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy() const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetAllocatorInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD AllocatorInfo getAllocatorInfo() const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetAllocatorInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getAllocatorInfo(AllocatorInfo* pAllocatorInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetPhysicalDeviceProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties* getPhysicalDeviceProperties() const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetPhysicalDeviceProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getPhysicalDeviceProperties(const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties** ppPhysicalDeviceProperties) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties* getMemoryProperties() const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getMemoryProperties(const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties** ppPhysicalDeviceMemoryProperties) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetMemoryTypeProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::MemoryPropertyFlags getMemoryTypeProperties(uint32_t memoryTypeIndex) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetMemoryTypeProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getMemoryTypeProperties(uint32_t memoryTypeIndex,
                                 VULKAN_HPP_NAMESPACE::MemoryPropertyFlags* pFlags) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaSetCurrentFrameIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setCurrentFrameIndex(uint32_t frameIndex) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaSetCurrentFrameIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setCurrentFrameIndex(uint32_t frameIndex) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCalculateStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD TotalStatistics calculateStatistics() const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaCalculateStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void calculateStatistics(TotalStatistics* pStats) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetHeapBudgets, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    template <typename BudgetAllocator = std::allocator<Budget>>
    VULKAN_HPP_NODISCARD std::vector<Budget, BudgetAllocator> getHeapBudgets() const VULKAN_HPP_NOEXCEPT;
    // wrapper function for command vmaGetHeapBudgets, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    template <typename BudgetAllocator,
              typename std::enable_if<std::is_same<typename BudgetAllocator::value_type, Budget>::value, int>::type = 0>
    VULKAN_HPP_NODISCARD std::vector<Budget, BudgetAllocator> getHeapBudgets(BudgetAllocator& budgetAllocator) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetHeapBudgets, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getHeapBudgets(Budget* pBudgets) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFindMemoryTypeIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type findMemoryTypeIndex(uint32_t memoryTypeBits,
                                                                                                            const AllocationCreateInfo& allocationCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaFindMemoryTypeIndex, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result findMemoryTypeIndex(uint32_t memoryTypeBits,
                                                                          const AllocationCreateInfo* pAllocationCreateInfo,
                                                                          uint32_t* pMemoryTypeIndex) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFindMemoryTypeIndexForBufferInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                         const AllocationCreateInfo& allocationCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaFindMemoryTypeIndexForBufferInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                                       const AllocationCreateInfo* pAllocationCreateInfo,
                                                                                       uint32_t* pMemoryTypeIndex) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFindMemoryTypeIndexForImageInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                        const AllocationCreateInfo& allocationCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaFindMemoryTypeIndexForImageInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo* pImageCreateInfo,
                                                                                      const AllocationCreateInfo* pAllocationCreateInfo,
                                                                                      uint32_t* pMemoryTypeIndex) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCreatePool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Pool>::type createPool(const PoolCreateInfo& createInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaCreatePool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniquePool>::type createPoolUnique(const PoolCreateInfo& createInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaCreatePool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createPool(const PoolCreateInfo* pCreateInfo,
                                                                 Pool* pPool) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaDestroyPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroyPool(Pool pool) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaDestroyPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroyPool(Pool pool) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaDestroyPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy(Pool pool) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaDestroyPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy(Pool pool) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetPoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD Statistics getPoolStatistics(Pool pool) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetPoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getPoolStatistics(Pool pool,
                           Statistics* pPoolStats) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCalculatePoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD DetailedStatistics calculatePoolStatistics(Pool pool) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaCalculatePoolStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void calculatePoolStatistics(Pool pool,
                                 DetailedStatistics* pPoolStats) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCheckPoolCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type checkPoolCorruption(Pool pool) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaCheckPoolCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result checkPoolCorruption(Pool pool) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD const char* getPoolName(Pool pool) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getPoolName(Pool pool,
                     const char** ppName) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaSetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setPoolName(Pool pool,
                     const char* name) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaSetPoolName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setPoolName(Pool pool,
                     const char* pName) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaAllocateMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements& vkMemoryRequirements,
                                                                                                         const AllocationCreateInfo& createInfo,
                                                                                                         VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaAllocateMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type allocateMemoryUnique(const VULKAN_HPP_NAMESPACE::MemoryRequirements& vkMemoryRequirements,
                                                                                                                     const AllocationCreateInfo& createInfo,
                                                                                                                     VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaAllocateMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements* pVkMemoryRequirements,
                                                                     const AllocationCreateInfo* pCreateInfo,
                                                                     Allocation* pAllocation,
                                                                     AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    template <typename AllocationAllocator = std::allocator<Allocation>>
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation, AllocationAllocator>>::type allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> const & vkMemoryRequirements,
                                                                                                                                                VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> const & createInfo,
                                                                                                                                                VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
    // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    template <typename AllocationAllocator,
              typename std::enable_if<std::is_same<typename AllocationAllocator::value_type, Allocation>::value, int>::type = 0>
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation, AllocationAllocator>>::type allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> const & vkMemoryRequirements,
                                                                                                                                                VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> const & createInfo,
                                                                                                                                                VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> const & allocationInfo,
                                                                                                                                                AllocationAllocator& allocationAllocator) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    template <typename AllocationAllocator = std::allocator<UniqueAllocation>>
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<UniqueAllocation, AllocationAllocator>>::type allocateMemoryPagesUnique(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> const & vkMemoryRequirements,
                                                                                                                                                            VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> const & createInfo,
                                                                                                                                                            VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
    // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    template <typename AllocationAllocator,
              typename std::enable_if<std::is_same<typename AllocationAllocator::value_type, UniqueAllocation>::value, int>::type = 0>
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<UniqueAllocation, AllocationAllocator>>::type allocateMemoryPagesUnique(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> const & vkMemoryRequirements,
                                                                                                                                                            VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> const & createInfo,
                                                                                                                                                            VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> const & allocationInfo,
                                                                                                                                                            AllocationAllocator& allocationAllocator) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaAllocateMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result allocateMemoryPages(const VULKAN_HPP_NAMESPACE::MemoryRequirements* pVkMemoryRequirements,
                                                                          const AllocationCreateInfo* pCreateInfo,
                                                                          size_t allocationCount,
                                                                          Allocation* pAllocations,
                                                                          AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaAllocateMemoryForBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                  const AllocationCreateInfo& createInfo,
                                                                                                                  VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaAllocateMemoryForBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type allocateMemoryForBufferUnique(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                              const AllocationCreateInfo& createInfo,
                                                                                                                              VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaAllocateMemoryForBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                              const AllocationCreateInfo* pCreateInfo,
                                                                              Allocation* pAllocation,
                                                                              AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaAllocateMemoryForImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                 const AllocationCreateInfo& createInfo,
                                                                                                                 VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaAllocateMemoryForImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type allocateMemoryForImageUnique(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                             const AllocationCreateInfo& createInfo,
                                                                                                                             VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaAllocateMemoryForImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                                                                             const AllocationCreateInfo* pCreateInfo,
                                                                             Allocation* pAllocation,
                                                                             AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFreeMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void freeMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaFreeMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void freeMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFreeMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaFreeMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFreeMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void freeMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaFreeMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void freeMemoryPages(size_t allocationCount,
                         const Allocation* pAllocations) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFreeMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaFreeMemoryPages, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(size_t allocationCount,
                const Allocation* pAllocations) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD AllocationInfo getAllocationInfo(Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getAllocationInfo(Allocation allocation,
                           AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetAllocationInfo2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD AllocationInfo2 getAllocationInfo2(Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetAllocationInfo2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getAllocationInfo2(Allocation allocation,
                            AllocationInfo2* pAllocationInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaSetAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setAllocationUserData(Allocation allocation,
                               void* userData) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaSetAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setAllocationUserData(Allocation allocation,
                               void* pUserData) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaSetAllocationName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setAllocationName(Allocation allocation,
                           const char* name) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaSetAllocationName, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setAllocationName(Allocation allocation,
                           const char* pName) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetAllocationMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::MemoryPropertyFlags getAllocationMemoryProperties(Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetAllocationMemoryProperties, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getAllocationMemoryProperties(Allocation allocation,
                                       VULKAN_HPP_NAMESPACE::MemoryPropertyFlags* pFlags) const VULKAN_HPP_NOEXCEPT;

#if VMA_EXTERNAL_MEMORY_WIN32
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetMemoryWin32Handle, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<HANDLE>::type getMemoryWin32Handle(Allocation allocation,
                                                                                                           HANDLE hTargetProcess) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaGetMemoryWin32Handle, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result getMemoryWin32Handle(Allocation allocation,
                                                                           HANDLE hTargetProcess,
                                                                           HANDLE* pHandle) const VULKAN_HPP_NOEXCEPT;

#endif 
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaMapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<void*>::type mapMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaMapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result mapMemory(Allocation allocation,
                                                                void** ppData) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaUnmapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void unmapMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaUnmapMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void unmapMemory(Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFlushAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flushAllocation(Allocation allocation,
                                                                                                                       VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                                       VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaFlushAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result flushAllocation(Allocation allocation,
                                                                      VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                      VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaInvalidateAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidateAllocation(Allocation allocation,
                                                                                                                            VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                                                            VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaInvalidateAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result invalidateAllocation(Allocation allocation,
                                                                           VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                           VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFlushAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flushAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations,
                                                                                                                        VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & offsets,
                                                                                                                        VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & sizes) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaFlushAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result flushAllocations(uint32_t allocationCount,
                                                                       const Allocation* allocations,
                                                                       const VULKAN_HPP_NAMESPACE::DeviceSize* offsets,
                                                                       const VULKAN_HPP_NAMESPACE::DeviceSize* sizes) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaInvalidateAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidateAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> const & allocations,
                                                                                                                             VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & offsets,
                                                                                                                             VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> const & sizes) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaInvalidateAllocations, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result invalidateAllocations(uint32_t allocationCount,
                                                                            const Allocation* allocations,
                                                                            const VULKAN_HPP_NAMESPACE::DeviceSize* offsets,
                                                                            const VULKAN_HPP_NAMESPACE::DeviceSize* sizes) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCopyMemoryToAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type copyMemoryToAllocation(const void* srcHostPointer,
                                                                                                                              Allocation dstAllocation,
                                                                                                                              VULKAN_HPP_NAMESPACE::DeviceSize dstAllocationLocalOffset,
                                                                                                                              VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaCopyMemoryToAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result copyMemoryToAllocation(const void* pSrcHostPointer,
                                                                             Allocation dstAllocation,
                                                                             VULKAN_HPP_NAMESPACE::DeviceSize dstAllocationLocalOffset,
                                                                             VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCopyAllocationToMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type copyAllocationToMemory(Allocation srcAllocation,
                                                                                                                              VULKAN_HPP_NAMESPACE::DeviceSize srcAllocationLocalOffset,
                                                                                                                              void* dstHostPointer,
                                                                                                                              VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaCopyAllocationToMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result copyAllocationToMemory(Allocation srcAllocation,
                                                                             VULKAN_HPP_NAMESPACE::DeviceSize srcAllocationLocalOffset,
                                                                             void* pDstHostPointer,
                                                                             VULKAN_HPP_NAMESPACE::DeviceSize size) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCheckCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type checkCorruption(uint32_t memoryTypeBits) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaCheckCorruption, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result checkCorruption(uint32_t memoryTypeBits) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaBeginDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationContext>::type beginDefragmentation(const DefragmentationInfo& info) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaBeginDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result beginDefragmentation(const DefragmentationInfo* pInfo,
                                                                           DefragmentationContext* pContext) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaEndDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void endDefragmentation(DefragmentationContext context,
                            VULKAN_HPP_NAMESPACE::Optional<DefragmentationStats> const & stats = nullptr) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaEndDefragmentation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void endDefragmentation(DefragmentationContext context,
                            DefragmentationStats* pStats) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaBeginDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValue<DefragmentationPassMoveInfo> beginDefragmentationPass(DefragmentationContext context) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaBeginDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result beginDefragmentationPass(DefragmentationContext context,
                                                                               DefragmentationPassMoveInfo* pPassInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaEndDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::ResultValue<DefragmentationPassMoveInfo> endDefragmentationPass(DefragmentationContext context) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
    // wrapper function for command vmaEndDefragmentationPass, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result endDefragmentationPass(DefragmentationContext context,
                                                                             DefragmentationPassMoveInfo* pPassInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaBindBufferMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindBufferMemory(Allocation allocation,
                                                                                                                        VULKAN_HPP_NAMESPACE::Buffer buffer) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaBindBufferMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result bindBufferMemory(Allocation allocation,
                                                                       VULKAN_HPP_NAMESPACE::Buffer buffer) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaBindBufferMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindBufferMemory2(Allocation allocation,
                                                                                                                         VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                         VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                         const void* next) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaBindBufferMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result bindBufferMemory2(Allocation allocation,
                                                                        VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                        VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                        const void* pNext) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaBindImageMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindImageMemory(Allocation allocation,
                                                                                                                       VULKAN_HPP_NAMESPACE::Image image) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaBindImageMemory, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result bindImageMemory(Allocation allocation,
                                                                      VULKAN_HPP_NAMESPACE::Image image) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaBindImageMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindImageMemory2(Allocation allocation,
                                                                                                                        VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                        VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                        const void* next) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#else
    // wrapper function for command vmaBindImageMemory2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result bindImageMemory2(Allocation allocation,
                                                                       VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                       VULKAN_HPP_NAMESPACE::Image image,
                                                                       const void* pNext) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCreateBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<Allocation, VULKAN_HPP_NAMESPACE::Buffer>>::type createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaCreateBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueAllocation, UniqueBuffer>>::type createBufferUnique(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                            const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                            VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaCreateBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                   const AllocationCreateInfo* pAllocationCreateInfo,
                                                                   VULKAN_HPP_NAMESPACE::Buffer* pBuffer,
                                                                   Allocation* pAllocation,
                                                                   AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCreateBufferWithAlignment, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<Allocation, VULKAN_HPP_NAMESPACE::Buffer>>::type createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                             const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                             VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                                                                             VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaCreateBufferWithAlignment, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueAllocation, UniqueBuffer>>::type createBufferWithAlignmentUnique(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                         const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                         VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                                                                         VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaCreateBufferWithAlignment, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                                const AllocationCreateInfo* pAllocationCreateInfo,
                                                                                VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                VULKAN_HPP_NAMESPACE::Buffer* pBuffer,
                                                                                Allocation* pAllocation,
                                                                                AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCreateAliasingBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Buffer>::type createAliasingBuffer(Allocation allocation,
                                                                                                                                 const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaCreateAliasingBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueBuffer>::type createAliasingBufferUnique(Allocation allocation,
                                                                                                                       const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaCreateAliasingBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createAliasingBuffer(Allocation allocation,
                                                                           const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                           VULKAN_HPP_NAMESPACE::Buffer* pBuffer) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCreateAliasingBuffer2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Buffer>::type createAliasingBuffer2(Allocation allocation,
                                                                                                                                  VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                  const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaCreateAliasingBuffer2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueBuffer>::type createAliasingBuffer2Unique(Allocation allocation,
                                                                                                                        VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                        const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaCreateAliasingBuffer2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createAliasingBuffer2(Allocation allocation,
                                                                            VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                            const VULKAN_HPP_NAMESPACE::BufferCreateInfo* pBufferCreateInfo,
                                                                            VULKAN_HPP_NAMESPACE::Buffer* pBuffer) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaDestroyBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroyBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                       Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaDestroyBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroyBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                       Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaDestroyBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy(VULKAN_HPP_NAMESPACE::Buffer buffer,
                 Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaDestroyBuffer, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy(VULKAN_HPP_NAMESPACE::Buffer buffer,
                 Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCreateImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<Allocation, VULKAN_HPP_NAMESPACE::Image>>::type createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                              const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                              VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaCreateImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueAllocation, UniqueImage>>::type createImageUnique(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                          const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                          VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> const & allocationInfo = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaCreateImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo* pImageCreateInfo,
                                                                  const AllocationCreateInfo* pAllocationCreateInfo,
                                                                  VULKAN_HPP_NAMESPACE::Image* pImage,
                                                                  Allocation* pAllocation,
                                                                  AllocationInfo* pAllocationInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCreateAliasingImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Image>::type createAliasingImage(Allocation allocation,
                                                                                                                               const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaCreateAliasingImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueImage>::type createAliasingImageUnique(Allocation allocation,
                                                                                                                     const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaCreateAliasingImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createAliasingImage(Allocation allocation,
                                                                          const VULKAN_HPP_NAMESPACE::ImageCreateInfo* pImageCreateInfo,
                                                                          VULKAN_HPP_NAMESPACE::Image* pImage) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCreateAliasingImage2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Image>::type createAliasingImage2(Allocation allocation,
                                                                                                                                VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                                const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaCreateAliasingImage2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueImage>::type createAliasingImage2Unique(Allocation allocation,
                                                                                                                      VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                                                      const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaCreateAliasingImage2, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createAliasingImage2(Allocation allocation,
                                                                           VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                           const VULKAN_HPP_NAMESPACE::ImageCreateInfo* pImageCreateInfo,
                                                                           VULKAN_HPP_NAMESPACE::Image* pImage) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaDestroyImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroyImage(VULKAN_HPP_NAMESPACE::Image image,
                      Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaDestroyImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroyImage(VULKAN_HPP_NAMESPACE::Image image,
                      Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaDestroyImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy(VULKAN_HPP_NAMESPACE::Image image,
                 Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaDestroyImage, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy(VULKAN_HPP_NAMESPACE::Image image,
                 Allocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif

#if VMA_STATS_STRING_ENABLED
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaBuildStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD char* buildStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaBuildStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void buildStatsString(char** ppStatsString,
                          VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFreeStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void freeStatsString(char* statsString) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaFreeStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void freeStatsString(char* pStatsString) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFreeStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(char* statsString) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaFreeStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(char* pStatsString) const VULKAN_HPP_NOEXCEPT;
#endif
#endif 

  private:
    VmaAllocator m_allocator = {};
  };

  // wrapper class for handle VmaPool, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_pool.html
  class Pool {
  public:
    using CType      = VmaPool;
    using NativeType = VmaPool;

  public:
    VULKAN_HPP_CONSTEXPR Pool() VULKAN_HPP_NOEXCEPT = default;

    Pool(Pool const & rhs)             = default;
    Pool & operator=(Pool const & rhs) = default;

#if !defined( VULKAN_HPP_HANDLES_MOVE_EXCHANGE )
    Pool(Pool && rhs)             = default;
    Pool & operator=(Pool && rhs) = default;
#else
    Pool(Pool && rhs) VULKAN_HPP_NOEXCEPT : m_pool(exchange(rhs.m_pool, {})) {}
    Pool & operator=(Pool && rhs) VULKAN_HPP_NOEXCEPT {
      m_pool = exchange(rhs.m_pool, {});
      return *this;
    }
#endif

    VULKAN_HPP_CONSTEXPR         Pool(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT Pool(VmaPool pool) VULKAN_HPP_NOEXCEPT : m_pool(pool) {}

#if (VULKAN_HPP_TYPESAFE_CONVERSION == 1)
    Pool& operator=(VmaPool pool) VULKAN_HPP_NOEXCEPT {
      m_pool = pool;
      return *this;
    }
#endif

    Pool& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_pool = {};
      return *this;
    }

    VULKAN_HPP_TYPESAFE_EXPLICIT operator VmaPool() const VULKAN_HPP_NOEXCEPT {
      return m_pool;
    }

    explicit operator bool() const VULKAN_HPP_NOEXCEPT {
      return m_pool != VK_NULL_HANDLE;
    }

    bool operator!() const VULKAN_HPP_NOEXCEPT {
      return m_pool == VK_NULL_HANDLE;
    }

  private:
    VmaPool m_pool = {};
  };

  // wrapper class for handle VmaAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_allocation.html
  class Allocation {
  public:
    using CType      = VmaAllocation;
    using NativeType = VmaAllocation;

  public:
    VULKAN_HPP_CONSTEXPR Allocation() VULKAN_HPP_NOEXCEPT = default;

    Allocation(Allocation const & rhs)             = default;
    Allocation & operator=(Allocation const & rhs) = default;

#if !defined( VULKAN_HPP_HANDLES_MOVE_EXCHANGE )
    Allocation(Allocation && rhs)             = default;
    Allocation & operator=(Allocation && rhs) = default;
#else
    Allocation(Allocation && rhs) VULKAN_HPP_NOEXCEPT : m_allocation(exchange(rhs.m_allocation, {})) {}
    Allocation & operator=(Allocation && rhs) VULKAN_HPP_NOEXCEPT {
      m_allocation = exchange(rhs.m_allocation, {});
      return *this;
    }
#endif

    VULKAN_HPP_CONSTEXPR         Allocation(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT Allocation(VmaAllocation allocation) VULKAN_HPP_NOEXCEPT : m_allocation(allocation) {}

#if (VULKAN_HPP_TYPESAFE_CONVERSION == 1)
    Allocation& operator=(VmaAllocation allocation) VULKAN_HPP_NOEXCEPT {
      m_allocation = allocation;
      return *this;
    }
#endif

    Allocation& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_allocation = {};
      return *this;
    }

    VULKAN_HPP_TYPESAFE_EXPLICIT operator VmaAllocation() const VULKAN_HPP_NOEXCEPT {
      return m_allocation;
    }

    explicit operator bool() const VULKAN_HPP_NOEXCEPT {
      return m_allocation != VK_NULL_HANDLE;
    }

    bool operator!() const VULKAN_HPP_NOEXCEPT {
      return m_allocation == VK_NULL_HANDLE;
    }

  private:
    VmaAllocation m_allocation = {};
  };

  // wrapper class for handle VmaDefragmentationContext, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_defragmentation_context.html
  class DefragmentationContext {
  public:
    using CType      = VmaDefragmentationContext;
    using NativeType = VmaDefragmentationContext;

  public:
    VULKAN_HPP_CONSTEXPR DefragmentationContext() VULKAN_HPP_NOEXCEPT = default;

    DefragmentationContext(DefragmentationContext const & rhs)             = default;
    DefragmentationContext & operator=(DefragmentationContext const & rhs) = default;

#if !defined( VULKAN_HPP_HANDLES_MOVE_EXCHANGE )
    DefragmentationContext(DefragmentationContext && rhs)             = default;
    DefragmentationContext & operator=(DefragmentationContext && rhs) = default;
#else
    DefragmentationContext(DefragmentationContext && rhs) VULKAN_HPP_NOEXCEPT : m_defragmentationContext(exchange(rhs.m_defragmentationContext, {})) {}
    DefragmentationContext & operator=(DefragmentationContext && rhs) VULKAN_HPP_NOEXCEPT {
      m_defragmentationContext = exchange(rhs.m_defragmentationContext, {});
      return *this;
    }
#endif

    VULKAN_HPP_CONSTEXPR         DefragmentationContext(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT DefragmentationContext(VmaDefragmentationContext defragmentationContext) VULKAN_HPP_NOEXCEPT : m_defragmentationContext(defragmentationContext) {}

#if (VULKAN_HPP_TYPESAFE_CONVERSION == 1)
    DefragmentationContext& operator=(VmaDefragmentationContext defragmentationContext) VULKAN_HPP_NOEXCEPT {
      m_defragmentationContext = defragmentationContext;
      return *this;
    }
#endif

    DefragmentationContext& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_defragmentationContext = {};
      return *this;
    }

    VULKAN_HPP_TYPESAFE_EXPLICIT operator VmaDefragmentationContext() const VULKAN_HPP_NOEXCEPT {
      return m_defragmentationContext;
    }

    explicit operator bool() const VULKAN_HPP_NOEXCEPT {
      return m_defragmentationContext != VK_NULL_HANDLE;
    }

    bool operator!() const VULKAN_HPP_NOEXCEPT {
      return m_defragmentationContext == VK_NULL_HANDLE;
    }

  private:
    VmaDefragmentationContext m_defragmentationContext = {};
  };

  // wrapper class for handle VmaVirtualAllocation, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_virtual_allocation.html
  class VirtualAllocation {
  public:
    using CType      = VmaVirtualAllocation;
    using NativeType = VmaVirtualAllocation;

  public:
    VULKAN_HPP_CONSTEXPR VirtualAllocation() VULKAN_HPP_NOEXCEPT = default;

    VirtualAllocation(VirtualAllocation const & rhs)             = default;
    VirtualAllocation & operator=(VirtualAllocation const & rhs) = default;

#if !defined( VULKAN_HPP_HANDLES_MOVE_EXCHANGE )
    VirtualAllocation(VirtualAllocation && rhs)             = default;
    VirtualAllocation & operator=(VirtualAllocation && rhs) = default;
#else
    VirtualAllocation(VirtualAllocation && rhs) VULKAN_HPP_NOEXCEPT : m_virtualAllocation(exchange(rhs.m_virtualAllocation, {})) {}
    VirtualAllocation & operator=(VirtualAllocation && rhs) VULKAN_HPP_NOEXCEPT {
      m_virtualAllocation = exchange(rhs.m_virtualAllocation, {});
      return *this;
    }
#endif

    VULKAN_HPP_CONSTEXPR         VirtualAllocation(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT VirtualAllocation(VmaVirtualAllocation virtualAllocation) VULKAN_HPP_NOEXCEPT : m_virtualAllocation(virtualAllocation) {}

#if (VULKAN_HPP_TYPESAFE_CONVERSION == 1)
    VirtualAllocation& operator=(VmaVirtualAllocation virtualAllocation) VULKAN_HPP_NOEXCEPT {
      m_virtualAllocation = virtualAllocation;
      return *this;
    }
#endif

    VirtualAllocation& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_virtualAllocation = {};
      return *this;
    }

    VULKAN_HPP_TYPESAFE_EXPLICIT operator VmaVirtualAllocation() const VULKAN_HPP_NOEXCEPT {
      return m_virtualAllocation;
    }

    explicit operator bool() const VULKAN_HPP_NOEXCEPT {
      return m_virtualAllocation != VK_NULL_HANDLE;
    }

    bool operator!() const VULKAN_HPP_NOEXCEPT {
      return m_virtualAllocation == VK_NULL_HANDLE;
    }

  private:
    VmaVirtualAllocation m_virtualAllocation = {};
  };

  // wrapper class for handle VmaVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/struct_vma_virtual_block.html
  class VirtualBlock {
  public:
    using CType      = VmaVirtualBlock;
    using NativeType = VmaVirtualBlock;

  public:
    VULKAN_HPP_CONSTEXPR VirtualBlock() VULKAN_HPP_NOEXCEPT = default;

    VirtualBlock(VirtualBlock const & rhs)             = default;
    VirtualBlock & operator=(VirtualBlock const & rhs) = default;

#if !defined( VULKAN_HPP_HANDLES_MOVE_EXCHANGE )
    VirtualBlock(VirtualBlock && rhs)             = default;
    VirtualBlock & operator=(VirtualBlock && rhs) = default;
#else
    VirtualBlock(VirtualBlock && rhs) VULKAN_HPP_NOEXCEPT : m_virtualBlock(exchange(rhs.m_virtualBlock, {})) {}
    VirtualBlock & operator=(VirtualBlock && rhs) VULKAN_HPP_NOEXCEPT {
      m_virtualBlock = exchange(rhs.m_virtualBlock, {});
      return *this;
    }
#endif

    VULKAN_HPP_CONSTEXPR         VirtualBlock(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT VirtualBlock(VmaVirtualBlock virtualBlock) VULKAN_HPP_NOEXCEPT : m_virtualBlock(virtualBlock) {}

#if (VULKAN_HPP_TYPESAFE_CONVERSION == 1)
    VirtualBlock& operator=(VmaVirtualBlock virtualBlock) VULKAN_HPP_NOEXCEPT {
      m_virtualBlock = virtualBlock;
      return *this;
    }
#endif

    VirtualBlock& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_virtualBlock = {};
      return *this;
    }

    VULKAN_HPP_TYPESAFE_EXPLICIT operator VmaVirtualBlock() const VULKAN_HPP_NOEXCEPT {
      return m_virtualBlock;
    }

    explicit operator bool() const VULKAN_HPP_NOEXCEPT {
      return m_virtualBlock != VK_NULL_HANDLE;
    }

    bool operator!() const VULKAN_HPP_NOEXCEPT {
      return m_virtualBlock == VK_NULL_HANDLE;
    }

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaDestroyVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy() const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaDestroyVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void destroy() const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaIsVirtualBlockEmpty, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Bool32 isVirtualBlockEmpty() const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaIsVirtualBlockEmpty, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Bool32 isVirtualBlockEmpty() const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetVirtualAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VirtualAllocationInfo getVirtualAllocationInfo(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetVirtualAllocationInfo, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getVirtualAllocationInfo(VirtualAllocation allocation,
                                  VirtualAllocationInfo* pVirtualAllocInfo) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaVirtualAllocate, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualAllocation>::type virtualAllocate(const VirtualAllocationCreateInfo& createInfo,
                                                                                                                 VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> const & offset = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    // wrapper function for command vmaVirtualAllocate, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueVirtualAllocation>::type virtualAllocateUnique(const VirtualAllocationCreateInfo& createInfo,
                                                                                                                             VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> const & offset = nullptr) const VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
    // wrapper function for command vmaVirtualAllocate, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result virtualAllocate(const VirtualAllocationCreateInfo* pCreateInfo,
                                                                      VirtualAllocation* pAllocation,
                                                                      VULKAN_HPP_NAMESPACE::DeviceSize* pOffset) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaVirtualFree, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void virtualFree(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaVirtualFree, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void virtualFree(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaVirtualFree, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaVirtualFree, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(VirtualAllocation allocation) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaClearVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void clearVirtualBlock() const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaClearVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void clearVirtualBlock() const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaSetVirtualAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setVirtualAllocationUserData(VirtualAllocation allocation,
                                      void* userData) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaSetVirtualAllocationUserData, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void setVirtualAllocationUserData(VirtualAllocation allocation,
                                      void* pUserData) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaGetVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD Statistics getVirtualBlockStatistics() const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaGetVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void getVirtualBlockStatistics(Statistics* pStats) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaCalculateVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD DetailedStatistics calculateVirtualBlockStatistics() const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaCalculateVirtualBlockStatistics, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void calculateVirtualBlockStatistics(DetailedStatistics* pStats) const VULKAN_HPP_NOEXCEPT;

#if VMA_STATS_STRING_ENABLED
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaBuildVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    VULKAN_HPP_NODISCARD char* buildVirtualBlockStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT;
#endif
    // wrapper function for command vmaBuildVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void buildVirtualBlockStatsString(char** ppStatsString,
                                      VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFreeVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void freeVirtualBlockStatsString(char* statsString) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaFreeVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void freeVirtualBlockStatsString(char* pStatsString) const VULKAN_HPP_NOEXCEPT;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    // wrapper function for command vmaFreeVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(char* statsString) const VULKAN_HPP_NOEXCEPT;
#else
    // wrapper function for command vmaFreeVirtualBlockStatsString, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
    void (free)(char* pStatsString) const VULKAN_HPP_NOEXCEPT;
#endif
#endif 

  private:
    VmaVirtualBlock m_virtualBlock = {};
  };

#ifdef VOLK_HEADER_VERSION
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaImportVulkanFunctionsFromVolk, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VulkanFunctions>::type importVulkanFunctionsFromVolk(const AllocatorCreateInfo& allocatorCreateInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
  // wrapper function for command vmaImportVulkanFunctionsFromVolk, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result importVulkanFunctionsFromVolk(const AllocatorCreateInfo* pAllocatorCreateInfo,
                                                                                  VulkanFunctions* pDstVulkanFunctions) VULKAN_HPP_NOEXCEPT;

#endif
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocator>::type createAllocator(const AllocatorCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocator>::type createAllocatorUnique(const AllocatorCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
  // wrapper function for command vmaCreateAllocator, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createAllocator(const AllocatorCreateInfo* pCreateInfo,
                                                                    Allocator* pAllocator) VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  // wrapper function for command vmaCreateVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualBlock>::type createVirtualBlock(const VirtualBlockCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  // wrapper function for command vmaCreateVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueVirtualBlock>::type createVirtualBlockUnique(const VirtualBlockCreateInfo& createInfo) VULKAN_HPP_NOEXCEPT_WHEN_NO_EXCEPTIONS;
#endif
#endif
  // wrapper function for command vmaCreateVirtualBlock, see https://gpuopen-librariesandsdks.github.io/VulkanMemoryAllocator/html/globals_func.html
  VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createVirtualBlock(const VirtualBlockCreateInfo* pCreateInfo,
                                                                       VirtualBlock* pVirtualBlock) VULKAN_HPP_NOEXCEPT;

#ifndef VULKAN_HPP_NO_SMART_HANDLE
  namespace detail {
    template <typename OwnerType> class UniqueBase {
    public:
      UniqueBase() = default;
      UniqueBase(OwnerType owner) VULKAN_HPP_NOEXCEPT : m_owner(owner) {}
      OwnerType getOwner() const VULKAN_HPP_NOEXCEPT { return m_owner; }
    private:
      OwnerType m_owner = {};
    };
    template <> class UniqueBase<void> {};
  }
#endif
}

namespace VULKAN_HPP_NAMESPACE {
  template <> struct isVulkanHandleType<VMA_HPP_NAMESPACE::Allocator> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool value = true;
  };
  template <> struct isVulkanHandleType<VMA_HPP_NAMESPACE::Pool> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool value = true;
  };
  template <> struct isVulkanHandleType<VMA_HPP_NAMESPACE::Allocation> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool value = true;
  };
  template <> struct isVulkanHandleType<VMA_HPP_NAMESPACE::DefragmentationContext> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool value = true;
  };
  template <> struct isVulkanHandleType<VMA_HPP_NAMESPACE::VirtualAllocation> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool value = true;
  };
  template <> struct isVulkanHandleType<VMA_HPP_NAMESPACE::VirtualBlock> {
    static VULKAN_HPP_CONST_OR_CONSTEXPR bool value = true;
  };

#if (VK_USE_64_BIT_PTR_DEFINES == 1)
  template <> struct CppType<VmaAllocator, VK_NULL_HANDLE> {
    using Type = VMA_HPP_NAMESPACE::Allocator;
  };
  template <> struct CppType<VmaPool, VK_NULL_HANDLE> {
    using Type = VMA_HPP_NAMESPACE::Pool;
  };
  template <> struct CppType<VmaAllocation, VK_NULL_HANDLE> {
    using Type = VMA_HPP_NAMESPACE::Allocation;
  };
  template <> struct CppType<VmaDefragmentationContext, VK_NULL_HANDLE> {
    using Type = VMA_HPP_NAMESPACE::DefragmentationContext;
  };
  template <> struct CppType<VmaVirtualAllocation, VK_NULL_HANDLE> {
    using Type = VMA_HPP_NAMESPACE::VirtualAllocation;
  };
  template <> struct CppType<VmaVirtualBlock, VK_NULL_HANDLE> {
    using Type = VMA_HPP_NAMESPACE::VirtualBlock;
  };
#endif

#ifndef VULKAN_HPP_NO_SMART_HANDLE
  template <> class UniqueHandleTraits<Buffer, VMA_HPP_NAMESPACE::detail::Dispatcher> {
  public:
    class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<VMA_HPP_NAMESPACE::Allocator> {
    public:
      using UniqueBase::UniqueBase;
    protected:
      template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
        vmaDestroyBuffer(static_cast<VmaAllocator>(getOwner()), static_cast<VkBuffer>(t), nullptr);
      }
    };
  };
  template <> class UniqueHandleTraits<Image, VMA_HPP_NAMESPACE::detail::Dispatcher> {
  public:
    class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<VMA_HPP_NAMESPACE::Allocator> {
    public:
      using UniqueBase::UniqueBase;
    protected:
      template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
        vmaDestroyImage(static_cast<VmaAllocator>(getOwner()), static_cast<VkImage>(t), nullptr);
      }
    };
  };
  template <> class UniqueHandleTraits<VMA_HPP_NAMESPACE::Allocator, VMA_HPP_NAMESPACE::detail::Dispatcher> {
  public:
    class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<void> {
    public:
      using UniqueBase::UniqueBase;
    protected:
      template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
        t.destroy();
      }
    };
  };
  template <> class UniqueHandleTraits<VMA_HPP_NAMESPACE::Pool, VMA_HPP_NAMESPACE::detail::Dispatcher> {
  public:
    class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<VMA_HPP_NAMESPACE::Allocator> {
    public:
      using UniqueBase::UniqueBase;
    protected:
      template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
        getOwner().destroyPool(t);
      }
    };
  };
  template <> class UniqueHandleTraits<VMA_HPP_NAMESPACE::Allocation, VMA_HPP_NAMESPACE::detail::Dispatcher> {
  public:
    class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<VMA_HPP_NAMESPACE::Allocator> {
    public:
      using UniqueBase::UniqueBase;
    protected:
      template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
        getOwner().freeMemory(t);
      }
    };
  };
  template <> class UniqueHandleTraits<VMA_HPP_NAMESPACE::VirtualAllocation, VMA_HPP_NAMESPACE::detail::Dispatcher> {
  public:
    class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<VMA_HPP_NAMESPACE::VirtualBlock> {
    public:
      using UniqueBase::UniqueBase;
    protected:
      template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
        getOwner().virtualFree(t);
      }
    };
  };
  template <> class UniqueHandleTraits<VMA_HPP_NAMESPACE::VirtualBlock, VMA_HPP_NAMESPACE::detail::Dispatcher> {
  public:
    class deleter : public VMA_HPP_NAMESPACE::detail::UniqueBase<void> {
    public:
      using UniqueBase::UniqueBase;
    protected:
      template <typename T> void destroy(T t) VULKAN_HPP_NOEXCEPT {
        t.destroy();
      }
    };
  };
#endif
}
#endif