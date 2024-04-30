// Last edit: 30.04.24 11:03 by Mateusz Chojnowski mateusz.chojnowski@inseye.com
//
// Copyright (c) Inseye Inc. 2024.
//
// This file is part of Inseye Software Development Kit subject to Inseye SDK License
// See  https://github.com/Inseye/Licenses/blob/master/SDKLicense.txt.
// All other rights reserved.

#include "remote_connector.h"
#include <windows.h>
#include <cassert>
#include <cmath>
#include <format>
#include <thread>

#include "errors.hpp"
#include "eye_tracker_data_struct.hpp"
#include "named_pipe_communicator.hpp"
#include "shared_memory_header.hpp"


constexpr uint32_t UNREAD_SAMPLE_INDEX = 0;
constexpr uint32_t UNWRITTEN_SAMPLE_INDEX =
    (std::numeric_limits<uint32_t>::max)();
using exc_msg = std::array<char, 1024>;
static_assert(sizeof(enum inseye::c::InseyeGazeEvent) == sizeof(uint32_t),
              "Incompatible type size");
static_assert((inseye::c::kInsGazeBlinkLeft ^ ((uint32_t)1)) == 0,
              "Incompatible binary layout");

struct inseye::c::InseyeEyeTracker {
  std::unique_ptr<void, std::function<void(void*)>> mapped_file_handle;
  std::unique_ptr<byte, std::function<void(void*)>> in_memory_buffer_pointer;
  uint32_t lastSampleIndex = UNREAD_SAMPLE_INDEX;
  inseye::internal::NamedPipeCommunicator named_pipe_communicator;
  std::unique_ptr<inseye::internal::SharedMemoryHeader> shared_memory_header =
      nullptr;
};

inline uint32_t CalculateEyeTrackerDataMemoryOffset(
    uint32_t headerSize, uint32_t single_eye_tracker_sample_size,
    uint32_t sample_index, uint32_t number_of_samples) {
  const int64_t as_long = sample_index;
  const auto wholes = static_cast<int32_t>(as_long % number_of_samples);
  return headerSize + single_eye_tracker_sample_size * wholes;
}

inline void ReadDataSampleInternal(const inseye::c::InseyeEyeTracker& commonData, inseye::c::InseyeEyeTrackerDataStruct& data_struct) {
  const uint32_t sampleSize =
      commonData.shared_memory_header->GetDataSampleSize();
  auto offset = CalculateEyeTrackerDataMemoryOffset(
      commonData.shared_memory_header->GetHeaderSize(), sampleSize,
      commonData.lastSampleIndex,
      commonData.shared_memory_header->GetSampleCount());
  assert(offset + sampleSize <=
         commonData.shared_memory_header->GetBufferSize());
  inseye::internal::readDataSample(
      commonData.in_memory_buffer_pointer.get() + offset, data_struct);
}

bool TryReadNextDataSampleInternal(
    inseye::c::InseyeEyeTracker& commonData,  // NOLINT(*-no-recursion)
    inseye::c::InseyeEyeTrackerDataStruct& dataStruct, int recursionCount) {
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
  ReadDataSampleInternal(commonData, dataStruct);
  // check post read if data just read was not overwritten
  if (commonData.shared_memory_header->ReadSamplesWrittenCount() -
          commonData.lastSampleIndex >
      total_samples_in_buffer) {
    return TryReadNextDataSampleInternal(commonData, dataStruct,
                                         ++recursionCount);
  }
  return true;
}

bool TryReadLatestDataSampleInternal(
    inseye::c::InseyeEyeTracker& implementation,
    inseye::c::InseyeEyeTrackerDataStruct& data_struct) {
  auto latest_written =
      implementation.shared_memory_header->ReadSamplesWrittenCount();
  implementation.lastSampleIndex =
      (std::max)(implementation.lastSampleIndex, latest_written - 1);
  return TryReadNextDataSampleInternal(implementation, data_struct, 0);
}

/**
 * \brief initialized eye tracker reader
 * \tparam T type of data to initialize, should inherit CommonData
 * \param pptr addres of pointer to which new instance can be assigned
 */

