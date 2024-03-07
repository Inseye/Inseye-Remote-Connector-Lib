//
// Created by Mateusz on 28.02.2024.
//

#ifndef REMOTE_CONNECTOR_LIB_NAMED_PIPE_COMMUNICATOR_HPP
#define REMOTE_CONNECTOR_LIB_NAMED_PIPE_COMMUNICATOR_HPP
#include <memory>
#include <array>
#include <algorithm>
#include <functional>
#include <string>
#include <exception>
#include <mutex>
#include "remote_connector.h"
namespace inseye::internal {


struct ServiceInfo {
  inseye::Version service_version;
  std::string shared_buffer_path;
};

class NamedPipeException : public std::exception {
 public:
  const std::string error_message;
  explicit NamedPipeException(const std::string &message) : error_message(message) {}
  explicit NamedPipeException(std::string &&message) : error_message(std::move(message)) {}
};


class NamedPipeCommunicator {
  std::mutex mutex;
  std::unique_ptr<void, std::function<void(void*)>> pipe_handle;
  explicit NamedPipeCommunicator(std::unique_ptr<void, std::function<void(void*)>> &&pipe_handle);
 public:
  static NamedPipeCommunicator Create(const std::function<bool()>& should_cancel_function);
  NamedPipeCommunicator(NamedPipeCommunicator &) = delete;
  NamedPipeCommunicator(NamedPipeCommunicator &&other) noexcept;
  ServiceInfo GetServiceInfo();
};

}
#endif  //REMOTE_CONNECTOR_LIB_NAMED_PIPE_COMMUNICATOR_HPP
