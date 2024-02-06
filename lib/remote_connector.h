#pragma once
#include "lib_export.h"
extern "C" {
#ifndef REMOTE_CONNECTOR_LIB_REMOTE_CONNECTOR_SHARED
#define REMOTE_CONNECTOR_LIB_REMOTE_CONNECTOR_SHARED
#include <stdint.h>
enum InitializationStatus {
    SUCCESS,
    FAILURE
};
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
#endif //REMOTE_CONNECTOR_LIB_REMOTE_CONNECTOR_SHARED
struct SharedMemoryEyeTrackerReaderImplementation;
LIB_EXPORT InitializationStatus initializeEyeTrackerReader(struct SharedMemoryEyeTrackerReaderImplementation*);
LIB_EXPORT void destroyEyeTrackerReader(struct SharedMemoryEyeTrackerReaderImplementation*);
LIB_EXPORT void helloRemote();
}
