//
// Created by Mateusz on 27.02.2024.
//

#ifndef REMOTE_CONNECTOR_LIB_EXCEPTIONS_HPP
#define REMOTE_CONNECTOR_LIB_EXCEPTIONS_HPP
#include <cassert>
#include <string>
#include <array>
#include <cstring>
#include "remote_connector.h"

using internal_exception_type = inseye::c::InseyeInitializationStatus;

class InitializationException : std::exception {
 public:
  const inseye::c::InseyeInitializationStatus status;
  explicit InitializationException(inseye::c::InseyeInitializationStatus status) : status(status) {}
};

inline void write_error_message(std::array<char, 1024> message) {
  std::copy(message.begin(), message.end(), inseye::c::kErrorDescription);
}

inline inseye::c::InseyeInitializationStatus throw_initialization(std::array<char, 1024> message, inseye::c::InseyeInitializationStatus status) {
  write_error_message(message);
  throw InitializationException(status);
}

inline inseye::c::InseyeInitializationStatus throw_initialization(const std::string &message, inseye::c::InseyeInitializationStatus status) {
  // assert(message.length() < 1024);
  auto length = message.length();
  if (message.length() >= 1024) {
    length = 1023;
  }
  for (int i = 0; i < length; ++i) {
    inseye::c::kErrorDescription[i] = message[i];
  }
  inseye::c::kErrorDescription[length] = '\0';
  throw InitializationException(status);
}

inline void ThrowIfCancellationRequested(const std::function<bool ()> &is_cancellation_requested) {
  if (is_cancellation_requested()) {
    std::strcpy(inseye::c::kErrorDescription, "Cancelled");
    throw InitializationException(
        inseye::c::InseyeInitializationStatus::kCancelled);
  }
}

#endif  //REMOTE_CONNECTOR_LIB_EXCEPTIONS_HPP
