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
  struct DefragmentationInfo;
  struct DefragmentationMove;
  struct DefragmentationPassMoveInfo;
  struct DefragmentationStats;
  struct VirtualBlockCreateInfo;
  struct VirtualAllocationCreateInfo;
  struct VirtualAllocationInfo;

  class Pool;
  class Allocation;
  class DefragmentationContext;
  class VirtualAllocation;
  class Allocator;
  class VirtualBlock;
}


namespace VMA_HPP_NAMESPACE {
  class Pool {
  public:
    using CType      = VmaPool;
    using NativeType = VmaPool;
  public:
    VULKAN_HPP_CONSTEXPR         Pool() = default;
    VULKAN_HPP_CONSTEXPR         Pool(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT Pool(VmaPool pool) VULKAN_HPP_NOEXCEPT : m_pool(pool) {}

#if defined(VULKAN_HPP_TYPESAFE_CONVERSION)
    Pool& operator=(VmaPool pool) VULKAN_HPP_NOEXCEPT {
      m_pool = pool;
      return *this;
    }
#endif

    Pool& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_pool = {};
      return *this;
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    auto operator<=>(Pool const &) const = default;
#else
    bool operator==(Pool const & rhs) const VULKAN_HPP_NOEXCEPT {
      return m_pool == rhs.m_pool;
    }
#endif

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
  VULKAN_HPP_STATIC_ASSERT(sizeof(Pool) == sizeof(VmaPool),
                           "handle and wrapper have different size!");
}
#ifndef VULKAN_HPP_NO_SMART_HANDLE
namespace VULKAN_HPP_NAMESPACE {
  template<> struct UniqueHandleTraits<VMA_HPP_NAMESPACE::Pool, VMA_HPP_NAMESPACE::Dispatcher> {
    using deleter = VMA_HPP_NAMESPACE::Deleter<VMA_HPP_NAMESPACE::Pool, VMA_HPP_NAMESPACE::Allocator>;
  };
}
namespace VMA_HPP_NAMESPACE { using UniquePool = VULKAN_HPP_NAMESPACE::UniqueHandle<Pool, Dispatcher>; }
#endif

namespace VMA_HPP_NAMESPACE {
  class Allocation {
  public:
    using CType      = VmaAllocation;
    using NativeType = VmaAllocation;
  public:
    VULKAN_HPP_CONSTEXPR         Allocation() = default;
    VULKAN_HPP_CONSTEXPR         Allocation(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT Allocation(VmaAllocation allocation) VULKAN_HPP_NOEXCEPT : m_allocation(allocation) {}

#if defined(VULKAN_HPP_TYPESAFE_CONVERSION)
    Allocation& operator=(VmaAllocation allocation) VULKAN_HPP_NOEXCEPT {
      m_allocation = allocation;
      return *this;
    }
#endif

    Allocation& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_allocation = {};
      return *this;
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    auto operator<=>(Allocation const &) const = default;
#else
    bool operator==(Allocation const & rhs) const VULKAN_HPP_NOEXCEPT {
      return m_allocation == rhs.m_allocation;
    }
#endif

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
  VULKAN_HPP_STATIC_ASSERT(sizeof(Allocation) == sizeof(VmaAllocation),
                           "handle and wrapper have different size!");
}
#ifndef VULKAN_HPP_NO_SMART_HANDLE
namespace VULKAN_HPP_NAMESPACE {
  template<> struct UniqueHandleTraits<VMA_HPP_NAMESPACE::Allocation, VMA_HPP_NAMESPACE::Dispatcher> {
    using deleter = VMA_HPP_NAMESPACE::Deleter<VMA_HPP_NAMESPACE::Allocation, VMA_HPP_NAMESPACE::Allocator>;
  };
}
namespace VMA_HPP_NAMESPACE { using UniqueAllocation = VULKAN_HPP_NAMESPACE::UniqueHandle<Allocation, Dispatcher>; }
#endif

namespace VMA_HPP_NAMESPACE {
  class DefragmentationContext {
  public:
    using CType      = VmaDefragmentationContext;
    using NativeType = VmaDefragmentationContext;
  public:
    VULKAN_HPP_CONSTEXPR         DefragmentationContext() = default;
    VULKAN_HPP_CONSTEXPR         DefragmentationContext(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT DefragmentationContext(VmaDefragmentationContext defragmentationContext) VULKAN_HPP_NOEXCEPT : m_defragmentationContext(defragmentationContext) {}

#if defined(VULKAN_HPP_TYPESAFE_CONVERSION)
    DefragmentationContext& operator=(VmaDefragmentationContext defragmentationContext) VULKAN_HPP_NOEXCEPT {
      m_defragmentationContext = defragmentationContext;
      return *this;
    }
#endif

    DefragmentationContext& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_defragmentationContext = {};
      return *this;
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    auto operator<=>(DefragmentationContext const &) const = default;
#else
    bool operator==(DefragmentationContext const & rhs) const VULKAN_HPP_NOEXCEPT {
      return m_defragmentationContext == rhs.m_defragmentationContext;
    }
#endif

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
  VULKAN_HPP_STATIC_ASSERT(sizeof(DefragmentationContext) == sizeof(VmaDefragmentationContext),
                           "handle and wrapper have different size!");
}
#ifndef VULKAN_HPP_NO_SMART_HANDLE
namespace VULKAN_HPP_NAMESPACE {
  template<> struct UniqueHandleTraits<VMA_HPP_NAMESPACE::DefragmentationContext, VMA_HPP_NAMESPACE::Dispatcher> {
    using deleter = VMA_HPP_NAMESPACE::Deleter<VMA_HPP_NAMESPACE::DefragmentationContext, void>;
  };
}
namespace VMA_HPP_NAMESPACE { using UniqueDefragmentationContext = VULKAN_HPP_NAMESPACE::UniqueHandle<DefragmentationContext, Dispatcher>; }
#endif

namespace VMA_HPP_NAMESPACE {
  class Allocator {
  public:
    using CType      = VmaAllocator;
    using NativeType = VmaAllocator;
  public:
    VULKAN_HPP_CONSTEXPR         Allocator() = default;
    VULKAN_HPP_CONSTEXPR         Allocator(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT Allocator(VmaAllocator allocator) VULKAN_HPP_NOEXCEPT : m_allocator(allocator) {}

#if defined(VULKAN_HPP_TYPESAFE_CONVERSION)
    Allocator& operator=(VmaAllocator allocator) VULKAN_HPP_NOEXCEPT {
      m_allocator = allocator;
      return *this;
    }
#endif

    Allocator& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_allocator = {};
      return *this;
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    auto operator<=>(Allocator const &) const = default;
#else
    bool operator==(Allocator const & rhs) const VULKAN_HPP_NOEXCEPT {
      return m_allocator == rhs.m_allocator;
    }
#endif

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
    void destroy() const;
#else
    void destroy() const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS AllocatorInfo getAllocatorInfo() const;
#endif
    void getAllocatorInfo(AllocatorInfo* allocatorInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties* getPhysicalDeviceProperties() const;
#endif
    void getPhysicalDeviceProperties(const VULKAN_HPP_NAMESPACE::PhysicalDeviceProperties** physicalDeviceProperties) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties* getMemoryProperties() const;
#endif
    void getMemoryProperties(const VULKAN_HPP_NAMESPACE::PhysicalDeviceMemoryProperties** physicalDeviceMemoryProperties) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_NAMESPACE::MemoryPropertyFlags getMemoryTypeProperties(uint32_t memoryTypeIndex) const;
#endif
    void getMemoryTypeProperties(uint32_t memoryTypeIndex,
                                 VULKAN_HPP_NAMESPACE::MemoryPropertyFlags* flags) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void setCurrentFrameIndex(uint32_t frameIndex) const;
#else
    void setCurrentFrameIndex(uint32_t frameIndex) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS TotalStatistics calculateStatistics() const;
#endif
    void calculateStatistics(TotalStatistics* stats) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    template<typename VectorAllocator = std::allocator<Budget>,
             typename B = VectorAllocator,
             typename std::enable_if<std::is_same<typename B::value_type, Budget>::value, int>::type = 0>
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS std::vector<Budget, VectorAllocator> getHeapBudgets(VectorAllocator& vectorAllocator) const;

    template<typename VectorAllocator = std::allocator<Budget>>
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS std::vector<Budget, VectorAllocator> getHeapBudgets() const;
#endif
    void getHeapBudgets(Budget* budgets) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type findMemoryTypeIndex(uint32_t memoryTypeBits,
                                                                                                                               const AllocationCreateInfo& allocationCreateInfo) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result findMemoryTypeIndex(uint32_t memoryTypeBits,
                                                                          const AllocationCreateInfo* allocationCreateInfo,
                                                                          uint32_t* memoryTypeIndex) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                            const AllocationCreateInfo& allocationCreateInfo) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result findMemoryTypeIndexForBufferInfo(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* bufferCreateInfo,
                                                                                       const AllocationCreateInfo* allocationCreateInfo,
                                                                                       uint32_t* memoryTypeIndex) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<uint32_t>::type findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                           const AllocationCreateInfo& allocationCreateInfo) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result findMemoryTypeIndexForImageInfo(const VULKAN_HPP_NAMESPACE::ImageCreateInfo* imageCreateInfo,
                                                                                      const AllocationCreateInfo* allocationCreateInfo,
                                                                                      uint32_t* memoryTypeIndex) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<Pool>::type createPool(const PoolCreateInfo& createInfo) const;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<UniquePool>::type createPoolUnique(const PoolCreateInfo& createInfo) const;
#endif
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createPool(const PoolCreateInfo* createInfo,
                                                                 Pool* pool) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void destroyPool(Pool pool) const;
#else
    void destroyPool(Pool pool) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS Statistics getPoolStatistics(Pool pool) const;
#endif
    void getPoolStatistics(Pool pool,
                           Statistics* poolStats) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS DetailedStatistics calculatePoolStatistics(Pool pool) const;
#endif
    void calculatePoolStatistics(Pool pool,
                                 DetailedStatistics* poolStats) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type checkPoolCorruption(Pool pool) const;
#else
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result checkPoolCorruption(Pool pool) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS const char* getPoolName(Pool pool) const;
#endif
    void getPoolName(Pool pool,
                     const char** name) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void setPoolName(Pool pool,
                     const char* name) const;
#else
    void setPoolName(Pool pool,
                     const char* name) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements& vkMemoryRequirements,
                                                                                                                            const AllocationCreateInfo& createInfo,
                                                                                                                            VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type allocateMemoryUnique(const VULKAN_HPP_NAMESPACE::MemoryRequirements& vkMemoryRequirements,
                                                                                                                                        const AllocationCreateInfo& createInfo,
                                                                                                                                        VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#endif
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result allocateMemory(const VULKAN_HPP_NAMESPACE::MemoryRequirements* vkMemoryRequirements,
                                                                     const AllocationCreateInfo* createInfo,
                                                                     Allocation* allocation,
                                                                     AllocationInfo* allocationInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    template<typename VectorAllocator = std::allocator<Allocation>,
             typename B = VectorAllocator,
             typename std::enable_if<std::is_same<typename B::value_type, Allocation>::value, int>::type = 0>
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation, VectorAllocator>>::type allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                                                                                                                                                               VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> createInfo,
                                                                                                                                                               VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> allocationInfo,
                                                                                                                                                               VectorAllocator& vectorAllocator) const;

    template<typename VectorAllocator = std::allocator<Allocation>>
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<Allocation, VectorAllocator>>::type allocateMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                                                                                                                                                               VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> createInfo,
                                                                                                                                                               VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> allocationInfo = nullptr) const;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    template<typename VectorAllocator = std::allocator<UniqueAllocation>,
             typename B = VectorAllocator,
             typename std::enable_if<std::is_same<typename B::value_type, UniqueAllocation>::value, int>::type = 0>
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<UniqueAllocation, VectorAllocator>>::type allocateMemoryPagesUnique(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                                                                                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> createInfo,
                                                                                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> allocationInfo,
                                                                                                                                                                           VectorAllocator& vectorAllocator) const;

