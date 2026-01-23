#pragma once

#include <bitset>
#include <cstdint>
#include <memory>
#include <vector>

namespace util
{
    /// Pointer-stable growable container
    template<typename T, size_t PageSize>
    class PagedStorage
    {
        struct Page
        {
            alignas(T) std::byte storage[sizeof(T) * PageSize];
            std::bitset<PageSize> occupied {};
        };

        std::vector<std::unique_ptr<Page>> pages;

        size_t currentPageIndex = 0;
        size_t nextElementIndex = 0;

        void add_page();

        /// Allocate a new slot, mark it as occupied, and move the next index pointer forward.
        /// Creates and allocates a new page if the last page is full.
        T* allocate_slot();

    public:
        PagedStorage();

        explicit PagedStorage(size_t capacity);
        ~PagedStorage();

        /// Obtain an element at a specific index. Equivalent to at(index)
        T& operator[](size_t index);

        /// Obtain an element at a specific index. Equivalent to operator[index]
        T& at(size_t index);

        /// Allocates `count` additional pages. This might be useful when many elements will be added at once.
        void preallocate_pages(size_t count);

        /// In-place construction of an element of type T.
        template<typename... Args>
        T& emplace(Args&&... args);

        /// Copy-insertion of an existing element.
        T& insert(const T& value);

        T* allocate_uninitialized();

        /// Checks if the element at given index exists and is currently alive.
        bool has_at(size_t index) const;

        /// Deletes element at given index. Any pointer to this element is now considered stale.
        void delete_at(size_t index);

        /// Returns the number of elements in the storage, including visited then freed slots.
        uint32_t size() const;

        /// Returns the number of live elements in the storage, excluding visited then freed slots.
        uint32_t count_alive() const;

        /// Returns the total number of allocated slots in the storage, regardless of usage.
        uint32_t capacity() const;

        /// Returns the number of allocated pages (includes empty, unused, pre-allocated pages).
        uint32_t page_count() const;

        /// Returns the byte count of the number of allocated slots (including unoccupied and freed slots).
        uint64_t mem_size() const;

        /// Returns the percentage of slots currently used over the total number of allocated slots.
        float occupation() const;

        /// Returns the percentage of visited slots that have been freed after their allocation over the total number
        /// of visited slots.
        float fragmentation() const;
    };


}

#include "PagedStorage.inl"
