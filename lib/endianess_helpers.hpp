//
// Created by Mateusz on 07.02.2024.
//
#ifndef ENDIANESS_HELPERS_HPP_
#define ENDIANESS_HELPERS_HPP_
#include <bit>
#include <array>
#include <functional>
#include <algorithm>

constexpr std::endian LIB_ENDIAN = std::endian::little;

#pragma optimize("", off)
// make sure that this function is not cut off during compilation 
inline std::endian GetEndian() {
    int i = 1;
    return (int) *(unsigned char *) &i == 1 ? std::endian::little : std::endian::big;
}
#pragma optimize("", on)

template<typename T>
T byteswap(const T& ref) {
  using arr_type = std::array<std::byte, sizeof(T)>;
  static_assert(sizeof (T) == sizeof (arr_type), "Mismatched size.");
  auto value_representation = std::bit_cast<arr_type, T>(ref);
  std::ranges::reverse(value_representation);
  return std::bit_cast<T, arr_type>(value_representation);
}

template<typename T>
std::function<T (const T*)> read_swap_endianes_if_needed = GetEndian() == LIB_ENDIAN ? [](const T* arg) -> T { return *arg; } : [](const T* arg) -> T { return byteswap<T>(*arg); }; 
#endif
