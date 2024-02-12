#include "remote_connector.h"
#include <windows.h>
#include <cmath>
#include <format>
#include <iostream>
#include <sstream>
#include <algorithm>
#include <cassert>
#include "remote_connector.hpp"


#include "eye_tracker_data_struct.hpp"
#include "shared_memory_header.hpp"
TCHAR szMappedMemoryName[] =
    TEXT("Local\\Inseye-Remote-Connector-Shared-Memory");
constexpr uint32_t UNREAD_SAMPLE_INDEX = 0;
constexpr uint32_t UNWRITTEN_SAMPLE_INDEX = -1;

struct CommonData {
  HANDLE mapped_file_handle = nullptr;
  LPBYTE in_memory_buffer_pointer = nullptr;
  uint32_t lastSampleIndex = UNREAD_SAMPLE_INDEX;
  std::unique_ptr<inseye::internal::SharedMemoryHeader> shared_memory_header =
      nullptr;
};

struct SharedMemoryEyeTrackerReader : CommonData {};

struct inseye::SharedMemoryEyeTrackerReaderImplementation : CommonData {};

/**
 * \brief Frees resources held by CommonData
 * \param data reference to structure holding resources
 */
inline void DisposeEyeTrackerReaderInternal(CommonData& data) {
  if (data.mapped_file_handle != nullptr) {
    UnmapViewOfFile(data.in_memory_buffer_pointer);
    data.in_memory_buffer_pointer = nullptr;
  }
  if (data.mapped_file_handle != nullptr) {
    CloseHandle(data.mapped_file_handle);
    data.mapped_file_handle = nullptr;
  }
}

inline void DestroyEyeTrackerReaderInternal(CommonData* ptr) {
  if (ptr == nullptr)
    return;
  DisposeEyeTrackerReaderInternal(*ptr);
  delete ptr;
}

inline uint32_t CalculateEyeTrackerDataMemoryOffset(
    uint32_t headerSize, uint32_t single_eye_tracker_sample_size,
    uint32_t sample_index, uint32_t number_of_samples) {
  const int64_t as_long = sample_index;
  const auto wholes = static_cast<int32_t>(as_long % number_of_samples);
  return headerSize + single_eye_tracker_sample_size * wholes;
}

template <typename TImpl, typename TDataStruct>
bool TryReadNextDataSampleInternal(TImpl& commonData,  // NOLINT(*-no-recursion)
                                   TDataStruct& dataStruct,
                                   int recursionCount) {
  constexpr int maxRecursionCount = 10;
  if (recursionCount > maxRecursionCount)
    return false;
  auto currentDataSample =
      commonData.shared_memory_header->ReadSamplesWrittenCount();
  if (currentDataSample == UNWRITTEN_SAMPLE_INDEX)
    return false;  // service has not written any data to shared memory
  if (currentDataSample == commonData.lastSampleIndex)
    return false;  // no new data since last call
  const uint32_t total_samples_in_buffer =
      commonData.shared_memory_header->GetSampleCount();
  if (currentDataSample - commonData.lastSampleIndex >
      total_samples_in_buffer) {
    // fallback to most 'old' data sample if service overwriten buffer
    // at least once since last call
    commonData.lastSampleIndex = currentDataSample - total_samples_in_buffer;
  }
  commonData.lastSampleIndex++;
  const uint32_t sampleSize =
      commonData.shared_memory_header->GetDataSampleSize();
  auto offset = CalculateEyeTrackerDataMemoryOffset(
      commonData.shared_memory_header->GetHeaderSize(), sampleSize,
      commonData.lastSampleIndex,
      commonData.shared_memory_header->GetSampleCount());
  assert(offset + sampleSize <=
         commonData.shared_memory_header->GetBufferSize());
  inseye::internal::readDataSample(commonData.in_memory_buffer_pointer + offset,
                                   dataStruct);
  // check post read if data just read was not overwritten
  if (commonData.shared_memory_header->ReadSamplesWrittenCount() -
          commonData.lastSampleIndex >
      total_samples_in_buffer) {
    return TryReadNextDataSampleInternal(commonData, dataStruct,
                                         recursionCount++);
  }
  return true;
}

template <typename TImpl, typename TDataStruct>
bool TryReadLatestDataSampleInternal(TImpl& implementation,
                                     TDataStruct& data_struct) {
  auto latest_written =
      implementation.shared_memory_header->ReadSamplesWrittenCount();
  implementation.lastSampleIndex =
      (std::max)(implementation.lastSampleIndex, latest_written - 1);
  return TryReadNextDataSampleInternal<TImpl, TDataStruct>(implementation,
                                                           data_struct, 0);
}

