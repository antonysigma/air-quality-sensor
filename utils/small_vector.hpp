#pragma once
#include <array>
#include <cstdint>

#include "clamp.hpp"

namespace utils {

template <typename T, uint16_t max_size>
class SmallVector {
    std::array<T, max_size> buffer{};
    uint16_t _size{0};

   public:
    constexpr void emplaceBack(T&& value) {
        if (_size >= max_size) {
            return;
        }

        buffer[_size] = std::move(value);
        _size++;
    }

    [[nodiscard]] constexpr uint16_t size() const { return _size; }

    constexpr void clear() { _size = 0; }

    constexpr void resize(uint16_t s) { _size = utils::Min(s, max_size); }

    constexpr const std::array<T, max_size>& sanitizedBuffer() {
        if (_size >= max_size) {
            return buffer;
        }
        std::fill(buffer.begin() + _size, buffer.end(), 0);
        return buffer;
    }

    constexpr const T& operator[](uint16_t index) const { return buffer[index]; }

    constexpr T& operator[](uint16_t index) { return buffer[index]; }

    constexpr const T* begin() const { return buffer.begin(); }

    constexpr const T* end() const { return buffer.begin() + _size; }
};
}  // namespace utils