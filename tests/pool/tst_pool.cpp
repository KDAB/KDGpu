#include <toy_renderer/pool.h>

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include <doctest.h>

struct int_tag;
using IntPool = Pool<int, int_tag>;

static_assert(std::is_nothrow_destructible<IntPool>{});
static_assert(std::is_nothrow_default_constructible<IntPool>{});
static_assert(!std::is_copy_constructible<IntPool>{});
static_assert(!std::is_copy_assignable<IntPool>{});
static_assert(std::is_nothrow_move_constructible<IntPool>{});
static_assert(std::is_nothrow_move_assignable<IntPool>{});

TEST_CASE("Should be truthy")
{
    SUBCASE("my first test")
    {
        REQUIRE(true == true);
    }
}

TEST_CASE("Construction")
{
    SUBCASE("A default constructed Array is empty")
    {
        IntPool array;
        REQUIRE(array.capacity() == 0);
        REQUIRE(array.size() == 0);
    }

    SUBCASE("A constructed Array with a size is empty but has capacity")
    {
        IntPool array(10);
        REQUIRE(array.capacity() == 10);
        REQUIRE(array.size() == 0);
    }

    SUBCASE("A move constructed array maintains the elements and resets the original")
    {
        IntPool array;
        auto index = array.insert(1);
        auto index2 = array.insert(2);
        auto index3 = array.insert(3);

        auto secondArray = std::move(array);
        REQUIRE(array.capacity() == 0);
        REQUIRE(array.size() == 0);
        REQUIRE(array.get(index) == nullptr);
        REQUIRE(array.get(index2) == nullptr);
        REQUIRE(array.get(index3) == nullptr);

        REQUIRE(secondArray.size() == 3);
        REQUIRE(*secondArray.get(index) == 1);
        REQUIRE(*secondArray.get(index2) == 2);
        REQUIRE(*secondArray.get(index3) == 3);
    }
}

TEST_CASE("Insertion and removal")
{
    SUBCASE("Values can be inserted and retrieved")
    {
        IntPool array;
        auto index = array.insert(5);
        REQUIRE(index.index() == 0);
        REQUIRE(index.generation() == 1);
        REQUIRE(index.isValid() == true);

        auto index2 = array.insert(7);
        REQUIRE(index2.index() == 1);
        REQUIRE(index2.generation() == 1);
        REQUIRE(index2.isValid() == true);

        REQUIRE(array.capacity() == 2);
        REQUIRE(array.size() == 2);
        REQUIRE(*array.get(index) == 5);
        REQUIRE(*array.get(index2) == 7);
    }

    SUBCASE("Deletion removes the value")
    {
        IntPool array;

        auto index = array.insert(5);
        REQUIRE(array.capacity() == 1);
        REQUIRE(array.size() == 1);

        array.remove(index);
        REQUIRE(array.get(index) == nullptr);
        REQUIRE_MESSAGE(array.capacity() == 1, "capacity does not get smaller during deletion");
        REQUIRE_MESSAGE(array.size() == 0, "size does get smaller during deletion");
    }

    SUBCASE("Deletion only invalidates the deleted index")
    {
        IntPool array;

        auto handle = array.insert(5);
        auto handle2 = array.insert(7);
        auto value2Ptr = array.get(handle2);

        array.remove(handle);
        REQUIRE(array.get(handle) == nullptr);
        REQUIRE(array.get(handle2) == value2Ptr);
        REQUIRE(*array.get(handle2) == 7);
    }

    SUBCASE("Clear invalidates all indices, but leaves capacity unchanged")
    {
        IntPool array;

        auto handle = array.insert(5);
        auto handle2 = array.insert(7);
        auto handle3 = array.insert(9);
        auto capacity = array.capacity();

        array.clear();
        REQUIRE(array.capacity() == capacity);
        REQUIRE(array.get(handle) == nullptr);
        REQUIRE(array.get(handle2) == nullptr);
        REQUIRE(array.get(handle3) == nullptr);
    }
}
