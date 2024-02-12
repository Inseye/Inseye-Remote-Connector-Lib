//
// Created by Mateusz on 08.02.2024.
//

#include <sstream>
#include <utility>
#include <windows.h>
#include "remote_connector.h"
#include "remote_connector.hpp"
#include "shared_memory_header.hpp"
#include "endianess_helpers.hpp"
#include "version.hpp"

using namespace inseye::internal;
constexpr uint32_t MINIMUM_HEADER_METADATA_SIZE = sizeof(PackedVersion);
using major_version_t = decltype( PackedVersion::major);
using minor_version_t = decltype( PackedVersion::minor);
using patch_version_t = decltype( PackedVersion::patch);

constexpr PackedVersion lowest_supported = {0, 0, 1};
constexpr PackedVersion highest_supported = {1, 0, 0};
const Version kLowestSupportedServiceVersion = {
    lowest_supported.major, lowest_supported.minor, lowest_supported.patch
};
const Version kHighestSupportedServiceVersion = {
    highest_supported.major, highest_supported.minor, highest_supported.patch
};
const inseye::Version inseye::lowest_supported_service_version = {
    lowest_supported.major, lowest_supported.minor, lowest_supported.patch
};
const inseye::Version inseye::highest_supported_service_version = {
    highest_supported.major, highest_supported.minor, highest_supported.patch
};

#pragma pack(push, 1)
struct InMemoryV1 {
  InMemoryV1() = delete;
  PackedVersion version;
  uint32_t header_size;
  uint32_t buffer_size;
  uint32_t sample_size;
  volatile uint32_t samples_written;
};
#pragma pack(pop)

class SharedMemoryHeaderV1 final : SharedMemoryHeader {
  std::unique_ptr<InMemoryV1, std::function<void (InMemoryV1*)>> header_memory_;
  const inseye::Version version_;
  const uint32_t header_size_;
  const uint32_t sample_size_;
  const uint32_t buffer_size_;
  const uint32_t total_samples_count_;

public:
  static constexpr inseye::Version minimumVersion = {
      lowest_supported.major, lowest_supported.minor, lowest_supported.patch
  };
  static constexpr inseye::Version maximumVersion = {
      highest_supported.major, highest_supported.minor, highest_supported.patch
  };

  SharedMemoryHeaderV1(
      std::unique_ptr<InMemoryV1, std::function<void (InMemoryV1*)>> memory,
      const inseye::Version &version,
      uint32_t header_size,
      uint32_t sample_size,
      uint32_t buffer_size,
      uint32_t total_samples_count);

  [[nodiscard]] const inseye::Version& GetVersion() const override;

  [[nodiscard]] uint32_t ReadSamplesWrittenCount() const override;

  [[nodiscard]] const uint32_t& GetHeaderSize() const override;

  [[nodiscard]] const uint32_t& GetDataSampleSize() const override;

  [[nodiscard]] const uint32_t& GetSampleCount() const override;
  [[nodiscard]] const uint32_t& GetBufferSize() const override;

  ~SharedMemoryHeaderV1() override;

};


SharedMemoryHeaderV1* createSharedMemoryHeaderV1(HANDLE shared_memory_handle, const inseye::Version& version) {
  auto mapped_memory = std::unique_ptr<
    InMemoryV1, std::function<void (InMemoryV1*)>>(
      static_cast<InMemoryV1*>(MapViewOfFile(shared_memory_handle,
                                             FILE_MAP_READ, 0, 0,
                                             sizeof(InMemoryV1))),
      [](const InMemoryV1* cleaned) -> void {
        UnmapViewOfFile(cleaned);
      });
  auto buffer_size = read_swap_endianes_if_needed<uint32_t>(&mapped_memory->buffer_size);
  auto header_size = read_swap_endianes_if_needed<uint32_t>(&mapped_memory->header_size);
  auto sample_size = read_swap_endianes_if_needed<uint32_t>(&mapped_memory->sample_size);
  return new SharedMemoryHeaderV1(std::move(mapped_memory), version,
      header_size,
      sample_size,
      buffer_size,
      (buffer_size - header_size) / sample_size);
}

SharedMemoryHeaderV1::~SharedMemoryHeaderV1() = default;

const inseye::Version& SharedMemoryHeaderV1::GetVersion() const {
  return version_;
}

SharedMemoryHeaderV1::SharedMemoryHeaderV1(
    std::unique_ptr<InMemoryV1, std::function<void (InMemoryV1*)>> memory,
    const inseye::Version &version,
    const uint32_t header_size,
    const uint32_t sample_size,
    const uint32_t buffer_size,
    const uint32_t total_samples_count
    ) : header_memory_(std::move(memory)), version_(version),
        header_size_(header_size), sample_size_(sample_size),
        buffer_size_(buffer_size), total_samples_count_(total_samples_count) {}

SharedMemoryHeader* inseye::internal::readHeaderInternal(
    HANDLE share_file_handle) {
  // map as little memory as required
  auto memory = static_cast<LPBYTE>(MapViewOfFile(
      share_file_handle,
      FILE_MAP_READ,
      0, 0, sizeof(PackedVersion)));
  // check header version
  const Version headerVersion = {
      read_swap_endianes_if_needed<major_version_t>(
          reinterpret_cast<major_version_t*>(memory + offsetof(
                                                 PackedVersion, major))),
      read_swap_endianes_if_needed<minor_version_t>(
          reinterpret_cast<minor_version_t*>(memory + offsetof(
                                                 PackedVersion, minor))),
      read_swap_endianes_if_needed<patch_version_t>(
          reinterpret_cast<patch_version_t*>(memory + offsetof(
                                                 PackedVersion, patch)))
  };
  UnmapViewOfFile(memory);
  if (headerVersion < SharedMemoryHeaderV1::minimumVersion) {
    std::stringstream ss;
    ss << "Library doesn't support service in version: " << headerVersion <<
        "lowest supported version is: " <<
        lowest_supported_service_version;
    throw CombinedException(ss.str(), kServiceVersionToLow);
  }
  if (headerVersion >= SharedMemoryHeaderV1::minimumVersion && headerVersion <
      SharedMemoryHeaderV1::maximumVersion) {
    return reinterpret_cast<SharedMemoryHeader*>(createSharedMemoryHeaderV1(share_file_handle, headerVersion));
  }
  // Add new ifs there when new data formats are added
  else {
    std::stringstream ss;
    ss << "Library doesn't support service in version: " << headerVersion <<
        ", highest  supported version is: " <<
        highest_supported_service_version;
    throw CombinedException(ss.str(), kServiceVersionToHigh);
  }
}

uint32_t SharedMemoryHeaderV1::ReadSamplesWrittenCount() const {
  auto samples_written = header_memory_->samples_written;
  return read_swap_endianes_if_needed<decltype(samples_written)>(&samples_written);
}

const uint32_t& SharedMemoryHeaderV1::GetHeaderSize() const {
  return header_size_;
}

const uint32_t& SharedMemoryHeaderV1::GetDataSampleSize() const {
  return sample_size_;
}

const uint32_t& SharedMemoryHeaderV1::GetSampleCount() const {
  return total_samples_count_;
}

const uint32_t& SharedMemoryHeaderV1::GetBufferSize() const {
  return buffer_size_;
}