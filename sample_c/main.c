// Last edit: 30.04.24 10:16 by Mateusz Chojnowski mateusz.chojnowski@inseye.com
//
// Copyright (c) Inseye Inc. 2024.
//
// This file is part of Inseye Software Development Kit subject to Inseye SDK License
// See  https://github.com/Inseye/Licenses/blob/master/SDKLicense.txt.
// All other rights reserved.

#include <stdio.h>
#include <Windows.h>
#include "remote_connector.h"

volatile static bool run = true;


BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
  switch (fdwCtrlType)
  {
    case CTRL_C_EVENT:
    case CTRL_CLOSE_EVENT:
    case CTRL_BREAK_EVENT:
      run = false;
      return TRUE;
    case CTRL_LOGOFF_EVENT:
    case CTRL_SHUTDOWN_EVENT:
    default:
      return FALSE;
  }
}

void print_data(struct InseyeEyeTrackerDataStruct*eye_tracker_data) {
  printf("Successfully read data\n");
  printf("Time: %llu\n", eye_tracker_data->time);
  printf("Left X: %f\n", eye_tracker_data->left_eye_x);
  printf("Left Y: %f\n", eye_tracker_data->left_eye_y);
  printf("Right X: %f\n", eye_tracker_data->right_eye_x);
  printf("Right Y: %f\n", eye_tracker_data->right_eye_y);
  printf("Event: %ud", eye_tracker_data->gaze_event);
}

int main() {
  printf("max version %d.%d.%d\n", kHighestSupportedServiceVersion.major, kHighestSupportedServiceVersion.minor, kHighestSupportedServiceVersion.patch);
  struct InseyeEyeTracker* reader_ptr = NULL;
  enum InseyeInitializationStatus initStatus = CreateEyeTrackerReader(&reader_ptr, -1);
  if (initStatus != kSuccess)
  {
    printf("Failed to initialize eye tracker reader %u\n%s", initStatus, GetLastErrorDescription());
    return 1;
  }
  printf("Reader successfully initialized.\n");
  struct InseyeEyeTrackerDataStruct eyeTrackerData;
  SetConsoleCtrlHandler(CtrlHandler, TRUE);
  while (run) {
    if (TryReadLatestEyeTrackerData(reader_ptr, &eyeTrackerData)) {
      print_data(&eyeTrackerData);
    } else {
      printf("Failed to read data\n");
      Sleep(5000);
    }
    printf("\n");
  }
  DestroyEyeTrackerReader(&reader_ptr);
}