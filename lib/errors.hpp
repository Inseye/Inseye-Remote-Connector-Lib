// Last edit: 30.04.24 11:03 by Mateusz Chojnowski mateusz.chojnowski@inseye.com
//
// Copyright (c) Inseye Inc. 2024.
//
// This file is part of Inseye Software Development Kit subject to Inseye SDK License
// See  https://github.com/Inseye/Licenses/blob/master/SDKLicense.txt.
// All other rights reserved.

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
