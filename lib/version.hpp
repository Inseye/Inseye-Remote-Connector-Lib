//
// Created by Mateusz on 08.02.2024.
//

#ifndef VERSION_HPP
#define VERSION_HPP
#include <cstdint>

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
