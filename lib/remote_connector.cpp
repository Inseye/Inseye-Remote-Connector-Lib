#include <iostream>
#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <format>
#include <sstream>
#include <cstring>

#include "endianess_helpers.hpp"
#include "remote_connector.h"
#include "remote_connector.hpp"

constexpr DWORD BUF_SIZE = 28028;
TCHAR szMappedMemoryName[] = TEXT("Local\\Inseye-Remote-Connector-Shared-Memory");
constexpr int UNREAD_SAMPLE_INDEX = -1;


struct CommonData {
    HANDLE hMapFile = nullptr;
    LPBYTE pBuf = nullptr;
    int lastSampleIndex = UNREAD_SAMPLE_INDEX;
    SharedMemoryHeader shared_memory_header{};
};

struct SharedMemoryEyeTrackerReaderImplementation : CommonData {
};

struct inseye::SharedMemoryEyeTrackerReaderImplementation : CommonData {
};

template<typename T>
InitializationStatus _createEyeTrackerReader(T **pptr) {
    CommonData impl;
    impl.hMapFile = CreateFileMapping(
        INVALID_HANDLE_VALUE, // use paging file
        nullptr, // default security
        PAGE_READONLY, // read/write access
        0, // maximum object size (high-order DWORD)
        BUF_SIZE, // maximum object size (low-order DWORD)
        szMappedMemoryName);
    if (impl.hMapFile == nullptr) {
        _tprintf(TEXT("Could not create file mapping object (%lu).\n"),
                 GetLastError());
        return InitializationStatus::FAILURE;
    }
    impl.pBuf = static_cast<LPBYTE>(MapViewOfFile(impl.hMapFile, // handle to map object
                                                  FILE_MAP_READ, // read/write permission
                                                  0,
                                                  0,
                                                  BUF_SIZE));
    if (impl.pBuf == nullptr) {
        _tprintf(TEXT("Could not map view of file (%lu).\n"),
                 GetLastError());
        CloseHandle(impl.hMapFile);
        impl.hMapFile = nullptr;
        return InitializationStatus::FAILURE;
    }

    *pptr = new T;
    (*pptr)->hMapFile = impl.hMapFile;
    (*pptr)->pBuf = impl.pBuf;
    (*pptr)->lastSampleIndex = impl.lastSampleIndex;
    (*pptr)->shared_memory_header = impl.shared_memory_header;
    return InitializationStatus::SUCCESS;
}

inline void _destroyEyeTrackerReader(CommonData *&ptr) {
    if (ptr == nullptr)
        return;
    if (ptr->hMapFile != nullptr) {
        UnmapViewOfFile(ptr->pBuf);
        ptr->pBuf = nullptr;
    }
    if (ptr->hMapFile != nullptr) {
        CloseHandle(ptr->hMapFile);
        ptr->hMapFile = nullptr;
    }
    delete ptr;
    ptr = nullptr;
}

inline bool _readHeader(LPBYTE memory, SharedMemoryHeader &header) {
    return false;
}

inseye::SharedMemoryEyeTrackerReader::SharedMemoryEyeTrackerReader() {
    SharedMemoryEyeTrackerReaderImplementation *ptr = nullptr;
    auto code = _createEyeTrackerReader<>(&ptr);
    if (code != InitializationStatus::SUCCESS) {
        std::stringstream ss;
        ss << "Failed to initialize eye tracker reader, code: " << code;
        throw std::runtime_error(ss.str());
    }
    auto lamb = [&](CommonData *p) { _destroyEyeTrackerReader(p); };
    m_impl = std::unique_ptr<SharedMemoryEyeTrackerReaderImplementation, decltype(lamb)>{ptr, lamb};
}

inseye::SharedMemoryEyeTrackerReader::SharedMemoryEyeTrackerReader(SharedMemoryEyeTrackerReader &&other) noexcept : m_impl(
    std::move(other.m_impl)) {
}


bool inseye::SharedMemoryEyeTrackerReader::TryReadLatestEyeTrackerData(EyeTrackerDataStruct &eyeTrackerDataStruct) noexcept {
    return false;
}


InitializationStatus createEyeTrackerReader(SharedMemoryEyeTrackerReaderImplementation **pptr) {
    return _createEyeTrackerReader<SharedMemoryEyeTrackerReaderImplementation>(pptr);
}


void destroyEyeTrackerReader(SharedMemoryEyeTrackerReaderImplementation **pptr) {
    if (pptr == nullptr)
        return;
    CommonData *ptr = *pptr;
    _destroyEyeTrackerReader(ptr);
    *pptr = nullptr;
}

bool _tryNextDataSample(const SharedMemoryEyeTrackerReaderImplementation *pImpl, EyeTrackerDataStruct *pDataStruct) {
    return false;
}


bool tryReadNextEyeTrackerData(SharedMemoryEyeTrackerReaderImplementation *pImpl, EyeTrackerDataStruct *pDataStruct) {
    if (pImpl == nullptr || pDataStruct == nullptr)
        return false;
    return _tryNextDataSample(pImpl, pDataStruct);
}

bool tryREadLatestEyeTrackerData(const SharedMemoryEyeTrackerReaderImplementation *pImpl,
                                 EyeTrackerDataStruct *pDataStruct) {
    if (pImpl == nullptr || pDataStruct == nullptr)
        return false;
    auto lastReadSuccessful = _tryNextDataSample(pImpl, pDataStruct);
    while (lastReadSuccessful) {
        lastReadSuccessful = _tryNextDataSample(pImpl, pDataStruct);
    }
    return lastReadSuccessful;
}


void helloRemote() {
    std::cout << "Hello remote" << std::endl;
}
