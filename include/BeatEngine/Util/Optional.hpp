#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
template<typename T>
class Optional {
private:
    std::shared_ptr<T> m_Data;
public:
    constexpr Optional() noexcept : m_Data(nullptr) {}
    constexpr Optional(Optional<T>& other) noexcept : m_Data(other.m_Data) {}
    constexpr Optional(Optional<T>&& other) noexcept : m_Data(std::move(other.m_Data)) {}
    constexpr Optional(std::optional<T>& other) noexcept : m_Data(&other.value()) {}
    constexpr Optional(std::nullopt_t) noexcept : m_Data(nullptr) {}
    constexpr Optional(std::nullptr_t) noexcept : m_Data(nullptr) {}

    template<class... Args>
    constexpr Optional(Args&&... args) : m_Data(std::shared_ptr<T>(std::forward<Args>(args)...)) {}
        
    constexpr Optional(T&& data) noexcept : m_Data(std::make_shared<T>(std::move(data))) {}

    template<typename U>
    constexpr Optional(U&& data) noexcept : m_Data(std::make_shared<U>(std::move(data))) {}

    template<typename U>
    constexpr Optional(U& data) noexcept : m_Data(std::make_shared<U>(std::forward<U>(data))) {}

    constexpr bool HasValue() { return m_Data.get() != nullptr; }
    constexpr T& Value() { return *m_Data; }
    constexpr T* Ptr() { return m_Data.get(); }

    template<typename TCast>
    constexpr TCast& StaticCastTo() { return *std::static_pointer_cast<TCast>(m_Data); }

    template<typename TCast>
    constexpr TCast& DynamicCastTo() { return *std::dynamic_pointer_cast<TCast>(m_Data); }
public:
    T* operator->() noexcept { assert(m_Data.get() != nullptr && "nullptr"); return m_Data.get(); };
    T& operator*() noexcept { assert(m_Data.get() != nullptr && "nullptr"); return Value(); }

    operator std::optional<T>() noexcept { if (!HasValue()) return std::nullopt; return *m_Data; }
    operator bool() noexcept { return HasValue(); }
};