void CreateEyeTrackerReaderInternal(
    inseye::c::InseyeEyeTracker** pptr,
    const std::function<bool()>& is_cancellation_requested) {
  auto named_pipe_communicator =
      inseye::internal::NamedPipeCommunicator::Create(
          is_cancellation_requested);
  ThrowIfCancellationRequested(is_cancellation_requested);
  auto serviceInfo = named_pipe_communicator.GetServiceInfo();

  std::unique_ptr<void, std::function<void(void*)>> memory_mapped_file{
      OpenFileMapping(FILE_MAP_READ,  // use paging file
                      FALSE, serviceInfo.shared_buffer_path.c_str()),
      CloseHandle};
  if (memory_mapped_file == nullptr) {
    ThrowInitialization(
        std::format("Could not open file mapping object, GLE={}.\n",
                    GetLastError()),
        inseye::c::InseyeInitializationStatus::kFailedToAccessSharedResources);
  }
  std::unique_ptr<inseye::internal::SharedMemoryHeader> shared_memory_header{
      inseye::internal::ReadHeaderInternal(memory_mapped_file.get())};
  std::unique_ptr<byte, std::function<void(void*)>> file_view{
      (byte*)MapViewOfFile(memory_mapped_file.get(),  // handle to map object
                           FILE_MAP_READ,             // read/write permission
                           0, 0, shared_memory_header->GetBufferSize()),
      UnmapViewOfFile};

  if (file_view == nullptr) {
    ThrowInitialization(
        std::format("Could not map view of file ({}).", GetLastError()),
        inseye::c::InseyeInitializationStatus::kFailedToMapSharedResources);
  }

  *pptr = new inseye::c::InseyeEyeTracker{
      std::move(memory_mapped_file), std::move(file_view), UNREAD_SAMPLE_INDEX,
      std::move(named_pipe_communicator), std::move(shared_memory_header)};
}
namespace inseye {
std::ostream& operator<<(std::ostream& os, const inseye::Version& p) {
  os << (long)p.major << "." << (long)p.minor << "." << (long)p.patch;
  return os;
}

std::ostream& operator<<(std::ostream& os, GazeEvent event) {
  switch (event) {
    case GazeEvent::kInsGazeNone:
      os << "None";
      break;
    case GazeEvent::kInsGazeBlinkLeft:
      os << "Blink Left";
      break;
    case GazeEvent::kInsGazeBlinkRight:
      os << "Blink Right";
      break;
    case GazeEvent::kInsGazeBlinkBoth:
      os << "Blink Both";
      break;
    case GazeEvent::kInsGazeSaccade:
      os << "Saccade";
      break;
    case GazeEvent::kInsGazeHeadsetMount:
      os << "HeadsetMount";
      break;
    case GazeEvent::kInsGazeHeadsetDismount:
      os << "HeadsetDismount";
      break;
    case GazeEvent::kUnknown:
      os << "Unknown";
      break;
    default:
      os << "Unknown (Invalid value of: " << (uint32_t)event << ")";
  }
  return os;
}

inseye::EyeTracker::EyeTracker(EyeTracker&& other) noexcept
    : implementation_pointer_(other.implementation_pointer_) {
  other.implementation_pointer_ = nullptr;
}

inseye::EyeTracker::~EyeTracker() noexcept {
  inseye::c::DestroyEyeTrackerReader(&implementation_pointer_);
}

bool inseye::EyeTracker::IsGazeDataAvailable() const noexcept {
  return inseye::c::IsGazeDataAvailable(implementation_pointer_);
}

bool inseye::EyeTracker::TryReadLatestEyeTrackerData(
    EyeTrackerDataStruct& eye_tracker_data_struct) noexcept {
  return TryReadLatestDataSampleInternal(*implementation_pointer_,
                                         eye_tracker_data_struct);
}

bool inseye::EyeTracker::TryReadNextEyeTrackerData(
    inseye::EyeTrackerDataStruct& eye_tracker_data_struct) noexcept {
  return TryReadNextDataSampleInternal(*implementation_pointer_,
                                       eye_tracker_data_struct, 0);
}

bool inseye::EyeTracker::TryReadLastEyeTrackerData(inseye::EyeTrackerDataStruct& out_data) const noexcept {
  return inseye::c::TryReadLastEyeTrackerData(implementation_pointer_, &out_data);
}

bool inseye::Version::operator==(const inseye::Version& other) const {
  return !(*this != other);
}
bool inseye::Version::operator!=(const inseye::Version& other) const {
  if (this->major != other.major)
    return true;
  if (this->minor != other.minor)
    return true;
  if (this->patch != other.patch)
    return true;
  return false;
}
bool inseye::Version::operator<(const inseye::Version& other) const {
  if (major < other.major)
    return true;
  if (major > other.major)
    return false;
  if (minor < other.minor)
    return true;
  if (minor > other.minor)
    return false;
  return patch < other.patch;
}

bool inseye::Version::operator>(const inseye::Version& other) const {
  return other < *this;
}

bool inseye::Version::operator>=(const inseye::Version& other) const {
  return !(*this < other);
}
bool inseye::Version::operator<=(const inseye::Version& other) const {
  return !(*this > other);
}

inseye::c::InseyeInitializationStatus inseye::c::CreateEyeTrackerReader(
    inseye::c::InseyeEyeTracker** pptr, uint32_t timeout_ms) {
  try {
    CreateEyeTrackerReaderInternal(
        pptr, [start_time = std::chrono::system_clock::now(), timeout_ms]() {
          if (duration_cast<std::chrono::milliseconds>(
                  (std::chrono::system_clock::now() - start_time)) >
              std::chrono::milliseconds(timeout_ms)) {
            return true;
          }
          return false;
        });
    return inseye::c::InseyeInitializationStatus::kSuccess;
  } catch (const InitializationException& initializationException) {
    return initializationException.status;
  } catch (const inseye::internal::NamedPipeException& named_pipe_exception) {
    WriteErrorMessage(named_pipe_exception.error_message);
    return inseye::c::kInsFailedToInitializeNamedPipe;
  }
}

void inseye::c::DestroyEyeTrackerReader(inseye::c::InseyeEyeTracker** pptr) {
  if (pptr == nullptr)
    return;
  if (*pptr == nullptr)
    return;
  delete *pptr;
  *pptr = nullptr;
}

bool inseye::c::IsGazeDataAvailable(
    struct inseye::c::InseyeEyeTracker* pointer) {
  if (pointer == nullptr)
    return false;
  auto samples_written_count = pointer->shared_memory_header->ReadSamplesWrittenCount();
  if (samples_written_count == UNWRITTEN_SAMPLE_INDEX)
    return false;  // service has not written any data to shared memory
  return pointer->shared_memory_header->ReadSamplesWrittenCount() >
         pointer->lastSampleIndex;
}

bool inseye::c::TryReadNextEyeTrackerData(
    inseye::c::InseyeEyeTracker* pImpl,
    inseye::c::InseyeEyeTrackerDataStruct* pDataStruct) {
  if (pImpl == nullptr || pDataStruct == nullptr)
    return false;
  return TryReadNextDataSampleInternal(*pImpl, *pDataStruct, 0);
}

bool inseye::c::TryReadLatestEyeTrackerData(
    inseye::c::InseyeEyeTracker* implementation,
    inseye::c::InseyeEyeTrackerDataStruct* data_struct) {
  if (implementation == nullptr || data_struct == nullptr)
    return false;
  auto latest_written =
      implementation->shared_memory_header->ReadSamplesWrittenCount();
  implementation->lastSampleIndex =
      (std::max)(implementation->lastSampleIndex, latest_written - 1);
  return TryReadNextDataSampleInternal(*implementation, *data_struct, 0);
}

bool inseye::c::TryReadLastEyeTrackerData(
    struct inseye::c::InseyeEyeTracker* implementation,
    struct inseye::c::InseyeEyeTrackerDataStruct* data_struct) {
  if (implementation == nullptr || data_struct == nullptr)
    return false;
  auto header = implementation->shared_memory_header.get();
  auto latest_written = header->ReadSamplesWrittenCount();
  auto latest_read = implementation->lastSampleIndex;
  auto samples = header->GetSampleCount();
  if ((latest_written - latest_read) > samples)
    return false;
  ReadDataSampleInternal(*implementation, *data_struct);
  latest_written = header->ReadSamplesWrittenCount();
  if ((latest_written - latest_read) > samples)
    return false; // check if what we read was not overwritten
  return true;
}

}  // namespace inseye