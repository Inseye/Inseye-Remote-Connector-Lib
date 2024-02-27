#include "remote_connector.h"
#include <windows.h>
#include <cassert>
#include <cmath>
#include <format>

#include "eye_tracker_data_struct.hpp"
#include "shared_memory_header.hpp"
TCHAR szMappedMemoryName[] =
    TEXT("Local\\Inseye-Remote-Connector-Shared-Memory");
TCHAR named_pipe_name[] = TEXT("\\\\\\\\.\\\\pipe\\\\inseye.desktop-service");
constexpr uint32_t UNREAD_SAMPLE_INDEX = 0;
constexpr uint32_t UNWRITTEN_SAMPLE_INDEX = -1;
static_assert(sizeof(enum inseye::c::GazeEvent) == sizeof(uint32_t),
              "Incompatible type size");
static_assert((inseye::c::kBlinkLeft ^ ((uint32_t)1)) == 0,
              "Incompatible binary layout");

struct inseye::c::SharedMemoryEyeTrackerReader {
  HANDLE mapped_file_handle = nullptr;
  LPBYTE in_memory_buffer_pointer = nullptr;
  uint32_t lastSampleIndex = UNREAD_SAMPLE_INDEX;
  std::unique_ptr<inseye::internal::SharedMemoryHeader> shared_memory_header =
      nullptr;
};

/**
 * \brief Frees resources held by CommonData
 * \param data reference to structure holding resources
 */
inline void DisposeEyeTrackerReaderInternal(
    inseye::c::SharedMemoryEyeTrackerReader& data) {
  if (data.mapped_file_handle != nullptr) {
    UnmapViewOfFile(data.in_memory_buffer_pointer);
    data.in_memory_buffer_pointer = nullptr;
  }
  if (data.mapped_file_handle != nullptr) {
    CloseHandle(data.mapped_file_handle);
    data.mapped_file_handle = nullptr;
  }
}

inline void DestroyEyeTrackerReaderInternal(
    inseye::c::SharedMemoryEyeTrackerReader* ptr) {
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

bool TryReadNextDataSampleInternal(inseye::c::SharedMemoryEyeTrackerReader&
                                       commonData,  // NOLINT(*-no-recursion)
                                   inseye::c::EyeTrackerDataStruct& dataStruct,
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

bool TryReadLatestDataSampleInternal(
    inseye::c::SharedMemoryEyeTrackerReader& implementation,
    inseye::c::EyeTrackerDataStruct& data_struct) {
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
    inseye::c::SharedMemoryEyeTrackerReader** pptr) {
  inseye::c::SharedMemoryEyeTrackerReader impl;
  HANDLE pipeHandle = CreateFile(named_pipe_name, GENERIC_READ | GENERIC_WRITE,
                                 0, nullptr, OPEN_EXISTING, 0, nullptr);
  // if (pipeHandle != INVALID_HANDLE_VALUE)
  HANDLE handle = impl.mapped_file_handle =
      OpenFileMapping(FILE_MAP_READ,  // use paging file
                      FALSE, szMappedMemoryName);
  if (impl.mapped_file_handle == nullptr) {
    DisposeEyeTrackerReaderInternal(impl);
    throw inseye::CombinedException(
        std::format("Could not open file mapping object ({}).\n",
                    GetLastError()),
        inseye::c::InitializationStatus::kFailedToAccessSharedResources);
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
        inseye::c::InitializationStatus::kFailedToMapSharedResources);
  }

  *pptr = new inseye::c::SharedMemoryEyeTrackerReader{
      impl.mapped_file_handle, impl.in_memory_buffer_pointer,
      impl.lastSampleIndex, std::move(impl.shared_memory_header)};
}

inseye::SharedMemoryEyeTrackerReader::SharedMemoryEyeTrackerReader() {
  inseye::c::SharedMemoryEyeTrackerReader* ptr = nullptr;
  CreateEyeTrackerReaderInternal(&ptr);
  static auto lamb = [](inseye::c::SharedMemoryEyeTrackerReader* p) {
    DestroyEyeTrackerReaderInternal(p);
  };
  implementatation_pointer_ =
      std::unique_ptr<inseye::c::SharedMemoryEyeTrackerReader, decltype(lamb)>{
          ptr, lamb};
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

inseye::c::InitializationStatus inseye::c::CreateEyeTrackerReader(
    inseye::c::SharedMemoryEyeTrackerReader** pptr) {
  try {
    CreateEyeTrackerReaderInternal(pptr);
    return inseye::c::InitializationStatus::kSuccess;
  } catch (const inseye::CombinedException& exception) {
    return exception.GetStatusCode();
  }
}

void inseye::c::DestroyEyeTrackerReader(
    inseye::c::SharedMemoryEyeTrackerReader** pptr) {
  if (pptr == nullptr)
    return;
  if (*pptr == nullptr)
    return;
  DestroyEyeTrackerReaderInternal(*pptr);
  *pptr = nullptr;
}

bool inseye::c::TryReadNextEyeTrackerData(
    inseye::c::SharedMemoryEyeTrackerReader* pImpl,
    inseye::c::EyeTrackerDataStruct* pDataStruct) {
  if (pImpl == nullptr || pDataStruct == nullptr)
    return false;
  return TryReadNextDataSampleInternal(*pImpl, *pDataStruct, 0);
}

bool inseye::c::TryReadLatestEyeTrackerData(
    inseye::c::SharedMemoryEyeTrackerReader* implementation,
    inseye::c::EyeTrackerDataStruct* data_struct) {
  if (implementation == nullptr || data_struct == nullptr)
    return false;
  auto latest_written =
      implementation->shared_memory_header->ReadSamplesWrittenCount();
  implementation->lastSampleIndex =
      (std::max)(implementation->lastSampleIndex, latest_written - 1);
  return TryReadNextDataSampleInternal(*implementation, *data_struct, 0);
}

bool inseye::Version::operator==(const inseye::Version& other) const {
  return !(*this != other);
}
bool inseye::Version::operator!=(const inseye::Version& other) const {
  if (this->major != other.major)
    return false;
  if (this->minor != other.minor)
    return false;
  if (this->patch != other.patch)
    return false;
  return true;
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
bool inseye::Version::operator>=(const inseye::Version& other) const {
  return !(*this < other);
}
bool inseye::Version::operator<=(const inseye::Version& other) const {
  return !(*this > other);
}
namespace inseye {
std::ostream& operator<<(std::ostream& os, const inseye::Version& p) {
  os << (long)p.major << "." << (long)p.minor << "." << (long)p.patch;
  return os;
}

std::ostream& operator<<(std::ostream& os, GazeEvent event) {
  switch (event) {
    case GazeEvent::kNone:
      os << "None";
      break;
    case GazeEvent::kBlinkLeft:
      os << "Blink Left";
      break;
    case GazeEvent::kBlinkRight:
      os << "Blink Right";
      break;
    case GazeEvent::kBlinkBoth:
      os << "Blink Both";
      break;
    case GazeEvent::kSaccade:
      os << "Saccade";
      break;
    case GazeEvent::kHeadsetMount:
      os << "HeadsetMount";
      break;
    case GazeEvent::kHeadsetDismount:
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
}  // namespace inseye