// Last edit: 30.04.24 11:03 by Mateusz Chojnowski mateusz.chojnowski@inseye.com
//
// Copyright (c) Inseye Inc. 2024.
//
// This file is part of Inseye Software Development Kit subject to Inseye SDK License
// See  https://github.com/Inseye/Licenses/blob/master/SDKLicense.txt.
// All other rights reserved.

#ifndef ENDIANESS_HELPERS_HPP_
#define ENDIANESS_HELPERS_HPP_
#include <algorithm>
#include <array>
#include <bit>
#include <functional>

namespace inseye::internal {
constexpr std::endian LIB_ENDIAN = std::endian::little;
#pragma optimize("", off)
// make sure that this function is not cut off during compilation
inline std::endian GetEndian() {
  int i = 1;
  return (int)*(unsigned char*)&i == 1 ? std::endian::little : std::endian::big;
}
#pragma optimize("", on)

template <typename T>
T byteswap(const T& ref) {
  using arr_type = std::array<std::byte, sizeof(T)>;
  static_assert(sizeof(T) == sizeof(arr_type), "Mismatched size.");
  auto value_representation = std::bit_cast<arr_type, T>(ref);
  std::ranges::reverse(value_representation);
  return std::bit_cast<T, arr_type>(value_representation);
}

template <typename T>
T read_swap_endianess_if_needed(const T* arg) {
  static const std::function<T(const T*)> implementation =
      GetEndian() == LIB_ENDIAN ? [](const T* arg) -> T {
    return *arg;
  }
  : [](const T* arg) -> T {
      return byteswap<T>(*arg);
    };

  return implementation(arg);
}

template <typename T>
void write_swap_endianess_if_needed(T* destination, const T* source) {
  static const std::function<void(T*, const T*)> implementation =
      GetEndian() == LIB_ENDIAN ? [](T*, const T*) -> void {
    return;
  }
  : [](T* destination, const T* source) -> void {
      *destination = byteswap<T>(*source);
      return;
    };
  return implementation(destination, source);
}
}
#endif
