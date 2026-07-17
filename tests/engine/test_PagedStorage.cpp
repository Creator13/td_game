#include <catch2/catch_all.hpp>

#include "../../build/debug/vcpkg_installed/x64-windows-static-md/include/spdlog/spdlog.h"
#include "math/math.h"
#include "util/PagedStorage.h"

namespace
{
    struct LifetimeTracker
    {
        int constructions = 0;
        int copies = 0;
        int moves = 0;
        int destructions = 0;
        int active = 0;

        void on_construct()
        {
            constructions++;
            active++;
        }

        void on_copy()
        {
            copies++;
            active++;
        }

        void on_move()
        {
            moves++;
            active++;
        }

        void on_destruct()
        {
            destructions++;
            active--;
        }

        void reset() { *this = LifetimeTracker{ }; }
    };

    struct TrackedTestType
    {
        LifetimeTracker* tracker = nullptr;
        int someValue = 0;

        TrackedTestType(LifetimeTracker* t, int v) : tracker(t), someValue(v)
        {
            if (tracker) tracker->on_construct();
        }

        TrackedTestType(const TrackedTestType& other)
            : tracker(other.tracker), someValue(other.someValue)
        {
            if (tracker) tracker->on_copy();
        }

        TrackedTestType(TrackedTestType&& other) noexcept
            : tracker(other.tracker), someValue(other.someValue)
        {
            if (tracker)
            {
                tracker->on_move();
            }
        }

        TrackedTestType& operator=(TrackedTestType&& other) noexcept
        {
            if (this == &other) return *this;
            if (tracker) tracker->on_destruct();

            tracker = other.tracker;
            someValue = other.someValue;

            if (tracker) tracker->on_move();
            return *this;
        }

        ~TrackedTestType()
        {
            if (tracker)
            {
                tracker->on_destruct();
            }
        }

        bool operator==(const TrackedTestType& other) const { return someValue == other.someValue; }
    };
}

using namespace util;
using Catch::Matchers::WithinAbs;
constexpr float EPS = math::EPSILON;

#define PAGED_STORAGE_TEST_TAG "[util][container]"

constexpr size_t SIZE = 16;

TEST_CASE("Default construction", PAGED_STORAGE_TEST_TAG)
{
    SPDLOG_INFO("sizeof(PagedStorage<TrackedTestType, 16>) == {}", sizeof(PagedStorage<TrackedTestType, 16>));
    const PagedStorage<TrackedTestType, SIZE> storage;
    CHECK(storage.page_count() == 1);
    CHECK(storage.mem_size() == SIZE * sizeof(TrackedTestType));
    CHECK(storage.size() == 0);
    CHECK(storage.capacity() == SIZE);
    CHECK(storage.count_alive() == 0);
    CHECK(storage.occupation() == 0);
}

TEST_CASE("Capacity constructor", PAGED_STORAGE_TEST_TAG)
{
    const PagedStorage<TrackedTestType, SIZE> storage(10);
    CHECK(storage.page_count() == 10);
    CHECK(storage.mem_size() == 10 * SIZE * sizeof(TrackedTestType));
    CHECK(storage.size() == 0);
    CHECK(storage.capacity() == 10 * SIZE);
    CHECK(storage.count_alive() == 0);
    CHECK(storage.occupation() == 0);
}

