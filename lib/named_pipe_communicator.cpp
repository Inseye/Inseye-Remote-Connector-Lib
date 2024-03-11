//
// Created by Mateusz on 27.02.2024.
//

#include "named_pipe_communicator.hpp"
#include <windows.h>
#include <filesystem>
#include <format>
#include <memory>
#include "endianess_helpers.hpp"
#include "errors.hpp"
#include "remote_connector.h"
#include "version.hpp"
constexpr int maximum_pipe_message_length = 1024;
using buffer_t = std::array<byte, maximum_pipe_message_length>;
TCHAR named_pipe_name[] = TEXT("\\\\.\\pipe\\inseye.desktop-service");
DWORD pipe_mode = PIPE_READMODE_MESSAGE;

using namespace inseye::internal;

enum NamedPipeMessageType : uint32_t {
  ServiceInfoRequest = 0,
  ServiceInfoResponse = 1,
};

template <typename T>
concept ReadableMessage = requires(T impl, const buffer_t& source) {
  { T::ExpectedMessageType } -> std::convertible_to<NamedPipeMessageType>;
  {impl.ReadFrom(source)};
};

template <typename T>
concept WritableMessage = requires(T impl, buffer_t destination) {
  { impl.WriteTo(destination) } -> std::convertible_to<int>;
};

#pragma pack(push, 1)
typedef struct ServiceInfoResponse {
  const NamedPipeMessageType message_type =
      NamedPipeMessageType::ServiceInfoResponse;
  inseye::internal::PackedVersion version{0, 0, 0};
  std::array<char, maximum_pipe_message_length - sizeof(NamedPipeMessageType) -
                       sizeof(PackedVersion)>
      shared_memory_path{'\0'};
  static const NamedPipeMessageType ExpectedMessageType =
      NamedPipeMessageType::ServiceInfoResponse;
  void ReadFrom(const buffer_t& buffer) {
    auto naked_pointer =
        buffer.data() + offsetof(ServiceInfoResponse, message_type);
    version = read_swap_endianess_if_needed((PackedVersion*)naked_pointer);
    naked_pointer =
        buffer.data() + offsetof(ServiceInfoResponse, shared_memory_path);
    for (int i = 0; i < shared_memory_path.size(); ++i) {
      shared_memory_path[i] = *((char*)naked_pointer + i);
    }
  }
} ServiceInfoResponseMessage;
#pragma pack(pop)

#pragma pack(push, 1)
typedef struct ServiceInfoRequest {
  const NamedPipeMessageType MessageType =
      NamedPipeMessageType::ServiceInfoRequest;

 public:
  int WriteTo(buffer_t& destination) const {
    const auto messageType = (uint32_t)MessageType;
    write_swap_endianess_if_needed<uint32_t>(
        reinterpret_cast<uint32_t*>(
            destination.data() +
            offsetof(struct ServiceInfoRequest, MessageType)),
        &messageType);
    return sizeof(struct ServiceInfoRequest);
  }
} ServiceInfoRequestMessage;
#pragma pack(pop)

template <WritableMessage T>
bool WriteMessage(HANDLE fileHandle, T& message, int64_t& bytes_written) {
  buffer_t buffer{};
  DWORD bytes_to_write = message.WriteTo(buffer);
  return WriteFile((HANDLE)fileHandle, (LPCVOID)buffer.data(),
                   (DWORD)bytes_to_write, (LPDWORD)&bytes_written, nullptr);
}

template <ReadableMessage T>
auto ReadSpecificMessage(HANDLE fileHandle, T& reference) {
  buffer_t buffer{};
  DWORD bytes_read = 0;
  auto operation_successful =
      ReadFile(fileHandle, buffer.data(),
               (DWORD)(sizeof(byte) * maximum_pipe_message_length), &bytes_read,
               nullptr);
  if (!operation_successful) {
    auto error = GetLastError();
    if (error == ERROR_MORE_DATA)
      while (ReadFile(fileHandle,  // drain the pipe
                      buffer.data(),
                      (DWORD)(sizeof(byte) * maximum_pipe_message_length),
                      &bytes_read, nullptr)) {}
    throw NamedPipeException(
        std::format("NP:: Failed to read message. GLE={}\n", error));
  }
  if (bytes_read < sizeof(NamedPipeMessageType))
    throw NamedPipeException("NP:: Not enough bytes written by server");

  uint32_t read_type_as_uint =
      read_swap_endianess_if_needed(reinterpret_cast<uint32_t*>(buffer.data()));
  if (read_type_as_uint != T::ExpectedMessageType)
    throw NamedPipeException(std::format(
        "Invalid message type sent by the pipe server, message type: {}",
        read_type_as_uint));
  return reference.ReadFrom(buffer);
}

