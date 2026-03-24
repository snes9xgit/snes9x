#pragma once
#include <cstring>
#include <vector>
#include <atomic>

template<typename T>
class atomic_ring_buffer
{
  public:
    std::atomic_int end;
    std::atomic_int start;

    std::vector<T> buffer;

    atomic_ring_buffer()
    {
        buffer.clear();
    }

    atomic_ring_buffer(int size)
    {
        buffer.resize(size);
    }

    inline void clear(void)
    {
        start.store(0);
        end.store(0);
    }

    inline bool pull(T *dst, size_t amount)
    {
        if (space_filled() < amount)
            return false;
        if (amount == 0)
            return true;

        auto local_start = start.load(std::memory_order_acquire);
        auto first_block_size = buffer.size() - local_start;

        memcpy(dst, &buffer[local_start], std::min(amount, first_block_size) * 2);

        if (amount > first_block_size)
            memcpy(dst + first_block_size, &buffer[0], (amount - first_block_size) * 2);

        start.store((local_start + amount) % buffer.size());

        return true;
    }

    inline bool push(T *src, size_t amount)
    {
        if (space_empty() < amount)
            return false;
        if (amount == 0)
            return true;

        auto local_end = end.load(std::memory_order::acquire);
        int first_block_size = std::min(amount, buffer.size() - local_end);

        memcpy(&buffer[local_end], src, first_block_size * 2);

        if (amount > first_block_size)
            memcpy(&buffer[0], src + first_block_size, (amount - first_block_size) * 2);

        end.store((local_end + amount) % buffer.size(), std::memory_order_release);

        return true;
    }

    inline int space_empty(void) const
    {
        return buffer.size() - 2 - space_filled();
    }

    inline int space_filled(void) const
    {
        int local_start = start.load(std::memory_order_acquire);
        int local_end = end.load(std::memory_order_acquire);

        int size = local_end - local_start;
        if (size < 0)
            size += buffer.size();

        return size;
    }

    inline int avail(void)
    {
        return space_filled();
    }

    inline size_t size()
    {
        return buffer.size();
    }

    void resize(int new_size)
    {
        if (new_size & 1)
            new_size++;
        buffer.resize(new_size);
  }
};