    template<typename VectorAllocator = std::allocator<UniqueAllocation>>
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::vector<UniqueAllocation, VectorAllocator>>::type allocateMemoryPagesUnique(VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::MemoryRequirements> vkMemoryRequirements,
                                                                                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxy<const AllocationCreateInfo> createInfo,
                                                                                                                                                                           VULKAN_HPP_NAMESPACE::ArrayProxyNoTemporaries<AllocationInfo> allocationInfo = nullptr) const;
#endif
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result allocateMemoryPages(const VULKAN_HPP_NAMESPACE::MemoryRequirements* vkMemoryRequirements,
                                                                          const AllocationCreateInfo* createInfo,
                                                                          size_t allocationCount,
                                                                          Allocation* allocations,
                                                                          AllocationInfo* allocationInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                                     const AllocationCreateInfo& createInfo,
                                                                                                                                     VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type allocateMemoryForBufferUnique(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                                                                                 const AllocationCreateInfo& createInfo,
                                                                                                                                                 VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#endif
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result allocateMemoryForBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                              const AllocationCreateInfo* createInfo,
                                                                              Allocation* allocation,
                                                                              AllocationInfo* allocationInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocation>::type allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                                    const AllocationCreateInfo& createInfo,
                                                                                                                                    VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocation>::type allocateMemoryForImageUnique(VULKAN_HPP_NAMESPACE::Image image,
                                                                                                                                                const AllocationCreateInfo& createInfo,
                                                                                                                                                VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#endif
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result allocateMemoryForImage(VULKAN_HPP_NAMESPACE::Image image,
                                                                             const AllocationCreateInfo* createInfo,
                                                                             Allocation* allocation,
                                                                             AllocationInfo* allocationInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void freeMemory(const Allocation allocation) const;
#else
    void freeMemory(const Allocation allocation) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void freeMemoryPages(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> allocations) const;
#endif
    void freeMemoryPages(size_t allocationCount,
                         const Allocation* allocations) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS AllocationInfo getAllocationInfo(Allocation allocation) const;
#endif
    void getAllocationInfo(Allocation allocation,
                           AllocationInfo* allocationInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void setAllocationUserData(Allocation allocation,
                               void* userData) const;
#else
    void setAllocationUserData(Allocation allocation,
                               void* userData) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void setAllocationName(Allocation allocation,
                           const char* name) const;
#else
    void setAllocationName(Allocation allocation,
                           const char* name) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_NAMESPACE::MemoryPropertyFlags getAllocationMemoryProperties(Allocation allocation) const;
#endif
    void getAllocationMemoryProperties(Allocation allocation,
                                       VULKAN_HPP_NAMESPACE::MemoryPropertyFlags* flags) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<void*>::type mapMemory(Allocation allocation) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result mapMemory(Allocation allocation,
                                                                void** data) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void unmapMemory(Allocation allocation) const;
#else
    void unmapMemory(Allocation allocation) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flushAllocation(Allocation allocation,
                                                                               VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                               VULKAN_HPP_NAMESPACE::DeviceSize size) const;
#else
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result flushAllocation(Allocation allocation,
                                                                      VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                      VULKAN_HPP_NAMESPACE::DeviceSize size) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidateAllocation(Allocation allocation,
                                                                                    VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                                    VULKAN_HPP_NAMESPACE::DeviceSize size) const;
#else
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result invalidateAllocation(Allocation allocation,
                                                                           VULKAN_HPP_NAMESPACE::DeviceSize offset,
                                                                           VULKAN_HPP_NAMESPACE::DeviceSize size) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type flushAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> allocations,
                                                                                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> offsets,
                                                                                VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> sizes) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result flushAllocations(uint32_t allocationCount,
                                                                       const Allocation* allocations,
                                                                       const VULKAN_HPP_NAMESPACE::DeviceSize* offsets,
                                                                       const VULKAN_HPP_NAMESPACE::DeviceSize* sizes) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type invalidateAllocations(VULKAN_HPP_NAMESPACE::ArrayProxy<const Allocation> allocations,
                                                                                     VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> offsets,
                                                                                     VULKAN_HPP_NAMESPACE::ArrayProxy<const VULKAN_HPP_NAMESPACE::DeviceSize> sizes) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result invalidateAllocations(uint32_t allocationCount,
                                                                            const Allocation* allocations,
                                                                            const VULKAN_HPP_NAMESPACE::DeviceSize* offsets,
                                                                            const VULKAN_HPP_NAMESPACE::DeviceSize* sizes) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type checkCorruption(uint32_t memoryTypeBits) const;
#else
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result checkCorruption(uint32_t memoryTypeBits) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationContext>::type beginDefragmentation(const DefragmentationInfo& info) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result beginDefragmentation(const DefragmentationInfo* info,
                                                                           DefragmentationContext* context) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void endDefragmentation(DefragmentationContext context,
                            VULKAN_HPP_NAMESPACE::Optional<DefragmentationStats> stats = nullptr) const;
#endif
    void endDefragmentation(DefragmentationContext context,
                            DefragmentationStats* stats) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationPassMoveInfo>::type beginDefragmentationPass(DefragmentationContext context) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result beginDefragmentationPass(DefragmentationContext context,
                                                                               DefragmentationPassMoveInfo* passInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<DefragmentationPassMoveInfo>::type endDefragmentationPass(DefragmentationContext context) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result endDefragmentationPass(DefragmentationContext context,
                                                                             DefragmentationPassMoveInfo* passInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindBufferMemory(Allocation allocation,
                                                                                VULKAN_HPP_NAMESPACE::Buffer buffer) const;
#else
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result bindBufferMemory(Allocation allocation,
                                                                       VULKAN_HPP_NAMESPACE::Buffer buffer) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindBufferMemory2(Allocation allocation,
                                                                                 VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                 VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                                 const void* next) const;
#else
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result bindBufferMemory2(Allocation allocation,
                                                                        VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                        VULKAN_HPP_NAMESPACE::Buffer buffer,
                                                                        const void* next) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindImageMemory(Allocation allocation,
                                                                               VULKAN_HPP_NAMESPACE::Image image) const;
#else
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result bindImageMemory(Allocation allocation,
                                                                      VULKAN_HPP_NAMESPACE::Image image) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    typename VULKAN_HPP_NAMESPACE::ResultValueType<void>::type bindImageMemory2(Allocation allocation,
                                                                                VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                                VULKAN_HPP_NAMESPACE::Image image,
                                                                                const void* next) const;
#else
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result bindImageMemory2(Allocation allocation,
                                                                       VULKAN_HPP_NAMESPACE::DeviceSize allocationLocalOffset,
                                                                       VULKAN_HPP_NAMESPACE::Image image,
                                                                       const void* next) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<VULKAN_HPP_NAMESPACE::Buffer, Allocation>>::type createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                                   const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                   VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueBuffer, UniqueAllocation>>::type createBufferUnique(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                               const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                               VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#endif
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createBuffer(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* bufferCreateInfo,
                                                                   const AllocationCreateInfo* allocationCreateInfo,
                                                                   VULKAN_HPP_NAMESPACE::Buffer* buffer,
                                                                   Allocation* allocation,
                                                                   AllocationInfo* allocationInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<VULKAN_HPP_NAMESPACE::Buffer, Allocation>>::type createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                                                const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                                VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                                                                                                VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueBuffer, UniqueAllocation>>::type createBufferWithAlignmentUnique(const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo,
                                                                                                                                                                            const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                            VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                                                                                                            VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#endif
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createBufferWithAlignment(const VULKAN_HPP_NAMESPACE::BufferCreateInfo* bufferCreateInfo,
                                                                                const AllocationCreateInfo* allocationCreateInfo,
                                                                                VULKAN_HPP_NAMESPACE::DeviceSize minAlignment,
                                                                                VULKAN_HPP_NAMESPACE::Buffer* buffer,
                                                                                Allocation* allocation,
                                                                                AllocationInfo* allocationInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Buffer>::type createAliasingBuffer(Allocation allocation,
                                                                                                                                                    const VULKAN_HPP_NAMESPACE::BufferCreateInfo& bufferCreateInfo) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createAliasingBuffer(Allocation allocation,
                                                                           const VULKAN_HPP_NAMESPACE::BufferCreateInfo* bufferCreateInfo,
                                                                           VULKAN_HPP_NAMESPACE::Buffer* buffer) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void destroyBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                       Allocation allocation) const;
#else
    void destroyBuffer(VULKAN_HPP_NAMESPACE::Buffer buffer,
                       Allocation allocation) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<VULKAN_HPP_NAMESPACE::Image, Allocation>>::type createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                                                 const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                                 VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<std::pair<UniqueImage, UniqueAllocation>>::type createImageUnique(const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo,
                                                                                                                                                             const AllocationCreateInfo& allocationCreateInfo,
                                                                                                                                                             VULKAN_HPP_NAMESPACE::Optional<AllocationInfo> allocationInfo = nullptr) const;
#endif
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createImage(const VULKAN_HPP_NAMESPACE::ImageCreateInfo* imageCreateInfo,
                                                                  const AllocationCreateInfo* allocationCreateInfo,
                                                                  VULKAN_HPP_NAMESPACE::Image* image,
                                                                  Allocation* allocation,
                                                                  AllocationInfo* allocationInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<VULKAN_HPP_NAMESPACE::Image>::type createAliasingImage(Allocation allocation,
                                                                                                                                                  const VULKAN_HPP_NAMESPACE::ImageCreateInfo& imageCreateInfo) const;
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createAliasingImage(Allocation allocation,
                                                                          const VULKAN_HPP_NAMESPACE::ImageCreateInfo* imageCreateInfo,
                                                                          VULKAN_HPP_NAMESPACE::Image* image) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void destroyImage(VULKAN_HPP_NAMESPACE::Image image,
                      Allocation allocation) const;
#else
    void destroyImage(VULKAN_HPP_NAMESPACE::Image image,
                      Allocation allocation) const;
#endif

#if VMA_STATS_STRING_ENABLED
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS char* buildStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const;
#endif
    void buildStatsString(char** statsString,
                          VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void freeStatsString(char* statsString) const;
#else
    void freeStatsString(char* statsString) const;
#endif

#endif
  private:
    VmaAllocator m_allocator = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(Allocator) == sizeof(VmaAllocator),
                           "handle and wrapper have different size!");
}
#ifndef VULKAN_HPP_NO_SMART_HANDLE
namespace VULKAN_HPP_NAMESPACE {
  template<> struct UniqueHandleTraits<VMA_HPP_NAMESPACE::Allocator, VMA_HPP_NAMESPACE::Dispatcher> {
    using deleter = VMA_HPP_NAMESPACE::Deleter<VMA_HPP_NAMESPACE::Allocator, void>;
  };
}
namespace VMA_HPP_NAMESPACE { using UniqueAllocator = VULKAN_HPP_NAMESPACE::UniqueHandle<Allocator, Dispatcher>; }
#endif

namespace VMA_HPP_NAMESPACE {
  class VirtualAllocation {
  public:
    using CType      = VmaVirtualAllocation;
    using NativeType = VmaVirtualAllocation;
  public:
    VULKAN_HPP_CONSTEXPR         VirtualAllocation() = default;
    VULKAN_HPP_CONSTEXPR         VirtualAllocation(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT VirtualAllocation(VmaVirtualAllocation virtualAllocation) VULKAN_HPP_NOEXCEPT : m_virtualAllocation(virtualAllocation) {}

#if defined(VULKAN_HPP_TYPESAFE_CONVERSION)
    VirtualAllocation& operator=(VmaVirtualAllocation virtualAllocation) VULKAN_HPP_NOEXCEPT {
      m_virtualAllocation = virtualAllocation;
      return *this;
    }
#endif

    VirtualAllocation& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_virtualAllocation = {};
      return *this;
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    auto operator<=>(VirtualAllocation const &) const = default;
#else
    bool operator==(VirtualAllocation const & rhs) const VULKAN_HPP_NOEXCEPT {
      return m_virtualAllocation == rhs.m_virtualAllocation;
    }
#endif

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
  VULKAN_HPP_STATIC_ASSERT(sizeof(VirtualAllocation) == sizeof(VmaVirtualAllocation),
                           "handle and wrapper have different size!");
}
#ifndef VULKAN_HPP_NO_SMART_HANDLE
namespace VULKAN_HPP_NAMESPACE {
  template<> struct UniqueHandleTraits<VMA_HPP_NAMESPACE::VirtualAllocation, VMA_HPP_NAMESPACE::Dispatcher> {
    using deleter = VMA_HPP_NAMESPACE::Deleter<VMA_HPP_NAMESPACE::VirtualAllocation, VMA_HPP_NAMESPACE::VirtualBlock>;
  };
}
namespace VMA_HPP_NAMESPACE { using UniqueVirtualAllocation = VULKAN_HPP_NAMESPACE::UniqueHandle<VirtualAllocation, Dispatcher>; }
#endif

namespace VMA_HPP_NAMESPACE {
  class VirtualBlock {
  public:
    using CType      = VmaVirtualBlock;
    using NativeType = VmaVirtualBlock;
  public:
    VULKAN_HPP_CONSTEXPR         VirtualBlock() = default;
    VULKAN_HPP_CONSTEXPR         VirtualBlock(std::nullptr_t) VULKAN_HPP_NOEXCEPT {}
    VULKAN_HPP_TYPESAFE_EXPLICIT VirtualBlock(VmaVirtualBlock virtualBlock) VULKAN_HPP_NOEXCEPT : m_virtualBlock(virtualBlock) {}

#if defined(VULKAN_HPP_TYPESAFE_CONVERSION)
    VirtualBlock& operator=(VmaVirtualBlock virtualBlock) VULKAN_HPP_NOEXCEPT {
      m_virtualBlock = virtualBlock;
      return *this;
    }
#endif

    VirtualBlock& operator=(std::nullptr_t) VULKAN_HPP_NOEXCEPT {
      m_virtualBlock = {};
      return *this;
    }

#if defined( VULKAN_HPP_HAS_SPACESHIP_OPERATOR )
    auto operator<=>(VirtualBlock const &) const = default;
#else
    bool operator==(VirtualBlock const & rhs) const VULKAN_HPP_NOEXCEPT {
      return m_virtualBlock == rhs.m_virtualBlock;
    }
#endif

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
    void destroy() const;
#else
    void destroy() const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VULKAN_HPP_NAMESPACE::Bool32 isVirtualBlockEmpty() const;
#else
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Bool32 isVirtualBlockEmpty() const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS VirtualAllocationInfo getVirtualAllocationInfo(VirtualAllocation allocation) const;
#endif
    void getVirtualAllocationInfo(VirtualAllocation allocation,
                                  VirtualAllocationInfo* virtualAllocInfo) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualAllocation>::type virtualAllocate(const VirtualAllocationCreateInfo& createInfo,
                                                                                                                                    VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> offset = nullptr) const;
#ifndef VULKAN_HPP_NO_SMART_HANDLE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueVirtualAllocation>::type virtualAllocateUnique(const VirtualAllocationCreateInfo& createInfo,
                                                                                                                                                VULKAN_HPP_NAMESPACE::Optional<VULKAN_HPP_NAMESPACE::DeviceSize> offset = nullptr) const;
#endif
#endif
    VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result virtualAllocate(const VirtualAllocationCreateInfo* createInfo,
                                                                      VirtualAllocation* allocation,
                                                                      VULKAN_HPP_NAMESPACE::DeviceSize* offset) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void virtualFree(VirtualAllocation allocation) const;
#else
    void virtualFree(VirtualAllocation allocation) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void clearVirtualBlock() const;
#else
    void clearVirtualBlock() const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void setVirtualAllocationUserData(VirtualAllocation allocation,
                                      void* userData) const;
#else
    void setVirtualAllocationUserData(VirtualAllocation allocation,
                                      void* userData) const;
#endif

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS Statistics getVirtualBlockStatistics() const;
#endif
    void getVirtualBlockStatistics(Statistics* stats) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS DetailedStatistics calculateVirtualBlockStatistics() const;
#endif
    void calculateVirtualBlockStatistics(DetailedStatistics* stats) const;

#if VMA_STATS_STRING_ENABLED
#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS char* buildVirtualBlockStatsString(VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const;
#endif
    void buildVirtualBlockStatsString(char** statsString,
                                      VULKAN_HPP_NAMESPACE::Bool32 detailedMap) const;

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
    void freeVirtualBlockStatsString(char* statsString) const;
#else
    void freeVirtualBlockStatsString(char* statsString) const;
#endif

#endif
  private:
    VmaVirtualBlock m_virtualBlock = {};
  };
  VULKAN_HPP_STATIC_ASSERT(sizeof(VirtualBlock) == sizeof(VmaVirtualBlock),
                           "handle and wrapper have different size!");
}
#ifndef VULKAN_HPP_NO_SMART_HANDLE
namespace VULKAN_HPP_NAMESPACE {
  template<> struct UniqueHandleTraits<VMA_HPP_NAMESPACE::VirtualBlock, VMA_HPP_NAMESPACE::Dispatcher> {
    using deleter = VMA_HPP_NAMESPACE::Deleter<VMA_HPP_NAMESPACE::VirtualBlock, void>;
  };
}
namespace VMA_HPP_NAMESPACE { using UniqueVirtualBlock = VULKAN_HPP_NAMESPACE::UniqueHandle<VirtualBlock, Dispatcher>; }
#endif

namespace VMA_HPP_NAMESPACE {

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<Allocator>::type createAllocator(const AllocatorCreateInfo& createInfo);
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueAllocator>::type createAllocatorUnique(const AllocatorCreateInfo& createInfo);
#endif
#endif
  VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createAllocator(const AllocatorCreateInfo* createInfo,
                                                                    Allocator* allocator);

#ifndef VULKAN_HPP_DISABLE_ENHANCED_MODE
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<VirtualBlock>::type createVirtualBlock(const VirtualBlockCreateInfo& createInfo);
#ifndef VULKAN_HPP_NO_SMART_HANDLE
  VULKAN_HPP_NODISCARD_WHEN_NO_EXCEPTIONS typename VULKAN_HPP_NAMESPACE::ResultValueType<UniqueVirtualBlock>::type createVirtualBlockUnique(const VirtualBlockCreateInfo& createInfo);
#endif
#endif
  VULKAN_HPP_NODISCARD VULKAN_HPP_NAMESPACE::Result createVirtualBlock(const VirtualBlockCreateInfo* createInfo,
                                                                       VirtualBlock* virtualBlock);
}
#endif
