#ifndef REMOTE_CONNECTOR_H_
#define REMOTE_CONNECTOR_H_

#if !defined(LIB_EXPORT)
#define LIB_EXPORT /* NOTHING */
#if defined(WIN32) || defined(WIN64)
#undef LIB_EXPORT
#if defined(Remote_Connector_Dlib_EXPORTS)
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT __declspec(dllimport)
#endif // defined(Remote_Connector_Dlib_EXPORTS)
#endif // defined(WIN32) || defined(WIN64)

#if defined(__GNUC__) || defined(__APPLE__) || defined(LINUX)
#if defined(Remote_Connector_Dlib_EXPORTS)
#undef LIB_EXPORT
#define LIB_EXPORT __attribute__((visibility("default")))
#endif // defined(DLib_EXPORTS)
#endif // defined(__GNUC__) || defined(__APPLE__) || defined(LINUX)

#endif // !defined(LIB_EXPORT)
#include <stdint.h>
#include <stdbool.h>

extern "C" {

    
enum InitializationStatus {
    SUCCESS,
    INVALID_SHARED_MEMORY_HEADER,
    FAILURE
};

struct SharedMemoryEyeTrackerReaderImplementation;
#pragma pack(push, 1)
typedef struct SharedMemoryHeader {
    uint32_t uHeaderSize;
    uint32_t uSamplesWriten;
    uint32_t uVersionMajor;
    uint32_t uVersionMinor;
    uint32_t uVersionPatch;
    uint32_t uTotalSize;
    uint32_t uGazeDataSamplesCount;
} SharedMemoryHeader;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct EyeTrackerDataStruct {
    uint64_t ulTime;
    float fLeftEyeX;
    float fLeftEyeY;
    float fRightEyeX;
    float fRightEyeY;
    uint32_t uGazeEvent;
} EyeTrackerDataStruct;
#pragma pack(pop)
    
struct SharedMemoryEyeTrackerReaderImplementation;
LIB_EXPORT InitializationStatus createEyeTrackerReader(SharedMemoryEyeTrackerReaderImplementation **);

LIB_EXPORT void destroyEyeTrackerReader(SharedMemoryEyeTrackerReaderImplementation **);

LIB_EXPORT bool tryReadNextEyeTrackerData(SharedMemoryEyeTrackerReaderImplementation *, EyeTrackerDataStruct *);

LIB_EXPORT bool tryReadLatestEyeTrackerData(SharedMemoryEyeTrackerReaderImplementation *, EyeTrackerDataStruct *);
    
LIB_EXPORT void helloRemote();
}
#endif // REMOTE_CONNECTOR_H_

