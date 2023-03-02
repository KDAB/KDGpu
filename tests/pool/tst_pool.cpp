#include <toy_renderer/pool.h>

#include <set>

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

    SUBCASE("Inserting after a removal reuses the empty index")
    {
        IntPool array;

        auto handle = array.insert(5);
        auto handle2 = array.insert(7);
        auto handle3 = array.insert(9);

        array.remove(handle2);
        auto replacementHandle2 = array.insert(123);

        REQUIRE(handle2.index() == replacementHandle2.index());
        REQUIRE(handle2.generation() < replacementHandle2.generation());
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

    SUBCASE("After clearing, spots in the array are reused")
    {
        IntPool array;
        std::set<uint32_t> valueIndices;

        valueIndices.emplace(array.insert(5).index());
        valueIndices.emplace(array.insert(7).index());

        array.clear();

        std::set<uint32_t> newValueIndices;
        newValueIndices.emplace(array.insert(8).index());
        newValueIndices.emplace(array.insert(9).index());

        REQUIRE(array.capacity() == 2);
        REQUIRE(valueIndices == newValueIndices);
    }

    SUBCASE("After clearing, the generations are different")
    {
        IntPool array;
        std::set<uint32_t> generations;

        generations.emplace(array.insert(5).generation());
        generations.emplace(array.insert(7).generation());

        array.clear();

        std::set<uint32_t> newGenerations;
        newGenerations.emplace(array.insert(8).generation());
        newGenerations.emplace(array.insert(9).generation());

        REQUIRE(array.capacity() == 2);
        for (const auto &generation : generations) {
            REQUIRE(newGenerations.find(generation) == newGenerations.end());
        }
    }
}

TEST_CASE("handleForIndex")
{
    SUBCASE("An empty pool never returns a valid handle")
    {
        IntPool array;

        for (uint32_t i = 0; i < 10; ++i) {
            REQUIRE_FALSE(array.handleForIndex(i).isValid());
        }
    }

    SUBCASE("A full pool returns a valid handle for every index, but not more")
    {
        IntPool array;

        for (auto i = 0; i < 10; ++i) {
            array.emplace(std::move(i));
        }

        for (uint32_t i = 0; i < array.size(); ++i) {
            REQUIRE(array.handleForIndex(i).isValid());
            REQUIRE_FALSE(array.handleForIndex(i + array.size()).isValid());
        }
    }
}

class MyType
{
public:
    explicit MyType(uint32_t a, uint32_t b) noexcept
        : m_a(a)
        , m_b(b)
    {
    }

    ~MyType()
    {
        m_a = 0;
        m_b = 0;
        ms_destructorCalled = true;
    }

    uint32_t a() const noexcept { return m_a; }
    uint32_t b() const noexcept { return m_b; }

    static bool ms_destructorCalled;

private:
    uint32_t m_a;
    uint32_t m_b;
};

bool MyType::ms_destructorCalled = false;

struct MyType_tag;
using MyTypePool = Pool<MyType, MyType_tag>;

TEST_CASE("Non-trivial types")
{
    SUBCASE("Non-default constructible types can be used")
    {
        std::unique_ptr<MyTypePool> array = std::make_unique<MyTypePool>();

        auto handle = array->emplace(123, 69);
        REQUIRE(array->get(handle)->a() == 123);
        REQUIRE(array->get(handle)->b() == 69);

        array->remove(handle);
        REQUIRE(MyType::ms_destructorCalled == false);

        // Dtor only called when pool goes out of scope
        array = {};
        REQUIRE(MyType::ms_destructorCalled == true);
        MyType::ms_destructorCalled = false;
    }
}
