// Last edit: 30.04.24 11:03 by Mateusz Chojnowski mateusz.chojnowski@inseye.com
//
// Copyright (c) Inseye Inc. 2024.
//
// This file is part of Inseye Software Development Kit subject to Inseye SDK License
// See  https://github.com/Inseye/Licenses/blob/master/SDKLicense.txt.
// All other rights reserved.

#ifndef EYE_TRACKER_DATA_STRUCT_HPP
#define EYE_TRACKER_DATA_STRUCT_HPP
#include "remote_connector.h"
#include "endianess_helpers.hpp"

namespace inseye::internal {
// TODO: Make it cross compiler compatible
#pragma pack(push, 1)
typedef struct EyeTrackerDataStruct {
  uint64_t time;
  float left_eye_x;
  float left_eye_y;
  float right_eye_x;
  float right_eye_y;
  uint32_t gaze_event;
} EyeTrackerDataStruct;
#pragma pack(pop)

inline void readDataSample(LPBYTE offsetedMemory, inseye::EyeTrackerDataStruct& dataStruct) {
  using time_type = decltype(EyeTrackerDataStruct::time);
  using pos_type = decltype(EyeTrackerDataStruct::left_eye_x);
  dataStruct.time =
      read_swap_endianess_if_needed<time_type>(
      reinterpret_cast<time_type*>(offsetedMemory + offsetof(
                                                     EyeTrackerDataStruct, time)));
  dataStruct.left_eye_x =
      read_swap_endianess_if_needed<pos_type>(
      reinterpret_cast<pos_type*>(offsetedMemory + offsetof(
                                                    EyeTrackerDataStruct, left_eye_x)));
  dataStruct.left_eye_y =
      read_swap_endianess_if_needed<pos_type>(
      reinterpret_cast<pos_type*>(offsetedMemory + offsetof(
                                                    EyeTrackerDataStruct, left_eye_y)));
  dataStruct.right_eye_x =
      read_swap_endianess_if_needed<pos_type>(
      reinterpret_cast<pos_type*>(offsetedMemory + offsetof(
                                                    EyeTrackerDataStruct, right_eye_x)));
  dataStruct.right_eye_y =
      read_swap_endianess_if_needed<pos_type>(
      reinterpret_cast<pos_type*>(offsetedMemory + offsetof(
                                                    EyeTrackerDataStruct, right_eye_y)));
  auto read_value =
      read_swap_endianess_if_needed<uint32_t>(
    reinterpret_cast<uint32_t *>(offsetedMemory + offsetof(inseye::EyeTrackerDataStruct, gaze_event)));
  if (read_value >= static_cast<uint32_t>(GazeEvent::kUnknown)) // last value in enum
    read_value = static_cast<uint32_t>(GazeEvent::kUnknown);
  dataStruct.gaze_event = static_cast<GazeEvent>(read_value);
}

}
#endif //EYE_TRACKER_DATA_STRUCT_HPP