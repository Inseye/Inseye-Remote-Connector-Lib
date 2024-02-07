//
// Created by Mateusz on 07.02.2024.
//
#ifndef ENDIANESS_HELPERS_HPP_
#define ENDIANESS_HELPERS_HPP_
#include <bit>
#include <functional>

constexpr std::endian LIB_ENDIAN = std::endian::little;

inline std::endian GetEndian () {
    int i=1;
    return (int)*(unsigned char *)&i==1 ? std::endian::little : std::endian::big;
}

template<typename T>
std::function<void (T&)> swap_endianess_if_needed = GetEndian() == LIB_ENDIAN ? [](T& _) {} : std::byteswap;
#endif