bool NamedPipeExists(const std::filesystem::path& pipePath) {
  std::wstring pipeName = pipePath;
  if ((pipeName.size() < 10) ||
      (pipeName.compare(0, 9, L"\\\\.\\pipe\\") != 0) ||
      (pipeName.find(L'\\', 9) != std::string::npos)) {
    // This can't be a pipe, so it also can't exist
    return false;
  }
  pipeName.erase(0, 9);
  WIN32_FIND_DATAW fd;
  HANDLE hFind = FindFirstFileW(L"\\\\.\\pipe\\*", &fd);
  do {
    if (pipeName == fd.cFileName) {
      FindClose(hFind);
      return true;
    }
  } while (FindNextFileW(hFind, &fd));
  FindClose(hFind);
  return false;
}

NamedPipeCommunicator NamedPipeCommunicator::Create(
    const std::function<bool()>& should_cancel_function) {
  auto pipe_handle = std::unique_ptr<void, std::function<void(void*)>>(
      CreateFile(named_pipe_name,               // lpFileName
                 GENERIC_READ | GENERIC_WRITE,  // dwDesiredAccess
                 0,                             //dwShareMode
                 nullptr,                       //lpSecurityAttributes
                 OPEN_EXISTING,                 // dwCreatin\disposition
                 0,                             // dwFlagsAndAttributes
                 nullptr                        // hTemplateFile
                 ),
      [](auto ptr) {
        if (ptr != INVALID_HANDLE_VALUE)
          CloseHandle(ptr);
      });
  ThrowIfCancellationRequested(should_cancel_function);
  if (pipe_handle.get() == INVALID_HANDLE_VALUE) {
    auto gle = GetLastError();
    if (gle == ERROR_PIPE_BUSY)
      ThrowInitialization("All desktop pipe instances are busy.",
                          inseye::c::InseyeInitializationStatus::
                              kInsAllServiceNamedPipesAreBusy);
    ThrowInitialization(
        std::format("Invalid named pipe handle, GLE={0}", gle),
        inseye::c::InseyeInitializationStatus::kInsFailedToInitializeNamedPipe);
  }
  //  if (!SetNamedPipeHandleState(pipe_handle.get(), &pipe_mode, nullptr, nullptr))
  //    throw_initialization(std::format("Failed to configure pipe, GLE={}", GetLastError()),
  //                         inseye::c::InseyeInitializationStatus::kInternalError);
  NamedPipeCommunicator namedPipeCommunicator(std::move(pipe_handle));
  return std::move(namedPipeCommunicator);
}

inseye::internal::NamedPipeCommunicator::NamedPipeCommunicator(
    std::unique_ptr<void, std::function<void(void*)>>&& pipe_handle)
    : mutex(), pipe_handle(std::move(pipe_handle)) {}

inseye::internal::NamedPipeCommunicator::NamedPipeCommunicator(
    inseye::internal::NamedPipeCommunicator&& other) noexcept
    : mutex(), pipe_handle(std::move(other.pipe_handle)) {}

ServiceInfo inseye::internal::NamedPipeCommunicator::GetServiceInfo() {
  std::lock_guard lock(this->mutex);
  int64_t bytes_written = 0;
  ServiceInfoRequestMessage msg;
  bool operation_success = WriteMessage(pipe_handle.get(), msg, bytes_written);
  if (!operation_success)
    throw NamedPipeException("Failed to send request for service info.");
  ServiceInfoResponseMessage response;
  ReadSpecificMessage(pipe_handle.get(), response);
  int string_terminator_index = 0;
  for (; string_terminator_index < response.shared_memory_path.size();
       ++string_terminator_index) {
    if (response.shared_memory_path[string_terminator_index] == '\0')
      break;
  }
  if (string_terminator_index == response.shared_memory_path.size()) {
    throw NamedPipeException("Failed to read shared memory path.");
  }
  return ServiceInfo{
      .service_version = {response.version.major, response.version.minor,
                          response.version.patch},
      .shared_buffer_path{response.shared_memory_path.data()}};
}

bool inseye::c::IsServiceAvailable() {
  return NamedPipeExists(named_pipe_name);
}