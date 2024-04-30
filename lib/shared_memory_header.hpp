// Last edit: 30.04.24 11:03 by Mateusz Chojnowski mateusz.chojnowski@inseye.com
//
// Copyright (c) Inseye Inc. 2024.
//
// This file is part of Inseye Software Development Kit subject to Inseye SDK License
// See  https://github.com/Inseye/Licenses/blob/master/SDKLicense.txt.
// All other rights reserved.

#ifndef SHARED_MEMORY_HEADER_HPP
#define SHARED_MEMORY_HEADER_HPP
#include "windows.h"
#include "remote_connector.h"

namespace inseye::internal {
    class  SharedMemoryHeader {
    public:
        virtual ~SharedMemoryHeader() = default;
        [[nodiscard]] virtual const Version & GetVersion() const = 0;
        [[nodiscard]] virtual uint32_t ReadSamplesWrittenCount() const = 0;
        [[nodiscard]] virtual const uint32_t & GetHeaderSize() const = 0;
        [[nodiscard]] virtual const uint32_t & GetDataSampleSize() const = 0;
        [[nodiscard]] virtual const uint32_t & GetSampleCount() const = 0;
        [[nodiscard]] virtual const uint32_t & GetBufferSize() const = 0;
    };

    SharedMemoryHeader * ReadHeaderInternal(HANDLE share_file_handle);
}


#endif //SHARED_MEMORY_HEADER_HPP
