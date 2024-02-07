//
// Created by Mateusz on 06.02.2024.
//
#ifndef REMOTE_CONNECTOR_HPP_
#define REMOTE_CONNECTOR_HPP_

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

#include <cstdint>
#include <functional>
#include <memory>
extern "C++" {
namespace inseye {

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

    class LIB_EXPORT SharedMemoryEyeTrackerReader final {
        std::unique_ptr<SharedMemoryEyeTrackerReaderImplementation, std::function<void (SharedMemoryEyeTrackerReaderImplementation*)>> m_impl;
    public:
        SharedMemoryEyeTrackerReader();

        SharedMemoryEyeTrackerReader(SharedMemoryEyeTrackerReaderImplementation &) = delete;

        SharedMemoryEyeTrackerReader(SharedMemoryEyeTrackerReader &&) noexcept;

        bool TryReadLatestEyeTrackerData(EyeTrackerDataStruct &) noexcept;

        bool TryReadNextEyeTrackerData(EyeTrackerDataStruct &) noexcept;
    };
}
}
#endif // REMOTE_CONNECTOR_HPP_