/**
 * \brief initialized eye tracker reader
 * \tparam T type of data to initialize, should inherit CommonData
 * \param pptr addres of pointer to which new instance can be assigned
 */
template <typename T>
void CreateEyeTrackerReaderInternal(T** pptr) {
  CommonData impl;
  HANDLE handle = impl.mapped_file_handle =
      OpenFileMapping(FILE_MAP_READ,  // use paging file
                      FALSE, szMappedMemoryName);
  if (impl.mapped_file_handle == nullptr) {
    DisposeEyeTrackerReaderInternal(impl);
    throw inseye::CombinedException(
        std::format("Could not open file mapping object ({}).\n",
                    GetLastError()),
        kFailedToAccessSharedResources);
  }
  // read header
  try {
    impl.shared_memory_header =
        std::unique_ptr<inseye::internal::SharedMemoryHeader>(
            inseye::internal::readHeaderInternal(handle));
  } catch (...) {
    DisposeEyeTrackerReaderInternal(impl);
    throw;
  }

  impl.in_memory_buffer_pointer = static_cast<LPBYTE>(
      MapViewOfFile(impl.mapped_file_handle,  // handle to map object
                    FILE_MAP_READ,            // read/write permission
                    0, 0, impl.shared_memory_header->GetBufferSize()));
  if (impl.in_memory_buffer_pointer == nullptr) {
    DisposeEyeTrackerReaderInternal(impl);
    throw inseye::CombinedException(
        std::format("Could not map view of file ({}).", GetLastError()),
        kFailedToMapSharedResources);
  }

  *pptr = new T{impl.mapped_file_handle, impl.in_memory_buffer_pointer,
                impl.lastSampleIndex, std::move(impl.shared_memory_header)};
}

inseye::SharedMemoryEyeTrackerReader::SharedMemoryEyeTrackerReader() {
  SharedMemoryEyeTrackerReaderImplementation* ptr = nullptr;
  CreateEyeTrackerReaderInternal<>(&ptr);
  auto lamb = [](SharedMemoryEyeTrackerReaderImplementation* p) {
    DestroyEyeTrackerReaderInternal(p);
  };
  implementatation_pointer_ =
      std::unique_ptr<SharedMemoryEyeTrackerReaderImplementation,
                      decltype(lamb)>{ptr, lamb};
}

inseye::SharedMemoryEyeTrackerReader::SharedMemoryEyeTrackerReader(
    SharedMemoryEyeTrackerReader&& other) noexcept
    : implementatation_pointer_(std::move(other.implementatation_pointer_)) {}

bool inseye::SharedMemoryEyeTrackerReader::TryReadLatestEyeTrackerData(
    EyeTrackerDataStruct& eye_tracker_data_struct) noexcept {
  return TryReadLatestDataSampleInternal(*implementatation_pointer_,
                                         eye_tracker_data_struct);
}

bool inseye::SharedMemoryEyeTrackerReader::TryReadNextEyeTrackerData(
    inseye::EyeTrackerDataStruct& eye_tracker_data_struct) noexcept {
  return TryReadNextDataSampleInternal(*implementatation_pointer_,
                                       eye_tracker_data_struct, 0);
}

InitializationStatus CreateEyeTrackerReader(
    SharedMemoryEyeTrackerReader** pptr) {
  try {
    CreateEyeTrackerReaderInternal<SharedMemoryEyeTrackerReader>(pptr);
    return kSuccess;
  } catch (const inseye::CombinedException& exception) {
    return (InitializationStatus)exception.GetStatusCode();
  }
}

void DestroyEyeTrackerReader(SharedMemoryEyeTrackerReader** pptr) {
  if (pptr == nullptr)
    return;
  if (*pptr == nullptr)
    return;
  CommonData* ptr = *pptr;
  DestroyEyeTrackerReaderInternal(ptr);
  *pptr = nullptr;
}

bool TryReadNextEyeTrackerData(SharedMemoryEyeTrackerReader* pImpl,
                               EyeTrackerDataStruct* pDataStruct) {
  if (pImpl == nullptr || pDataStruct == nullptr)
    return false;
  return TryReadNextDataSampleInternal(*pImpl, *pDataStruct, 0);
}

bool TryReadLatestEyeTrackerData(SharedMemoryEyeTrackerReader* implementation,
                                 EyeTrackerDataStruct* data_struct) {
  if (implementation == nullptr || data_struct == nullptr)
    return false;
  return TryReadLatestDataSampleInternal(*implementation, *data_struct);
}