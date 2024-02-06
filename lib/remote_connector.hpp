//
// Created by Mateusz on 06.02.2024.
//
#pragma once

#include <memory>
#include "lib_export.h"
// extern "C++" {
namespace inseye {
#ifndef REMOTE_CONNECTOR_LIB_REMOTE_CONNECTOR_SHARED
#define REMOTE_CONNECTOR_LIB_REMOTE_CONNECTOR_SHARED

#include <cstdint>

    typedef struct SharedMemoryHeader {
        uint32_t uHeaderSize;
        uint32_t uSamplesWriten;
        uint32_t uVersionMajor;
        uint32_t uVersionMinor;
        uint32_t uVersionPatch;
        uint32_t uTotalSize;
        uint32_t uGazeDataSamplesCount;
    } SharedMemoryHeader;


    typedef struct EyeTrackerDataStruct {
        uint64_t ulTime;
        float fLeftEyeX;
        float fLeftEyeY;
        float fRightEyeX;
        float fRightEyeY;
        uint32_t uGazeEvent;
    } EyeTrackerDataStruct;

    struct SharedMemoryEyeTrackerReaderImplementation;
#endif //REMOTE_CONNECTOR_LIB_REMOTE_CONNECTOR_SHARED

    class LIB_EXPORT SharedMemoryEyeTrackerReader final {
    private:
        std::unique_ptr<SharedMemoryEyeTrackerReaderImplementation> impl;
    public:
        SharedMemoryEyeTrackerReader();

        ~SharedMemoryEyeTrackerReader();

        bool TryReadEyeTrackerData(EyeTrackerDataStruct &eyeTrackerDataStruct);
    };
}
// }