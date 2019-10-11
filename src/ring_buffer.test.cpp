#include "catch.hpp"
#include "ring_buffer.h"

TEST_CASE("size, empty, capacity", "[ringbuffer]")
{
    RingBuffer<int, 8> rb;
    REQUIRE(rb.empty()    == true);
    REQUIRE(rb.full()     == false);
    REQUIRE(rb.size()     == 0);
    REQUIRE(rb.capacity() == 8);

    rb.push_front(1);
    REQUIRE(rb.empty()    == false);
    REQUIRE(rb.full()     == false);
    REQUIRE(rb.size()     == 1);
    REQUIRE(rb.capacity() == 8);

    rb.push_front(2);
    REQUIRE(rb.empty()    == false);
    REQUIRE(rb.full()     == false);
    REQUIRE(rb.size()     == 2);
    REQUIRE(rb.capacity() == 8);
}

TEST_CASE("push_front", "[ringbuffer]")
{
    RingBuffer<int, 8> rb;
    REQUIRE(rb.empty()  == true);
    REQUIRE(rb.size()   == 0);

    for (int i = 0; i < 8; ++i) {
        int val = i + 1;
        rb.push_front(val);
        REQUIRE(rb.size()   == i + 1);
        REQUIRE(*rb.begin() == val);

        for (int j = 0; j < i; ++j) {
            REQUIRE(rb[j] == val - j);
        }
    }

    REQUIRE(rb.full() == true);

    auto i = rb.push_front(42);
    REQUIRE(i         == 42);
    REQUIRE(rb[0]     == 42);
    REQUIRE(rb.full() == true);
    for (int i = 1; i < 8; ++i) {
        INFO("i = " << i);
        REQUIRE(rb[i] == 9 - i);
    }

    {
        std::vector<int> expect = { 42, 8, 7, 6, 5, 4, 3, 2 };
        auto actual_it = rb.begin();
        auto expect_it = expect.begin();
        REQUIRE(rb.size() == expect.size());
        for (int i = 0; i < rb.size(); ++i) {
            REQUIRE(actual_it != rb.end());
            REQUIRE(*actual_it == *expect_it);
            ++actual_it;
            ++expect_it;
        }
        REQUIRE(actual_it == rb.end());
    }

    rb.push_front(44);
    {
        std::vector<int> expect = { 44, 42, 8, 7, 6, 5, 4, 3 };
        auto actual_it = rb.begin();
        auto expect_it = expect.begin();
        REQUIRE(rb.size() == expect.size());
        for (int i = 0; i < rb.size(); ++i) {
            REQUIRE(actual_it != rb.end());
            REQUIRE(*actual_it == *expect_it);
            ++actual_it;
            ++expect_it;
        }
        REQUIRE(actual_it == rb.end());
    }
}

TEST_CASE("const ops", "[ringbuffer]")
{
    RingBuffer<int, 8> rb;
    for (int i = 0; i < 8; ++i) {
        rb.push_front(i);
    }

    const auto& crb = rb;
    RingBuffer<int, 8>::const_iterator it  = crb.begin();
    RingBuffer<int, 8>::const_iterator end = crb.end();
    for (int i = 7; i >= 0; --i) {
        REQUIRE(it != end);
        REQUIRE(*it == i);
        ++it;
    }
    REQUIRE(it == end);

    for (int i = 0; i < 8; ++i) {
        rb.push_front(10 + i);
    }

    it  = crb.begin();
    end = crb.end();
    for (int i = 7 + 10; i >= 0 + 10; --i) {
        REQUIRE(it != end);
        REQUIRE(*it == i);
        ++it;
    }
    REQUIRE(it == end);
}
