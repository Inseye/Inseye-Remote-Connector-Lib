//
// Created by Mateusz on 08.02.2024.
//

#ifndef SHARED_MEMORY_HEADER_HPP
#define SHARED_MEMORY_HEADER_HPP
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
