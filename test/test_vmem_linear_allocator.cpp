#include "catch.hpp"

#include "gin/vmem_linear_allocator.h"
#include "gin/utils.h"

TEST_CASE("allocate and free from VMemLinearAllocator", "[VMemLinearAllocator]")
{
    using namespace gin;

    const size_t BUFFER_SIZE = 64 * 1024;

    VMemLinearAllocator alloc(BUFFER_SIZE);

    REQUIRE(alloc.IsInitialized());
    REQUIRE(alloc.GetAllocatedSize() == 0);
    REQUIRE(alloc.GetCommittedSize() == 0);

    SECTION("test IsOwnerOf()")
    {
        REQUIRE(!alloc.IsOwnerOf(nullptr));

        uint8_t* ptr0 = static_cast<uint8_t*>(alloc.Allocate(2, 1));
        if (ptr0) memset(ptr0, 0xcd, 2);

        REQUIRE(alloc.IsOwnerOf(ptr0));
        REQUIRE(alloc.IsOwnerOf(ptr0 + 1));
        REQUIRE(!alloc.IsOwnerOf(ptr0 + 2));
    }

    SECTION("test allocation")
    {
        void* ptr0 = alloc.Allocate(2, 1);
        if (ptr0) memset(ptr0, 0xcd, 2);

        REQUIRE(alloc.IsOwnerOf(ptr0));
        REQUIRE(alloc.GetAllocatedSize() == 2);
        REQUIRE(alloc.GetCommittedSize() == 4 * 1024);

        void* ptr1 = alloc.Allocate(BUFFER_SIZE - 2, 1);
        if (ptr1) memset(ptr1, 0xcd, BUFFER_SIZE - 2);

        REQUIRE(alloc.IsOwnerOf(ptr1));
        REQUIRE(alloc.GetAllocatedSize() == BUFFER_SIZE);
        REQUIRE(alloc.GetCommittedSize() == BUFFER_SIZE);
        REQUIRE(ptr0 != ptr1);

        void* ptr2 = alloc.Allocate(1, 1);
        if (ptr2) memset(ptr2, 0xcd, 1);

        REQUIRE(ptr2 == nullptr);
        REQUIRE(alloc.GetAllocatedSize() == BUFFER_SIZE);
        REQUIRE(alloc.GetCommittedSize() == BUFFER_SIZE);
    }

    SECTION("test alignment")
    {
        void* ptr0 = alloc.Allocate(2, 8);
        if (ptr0) memset(ptr0, 0xcd, 2);

        REQUIRE(alloc.IsOwnerOf(ptr0));
        REQUIRE(IsAlignedTo(ptr0, 8));

        void* ptr1 = alloc.Allocate(2, 16);
        if (ptr1) memset(ptr1, 0xcd, 2);

        REQUIRE(alloc.IsOwnerOf(ptr1));
        REQUIRE(IsAlignedTo(ptr1, 16));
        REQUIRE(ptr0 != ptr1);
    }

    SECTION("test nop free")
    {
        void* ptr0 = alloc.Allocate(2, 1);
        if (ptr0) memset(ptr0, 0xcd, 2);

        REQUIRE(alloc.GetAllocatedSize() == 2);
        REQUIRE(alloc.GetCommittedSize() == 4 * 1024);

        alloc.Deallocate(ptr0, 2);

        REQUIRE(alloc.GetAllocatedSize() == 2);
        REQUIRE(alloc.GetCommittedSize() == 4 * 1024);

        void* ptr1 = alloc.Allocate(2, 1);
        if (ptr1) memset(ptr1, 0xcd, 2);

        REQUIRE(ptr0 != ptr1);
        REQUIRE(alloc.GetAllocatedSize() == 4);
        REQUIRE(alloc.GetCommittedSize() == 4 * 1024);
    }

    SECTION("test reset")
    {
        void* ptr0 = alloc.Allocate(2, 1);
        if (ptr0) memset(ptr0, 0xcd, 2);

        REQUIRE(alloc.GetAllocatedSize() == 2);
        REQUIRE(alloc.GetCommittedSize() == 4 * 1024);

        alloc.Reset();

        REQUIRE(alloc.GetAllocatedSize() == 0);
        REQUIRE(alloc.GetCommittedSize() == 0);

        void* ptr1 = alloc.Allocate(2, 1);
        if (ptr1) memset(ptr1, 0xcd, 2);

        REQUIRE(alloc.GetAllocatedSize() == 2);
        REQUIRE(alloc.GetCommittedSize() == 4 * 1024);
        REQUIRE(ptr0 == ptr1);
    }
}

TEST_CASE("test invalid arguments to VMemLinearAllocator", "[VMemLinearAllocator]")
{
    using namespace gin;

    SECTION("test initialization")
    {
        VMemLinearAllocator alloc;

        REQUIRE(!alloc.IsInitialized());

        alloc.Initialize(1024);

        REQUIRE(!alloc.IsInitialized());
    }

    SECTION("test out of memory")
    {
        const size_t BUFFER_SIZE = 64 * 1024;

        VMemLinearAllocator alloc(BUFFER_SIZE);

        REQUIRE(alloc.IsInitialized());
        REQUIRE(alloc.GetAllocatedSize() == 0);
        REQUIRE(alloc.GetCommittedSize() == 0);

        void* ptr0 = alloc.Allocate(BUFFER_SIZE + 1, 1);
        if (ptr0) memset(ptr0, 0xcd, 2);

        REQUIRE(ptr0 == nullptr);
        REQUIRE(alloc.GetAllocatedSize() == 0);
        REQUIRE(alloc.GetCommittedSize() == 0);
    }

    SECTION("test alignment overflow")
    {
    }

    SECTION("test size overflow")
    {
    }
}

