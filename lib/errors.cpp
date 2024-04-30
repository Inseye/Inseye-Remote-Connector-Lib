// Last edit: 30.04.24 11:03 by Mateusz Chojnowski mateusz.chojnowski@inseye.com
//
// Copyright (c) Inseye Inc. 2024.
//
// This file is part of Inseye Software Development Kit subject to Inseye SDK License
// See  https://github.com/Inseye/Licenses/blob/master/SDKLicense.txt.
// All other rights reserved.

#include "remote_connector.h"
#include "errors.hpp"
constexpr int message_buffer_size = 1024;
thread_local char messageBuffer[message_buffer_size]{'\0'};

void WriteErrorMessage(std::array<char, message_buffer_size> message) {
  std::copy(message.begin(), message.end(), messageBuffer);
}

void WriteErrorMessage(const std::string& message) {
  auto length = message.length();
  if (message.length() >= message_buffer_size) {
    length = message_buffer_size - 1;
  }
  for (int i = 0; i < length; ++i) {
    messageBuffer[i] = message[i];
  }
  messageBuffer[length] = '\0';
}

inseye::c::InseyeInitializationStatus ThrowInitialization(
    std::array<char, message_buffer_size> message,
    inseye::c::InseyeInitializationStatus status) {
  WriteErrorMessage(message);
  throw InitializationException(status);
}

inseye::c::InseyeInitializationStatus ThrowInitialization(
    const std::string& message, inseye::c::InseyeInitializationStatus status) {
  WriteErrorMessage(message);
  throw InitializationException(status);
}

void ThrowIfCancellationRequested(const std::function<bool ()> &is_cancellation_requested)
{
  static const std::string cancelled = "Cancelled";
  if (is_cancellation_requested()) {
    ThrowInitialization(cancelled, inseye::c::InseyeInitializationStatus::kCancelled);
  }
}

char* inseye::c::GetLastErrorDescription() {
  return messageBuffer;
}