TEST_CASE("Insertion", PAGED_STORAGE_TEST_TAG)
{
    LifetimeTracker t; {
        PagedStorage<TrackedTestType, 16> storage;

        SECTION("Insert")
        {
            const TrackedTestType original = TrackedTestType(&t, 1);
            storage.insert(original);
            CHECK(storage.size() == 1);
            CHECK(storage.count_alive() == 1);

            TrackedTestType& val = storage.at(0);
            REQUIRE(val.someValue == original.someValue);

            CHECK(t.constructions == 1);
            CHECK(t.copies == 1);
        }

        SECTION("Emplace")
        {
            storage.emplace(&t, 5);
            CHECK(storage.size() == 1);
            CHECK(storage.count_alive() == 1);

            TrackedTestType& val = storage.at(0);
            REQUIRE(val.someValue == 5);

            CHECK(t.constructions == 1);
        }

        SECTION("Page overflow")
        {
            for (int i = 0; i < 19; i++)
            {
                storage.emplace(&t, i);
            }

            CHECK(storage.size() == 19);
            CHECK(storage.count_alive() == 19);
            CHECK(storage.page_count() == 2); // A second page was created
            CHECK(t.constructions == 19);

            for (int i = 0; i < 19; i++)
            {
                CHECK(storage.has_at(i));

                TrackedTestType& val = storage[i];
                REQUIRE(val.someValue == i);
            }
        }

        SECTION("Preallocating pages does not trigger new page creation upon insertion until storage is full")
        {
            storage.preallocate_pages(3);
            CHECK(storage.page_count() == 4);

            // Fill preallocated pages
            for (int i = 0; i < 4 * 16; i++)
            {
                storage.emplace(&t, i);
            }
            // No new pages have been created, 4x16 elements should fit on 4 pages of 16.
            CHECK(storage.page_count() == 4);

            // Add one more element to trigger overload of preallocated pages
            storage.emplace(&t, 16 * 4 + 1);

            // Should trigger the allocation of one new page
            CHECK(storage.page_count() == 5);
        }

        SECTION("Occupation increases while pages fill, decreases on deletion")
        {
            for (int i = 0; i < 16; i++)
            {
                storage.emplace(&t, i);
                CHECK_THAT(storage.occupation(), WithinAbs(static_cast<float>(i+1)/16, EPS));
            }
            storage.emplace(&t, 42);
            CHECK_THAT(storage.occupation(), WithinAbs(17.f/32.f, EPS));

            storage.delete_at(0);
            CHECK_THAT(storage.occupation(), WithinAbs(.5f, EPS));
        }
    } // storage dies

    CHECK(t.active == 0);
}

TEST_CASE("Deletion", PAGED_STORAGE_TEST_TAG)
{
    LifetimeTracker t; {
        PagedStorage<TrackedTestType, 8> storage;

        TrackedTestType* ptr1 = &storage.emplace(&t, 5);
        TrackedTestType* ptr2 = &storage.emplace(&t, 4);
        TrackedTestType* ptr3 = &storage.emplace(&t, 19);
        TrackedTestType* ptr4 = &storage.emplace(&t, 0);

        CHECK(t.constructions == 4);
        CHECK(storage.size() == 4);
        CHECK(storage.count_alive() == 4);

        SECTION("Deleting elements decreases live count, but size remains")
        {
            storage.delete_at(0);

            CHECK(storage.size() == 4); // Size does not change
            REQUIRE(storage.count_alive() == 3); // Live count decreases

            // Verify lifetimes
            CHECK(t.destructions == 1);
            CHECK(t.active == 3);

            // Add one more element; both size and live count increase by one from previous values
            storage.emplace(&t, 42);
            CHECK(storage.size() == 5);
            CHECK(storage.count_alive() == 4);
        }

        SECTION("Deleting elements fails has_at")
        {
            storage.delete_at(0);
            CHECK(storage.size() == 4);
            REQUIRE_FALSE(storage.has_at(0));

            CHECK(t.destructions == 1);
            CHECK(t.active == 3);
        }

        SECTION("Untouched elements point at their original data after misc elements are deleted")
        {
            CHECK(storage.fragmentation() == 0);

            storage.delete_at(0);
            storage.delete_at(2);

            // Original pointers are as valid as ever
            CHECK(ptr2->someValue == 4);
            CHECK(ptr4->someValue == 0);

            // Other method of validating: there are no moves or copies of elements in storage.
            CHECK(t.moves == 0);
            CHECK(t.copies == 0);

            // Note: do not test pointers to deleted elements as this is undefined behavior.

            // Test fragmentation
            CHECK(storage.fragmentation() == .5f);
        }
    }
    CHECK(t.active == 0);
}
