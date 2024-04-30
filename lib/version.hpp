// Last edit: 30.04.24 11:03 by Mateusz Chojnowski mateusz.chojnowski@inseye.com
//
// Copyright (c) Inseye Inc. 2024.
//
// This file is part of Inseye Software Development Kit subject to Inseye SDK License
// See  https://github.com/Inseye/Licenses/blob/master/SDKLicense.txt.
// All other rights reserved.

#ifndef VERSION_HPP
#define VERSION_HPP
#include <cstdint>
#include "endianess_helpers.hpp"

namespace inseye::internal {

#pragma pack(push, 1)
    struct PackedVersion {
        uint32_t major;
        uint32_t minor;
        uint32_t patch;
    };
#pragma pack(pop)
}
#endif //VERSION_HPP
