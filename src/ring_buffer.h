#pragma once

#include <type_traits>
#include <cassert>
#include <iostream> // TEMP TEMP

template <class T, size_t N>
class RingBuffer
{
    // static_assert(std::is_trivially_copyable_v<T>,
    //         "RingBuffer value type must be TriviallyCopyable");
    static_assert(std::is_nothrow_constructible_v<T>,
            "RingBuffer value type must be NoThrowConstructible");

public:
    using value_type = T;
    using pointer = T*;
    using const_pointer = const T*;
    using reference = T&;
    using const_reference = const T&;
    using size_type = int;

    struct iterator;
    struct const_iterator;

    constexpr RingBuffer() noexcept = default;

    // TODO: this semantic is confusing because the list will be in
    //       reversed order
    constexpr RingBuffer(std::initializer_list<T> vs) noexcept
    {
        for (auto v : vs) {
            emplace_front(v);
        }
    }

    constexpr iterator begin() noexcept { return iterator(&_data[_head], this); }
    constexpr const_iterator begin() const noexcept { return const_iterator{&_data[_head], this}; }
    constexpr iterator end() noexcept { return iterator{}; }
    constexpr const_iterator end() const noexcept { return const_iterator{}; }
    constexpr size_type size() const noexcept { return _size; }
    constexpr bool empty() const noexcept { return _size == 0; }
    constexpr bool full() const noexcept { return _size == N; }
    constexpr size_type capacity() const noexcept { return N; }

    template <class... Args>
    constexpr reference emplace_front(Args&&... args) noexcept
    {
        _head = _head == 0 ? N - 1 : _head - 1;
        new (&_data[_head]) T{args...};
        _size = std::min(_size + 1, capacity());
        return _data[_head];
    }

    constexpr reference push_front(const T& x) noexcept
    {
        return emplace_front(x);
    }

    constexpr void pop_front() noexcept
    {
        assert(!empty());
        _head = (_head + 1) % N;
        --_size;
    }

    constexpr reference operator[](size_type pos) noexcept
    {
        assert(0 <= pos && pos < _size);
        pos = (_head + pos) % N;
        return _data[pos];
    }

    constexpr const_reference operator[](size_type pos) const noexcept
    {
        assert(0 <= pos && pos < _size);
        pos = (_head + pos) % N;
        return _data[pos];
    }

    friend class iterator;
    friend class const_iterator;

private:
    constexpr static int _advance(int pos) noexcept
    {
        return (pos + 1) % N;
    }

    constexpr pointer _advance_pointer(pointer ptr) noexcept
    {
        int pos = _advance(ptr - &_data[0]);
        return &_data[pos];
    }

    constexpr const_pointer _advance_pointer(const_pointer ptr) const noexcept
    {
        int pos = _advance(ptr - &_data[0]);
        return &_data[pos];
    }

    std::array<T, N> _data;
    int              _head = 0;
    int              _size = 0;
};

template <class T, size_t N>
struct RingBuffer<T, N>::iterator
{
    constexpr iterator() noexcept = default;
    constexpr iterator(T* ptr, RingBuffer<T, N>* rb) noexcept
        : _ptr(ptr), _rb(rb) {}

    constexpr iterator& operator++() noexcept
    {
        _ptr = _rb->_advance_pointer(_ptr);
        if (_ptr == &_rb->_data[_rb->_head])
            _ptr = nullptr;
        return *this;
    }

    constexpr iterator operator++(int) noexcept
    {
        iterator tmp{*this};
        ++(*this);
        return tmp;
    }

    constexpr reference operator*() const noexcept
    {
        return *_ptr;
    }

    constexpr pointer operator->() const noexcept
    {
        return _ptr;
    }

    friend constexpr bool operator==(iterator a, iterator b) noexcept
    {
        return a._ptr == b._ptr;
    }

    friend constexpr bool operator!=(iterator a, iterator b) noexcept
    {
        return a._ptr != b._ptr;
    }

    friend class RingBuffer<T, N>::const_iterator;

private:
    T*                _ptr = nullptr;
    RingBuffer<T, N>* _rb = nullptr;
};

template <class T, size_t N>
struct RingBuffer<T, N>::const_iterator
{
    constexpr const_iterator() noexcept = default;
    constexpr const_iterator(const T* ptr, const RingBuffer<T, N>* rb) noexcept
        : _ptr(ptr), _rb(rb) {}
    constexpr const_iterator(iterator it) noexcept
        : _ptr(it._ptr), _rb(it._rb) {}

    constexpr const_iterator& operator++() noexcept
    {
        _ptr = _rb->_advance_pointer(_ptr);
        if (_ptr == &_rb->_data[_rb->_head])
            _ptr = nullptr;
        return *this;
    }

    constexpr const_iterator operator++(int) noexcept
    {
        const_iterator tmp{*this};
        ++(*this);
        return tmp;
    }

    constexpr const_reference operator*() const noexcept
    {
        return *_ptr;
    }

    constexpr const_pointer operator->() const noexcept
    {
        return _ptr;
    }

    friend constexpr bool operator==(const_iterator a, const_iterator b) noexcept
    {
        return a._ptr == b._ptr;
    }

    friend constexpr bool operator!=(const_iterator a, const_iterator b) noexcept
    {
        return a._ptr != b._ptr;
    }

private:
    const T*                _ptr = nullptr;
    const RingBuffer<T, N>* _rb = nullptr;
};
