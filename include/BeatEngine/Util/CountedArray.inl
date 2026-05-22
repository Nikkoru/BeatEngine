#include "BeatEngine/Util/CountedArray.h"
#include "BeatEngine/Util/Exception.h"
#include <cstddef>

template<typename T, std::size_t Size>
constexpr CountedArray<T, Size>::Iterator CountedArray<T, Size>::begin() noexcept {
    return Iterator(Data());
}


template<typename T, std::size_t Size>
constexpr CountedArray<T, Size>::ConstIterator CountedArray<T, Size>::begin() const noexcept {
    return Iterator(Data());
}

template<typename T, std::size_t Size>
constexpr CountedArray<T, Size>::Iterator CountedArray<T, Size>::end() noexcept {
    return Iterator(Data() + Size);
}

template<typename T, std::size_t Size>
constexpr CountedArray<T, Size>::ConstIterator CountedArray<T, Size>::end() const noexcept {
    return Iterator(Data() + Size);
}


template<typename T, std::size_t Size>
constexpr std::size_t CountedArray<T, Size>::ReservedSize() const noexcept {
    return Size;
}

template<typename T, std::size_t Size>
constexpr std::size_t CountedArray<T, Size>::UsedSize() const noexcept {
    return CurrentPos;
}

template<typename T, std::size_t Size>
constexpr bool CountedArray<T, Size>::Empty() const noexcept {
    return CurrentPos == 0 || Size == 0;
}

template<typename T, std::size_t Size>
constexpr bool CountedArray<T, Size>::Full() const noexcept {
    return CurrentPos == Size;
}


template<typename T, std::size_t Size>
constexpr CountedArray<T, Size>::Reference CountedArray<T, Size>::At(std::size_t pos) noexcept {
    if (pos >= Size)
        THROW_RUNTIME_ERROR("pos is more than Size");
    return Array[pos];
}

template<typename T, std::size_t Size>
constexpr CountedArray<T, Size>::Ptr CountedArray<T, Size>::Data() noexcept{
    return Array;
}

template<typename T, std::size_t Size>
constexpr CountedArray<T, Size>::ConstPtr CountedArray<T, Size>::Data() const noexcept {
    return Array;
}


template<typename T, std::size_t Size>
constexpr void CountedArray<T, Size>::SetAll(T value) noexcept {
    DefaultValue = value;

    for (std::size_t i = 0; i < Size; i++) {
        Array[i] = value;
    }
}

template<typename T, std::size_t Size>
constexpr void CountedArray<T, Size>::Add(T value) noexcept {
    while (Array[CurrentPos] != DefaultValue)
        CurrentPos++;

    Array[CurrentPos] = value;

    CurrentPos++;
}


template<typename T, std::size_t Size>
CountedArray<T, Size>::Reference CountedArray<T, Size>::operator[](std::size_t pos) noexcept {
    return At(pos);
}
