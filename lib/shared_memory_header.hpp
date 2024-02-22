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

    enum class HeaderReadResult {
        SUCCESS,
        BUFFER_SMALLER_THAN_HEADER_SIZE_TYPE,
        BUFFER_SMALLER_THAN_MINIMUM_HEADER_SIZE,
        SERVICE_DATA_FORMAT_VERSION_TO_HIGH,
        SERVICE_DATA_FORMAT_VERSION_TO_LOW
    };

    SharedMemoryHeader *readHeaderInternal(HANDLE);
}


#endif //SHARED_MEMORY_HEADER_HPP
