//
// Created by Mateusz on 09.02.2024.
//

#ifndef EYE_TRACKER_DATA_STRUCT_HPP
#define EYE_TRACKER_DATA_STRUCT_HPP
#include "remote_connector.h"
#include "remote_connector.hpp"
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

template <typename T>
inline void readCommon(LPBYTE offsetedMemory, T& dataStruct) {
  using time_t = decltype(EyeTrackerDataStruct::time);
  using pos_t = decltype(EyeTrackerDataStruct::left_eye_x);
  dataStruct.time = read_swap_endianes_if_needed<time_t>(
      reinterpret_cast<time_t*>(offsetedMemory + offsetof(
                                    EyeTrackerDataStruct, time)));
  dataStruct.left_eye_x = read_swap_endianes_if_needed<pos_t>(
      reinterpret_cast<pos_t*>(offsetedMemory + offsetof(
                                   EyeTrackerDataStruct, left_eye_x)));
  dataStruct.left_eye_y = read_swap_endianes_if_needed<pos_t>(
      reinterpret_cast<pos_t*>(offsetedMemory + offsetof(
                                   EyeTrackerDataStruct, left_eye_y)));
  dataStruct.right_eye_x = read_swap_endianes_if_needed<pos_t>(
      reinterpret_cast<pos_t*>(offsetedMemory + offsetof(
                                   EyeTrackerDataStruct, right_eye_x)));
  dataStruct.right_eye_y = read_swap_endianes_if_needed<pos_t>(
      reinterpret_cast<pos_t*>(offsetedMemory + offsetof(
                                   EyeTrackerDataStruct, right_eye_y)));
}


inline void readDataSample(LPBYTE offsetedMemory,
                           inseye::EyeTrackerDataStruct& dataStruct) {
  readCommon<inseye::EyeTrackerDataStruct>(offsetedMemory, dataStruct);
  uint32_t read_value = read_swap_endianes_if_needed<uint32_t>(
    reinterpret_cast<uint32_t *>(offsetedMemory + offsetof(inseye::EyeTrackerDataStruct, gaze_event)));
  if (read_value > static_cast<uint32_t>(GazeEvent::HeadsetDismount)) // last value
    read_value = static_cast<uint32_t>(GazeEvent::HeadsetDismount);
  dataStruct.gaze_event = static_cast<GazeEvent>(read_value);
}

inline void readDataSample(LPBYTE offsetedMemory, ::EyeTrackerDataStruct& dataStruct) {
  readCommon<::EyeTrackerDataStruct>(offsetedMemory, dataStruct);
  uint32_t read_value = read_swap_endianes_if_needed<uint32_t>(
    reinterpret_cast<uint32_t *>(offsetedMemory + offsetof(inseye::EyeTrackerDataStruct, gaze_event)));
  if (read_value >= static_cast<uint32_t>(kUnknownGazeEvent)) // last value in enum
    read_value = static_cast<uint32_t>(kUnknownGazeEvent);
  dataStruct.gaze_event = static_cast<::GazeEvent>(read_value);
}

}
#endif //EYE_TRACKER_DATA_STRUCT_HPP