#pragma once
#include <cstddef>

template<typename T, std::size_t Size>
struct CountedArray {
    using Ptr = T*;
    using ConstPtr = const T*;
    using Reference = T&;
    using ConstReference = const T&;
    using Iterator = T*;
    using ConstIterator = const T*;

    T Array[Size];
    T DefaultValue{};

    std::size_t CurrentPos{};

    constexpr Iterator begin() noexcept;
    constexpr ConstIterator begin() const noexcept;
    constexpr Iterator end() noexcept;
    constexpr ConstIterator end() const noexcept;

    constexpr std::size_t ReservedSize() const noexcept;
    constexpr std::size_t UsedSize() const noexcept;
    constexpr bool Empty() const noexcept;
    constexpr bool Full() const noexcept;

    constexpr Reference At(std::size_t pos) noexcept;
    constexpr Ptr Data() noexcept;
    constexpr ConstPtr Data() const noexcept;

    constexpr void SetAll(T value) noexcept;
    constexpr void Add(T value) noexcept;

    Reference operator[](std::size_t pos) noexcept;
};

#include "BeatEngine/Util/CountedArray.inl"
