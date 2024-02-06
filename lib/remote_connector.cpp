#include <iostream>
#include <windows.h>
#include <iostream>
#include <tchar.h>
#include <format>
#include <sstream>

#include "remote_connector.h"
#include "remote_connector.hpp"

const DWORD BUF_SIZE = 28028;
TCHAR szMappedMemoryName[] = TEXT("Global\\\\Inseye-Remote-Connector-Shared-Memory");

struct SharedMemoryEyeTrackerReaderImplementation {
    HANDLE hMapFile;
    LPCSTR pBuf;
};

inseye::SharedMemoryEyeTrackerReader::SharedMemoryEyeTrackerReader() : impl(
        new ::SharedMemoryEyeTrackerReaderImplementation{
                .hMapFile = nullptr
        }) {
    auto code = initializeEyeTrackerReader(impl.get());
    if (code != InitializationStatus::SUCCESS) {
        std::stringstream ss;
        ss << "Failed to initialize eye tracker reader, code: " << code;
        throw std::runtime_error(ss.str());
    }
}

inseye::SharedMemoryEyeTrackerReader::~SharedMemoryEyeTrackerReader() {
    destroyEyeTrackerReader(impl.get());
}

bool inseye::SharedMemoryEyeTrackerReader::TryReadEyeTrackerData(EyeTrackerDataStruct &eyeTrackerDataStruct) {

    return false;
}

InitializationStatus initializeEyeTrackerReader(SharedMemoryEyeTrackerReaderImplementation *ptr) {
    ptr->hMapFile = CreateFileMapping(
            INVALID_HANDLE_VALUE,    // use paging file
            nullptr,                    // default security
            PAGE_READONLY,          // read/write access
            0,                       // maximum object size (high-order DWORD)
            BUF_SIZE,                // maximum object size (low-order DWORD)
            szMappedMemoryName);
    if (ptr->hMapFile == nullptr) {
        _tprintf(TEXT("Could not create file mapping object (%lu).\n"),
                 GetLastError());
        return InitializationStatus::FAILURE;
    }
    ptr->pBuf = (LPTSTR) MapViewOfFile(ptr->hMapFile,   // handle to map object
                                       FILE_MAP_READ, // read/write permission
                                       0,
                                       0,
                                       BUF_SIZE);
    if (ptr->pBuf == nullptr)
    {
        _tprintf(TEXT("Could not map view of file (%lu).\n"),
                 GetLastError());
        CloseHandle(ptr->hMapFile);
        ptr->hMapFile = nullptr;
        return InitializationStatus::FAILURE;
    }
    return InitializationStatus::SUCCESS;
}

void destroyEyeTrackerReader(SharedMemoryEyeTrackerReaderImplementation *ptr) {
    if (ptr->hMapFile != nullptr) {
        UnmapViewOfFile(ptr->pBuf);
        ptr->pBuf = nullptr;
    }
    if (ptr->hMapFile != nullptr) {
        CloseHandle(ptr->hMapFile);
        ptr->hMapFile = nullptr;
    }
}

void helloRemote() {
    std::cout << "Hello remote" << std::endl;
}