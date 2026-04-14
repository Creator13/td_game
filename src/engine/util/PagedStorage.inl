#pragma once

#include "core/Assert.h"

namespace util
{
    template<typename T, size_t PageSize>
    void PagedStorage<T, PageSize>::add_page()
    {
        pages.push_back(std::make_unique<Page>());
    }

    template<typename T, size_t PageSize>
    std::pair<T*, size_t> PagedStorage<T, PageSize>::allocate_slot()
    {
        if (nextElementIndex >= PageSize)
        {
            if (currentPageIndex + 1 == pages.size())
            {
                add_page();
            }
            currentPageIndex++;
            nextElementIndex = 0;
        }

        Page& page = *pages[currentPageIndex];
        page.occupied.set(nextElementIndex);

        std::byte* base = page.storage + nextElementIndex * sizeof(T);
        T* element = std::launder(reinterpret_cast<T*>(base));

        nextElementIndex++;
        return {element, nextElementIndex - 1};
    }

    template<typename T, size_t PageSize>
    PagedStorage<T, PageSize>::PagedStorage()
    {
        add_page();
    }

    template<typename T, size_t PageSize>
    PagedStorage<T, PageSize>::PagedStorage(size_t capacity)
    {
        pages.reserve(capacity);
        for (size_t i = 0; i < capacity; ++i)
        {
            add_page();
        }
    }

    template<typename T, size_t PageSize>
    PagedStorage<T, PageSize>::~PagedStorage()
    {
        for (size_t i = 0; i < size(); ++i)
        {
            if (has_at(i))
            {
                at(i).~T();
            }
        }
    }

    template<typename T, size_t PageSize>
    T& PagedStorage<T, PageSize>::operator[](size_t index)
    {
        ENGINE_ASSERT(has_at(index), "Attempted to access deleted or out of range element in paged storage, index {}", index);

        Page& page = *pages[index / PageSize];
        const size_t offset = index % PageSize;

        return *std::launder(reinterpret_cast<T*>(page.storage + (sizeof(T) * offset)));
    }

    template<typename T, size_t PageSize>
    T& PagedStorage<T, PageSize>::at(size_t index)
    {
        return operator[](index);
    }

    template<typename T, size_t PageSize>
    void PagedStorage<T, PageSize>::preallocate_pages(size_t count)
    {
        pages.reserve(pages.size() + count);
        for (size_t i = 0; i < count; ++i)
        {
            add_page();
        }
    }

    template<typename T, size_t PageSize>
    template<typename... Args>
    std::pair<T&, size_t> PagedStorage<T, PageSize>::emplace(Args&&... args)
    {
        auto [ptr, index] = allocate_slot();
        std::construct_at(ptr, std::forward<Args>(args)...);
        return {*ptr, index};
    }

    template<typename T, size_t PageSize>
    std::pair<T&, size_t> PagedStorage<T, PageSize>::insert(const T& value)
    {
        auto [ptr, index] = allocate_slot();
        std::construct_at(ptr, value);
        return {*ptr, index};
    }

    template<typename T, size_t PageSize>
    std::pair<void*, size_t> PagedStorage<T, PageSize>::allocate_uninitialized()
    {
        return allocate_slot();
    }

    template<typename T, size_t PageSize>
    bool PagedStorage<T, PageSize>::has_at(size_t index) const
    {
        if (index >= size()) return false;

        const Page& page = *pages[index / PageSize];
        const size_t elementOffset = index % PageSize;
        return page.occupied.test(elementOffset);
    }

    template<typename T, size_t PageSize>
    void PagedStorage<T, PageSize>::delete_at(size_t index)
    {
        ENGINE_ASSERT(has_at(index), "Attempted to delete already deleted element, or element out of range. index: {}", index);

        Page& page = *pages[index / PageSize];
        const size_t elementOffset = index % PageSize;

        operator[](index).~T();
        page.occupied.reset(elementOffset);
    }

    template<typename T, size_t PageSize>
    uint32_t PagedStorage<T, PageSize>::size() const
    {
        return currentPageIndex * PageSize + nextElementIndex;
    }

    template<typename T, size_t PageSize>
    uint32_t PagedStorage<T, PageSize>::count_alive() const
    {
        uint32_t total = 0;
        for (size_t i = 0; i < pages.size(); i++)
        {
            total += pages[i]->occupied.count();
        }
        return total;
    }

    template<typename T, size_t PageSize>
    uint32_t PagedStorage<T, PageSize>::capacity() const
    {
        return pages.size() * PageSize;
    }

    template<typename T, size_t PageSize>
    uint32_t PagedStorage<T, PageSize>::page_count() const
    {
        return pages.size();
    }

    template<typename T, size_t PageSize>
    uint64_t PagedStorage<T, PageSize>::mem_size() const
    {
        return pages.size() * sizeof(T) * PageSize;
    }

    template<typename T, size_t PageSize>
    float PagedStorage<T, PageSize>::occupation() const
    {
        uint32_t occupied = 0;
        for (size_t i = 0; i < pages.size(); ++i)
        {
            occupied += pages[i]->occupied.count();
        }
        return static_cast<float>(occupied) / static_cast<float>(pages.size() * PageSize);
    }

    template<typename T, size_t PageSize>
    float PagedStorage<T, PageSize>::fragmentation() const
    {
        const uint32_t num_items = this->size();
        if (num_items == 0) return 0;

        uint32_t freed = 0;
        for (size_t i = 0; i < num_items; ++i)
        {
            size_t page_index = i / PageSize;
            size_t page_offset = i % PageSize;

            // This cannot be optimized with occupied.count() (or count_alive())
            // as this would include the never-visited slots on the last page,
            // which are explicitly not counted towards fragmentation.
            if (!pages[page_index]->occupied.test(page_offset)) freed++;
        }

        return static_cast<float>(freed) / static_cast<float>(num_items);
    }
}
