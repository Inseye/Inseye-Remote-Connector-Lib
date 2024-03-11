//
// Created by Mateusz on 27.02.2024.
//

#ifndef REMOTE_CONNECTOR_LIB_ERRORS_HPP
#define REMOTE_CONNECTOR_LIB_ERRORS_HPP
#include <cassert>
#include <string>
#include <array>
#include <cstring>
#include "remote_connector.h"

class InitializationException : public std::exception {
 public:
  const inseye::c::InseyeInitializationStatus status;
  explicit InitializationException(inseye::c::InseyeInitializationStatus status) : status(status) {}
};

void WriteErrorMessage(std::array<char, 1024> message);
void WriteErrorMessage(const std::string& message);
inseye::c::InseyeInitializationStatus ThrowInitialization(std::array<char, 1024> message, inseye::c::InseyeInitializationStatus status);
inseye::c::InseyeInitializationStatus ThrowInitialization(const std::string &message, inseye::c::InseyeInitializationStatus status);
void ThrowIfCancellationRequested(const std::function<bool ()> &is_cancellation_requested);

#endif  //REMOTE_CONNECTOR_LIB_ERRORS_HPP
