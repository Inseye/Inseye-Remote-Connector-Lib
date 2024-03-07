//
// Created by Mateusz on 22.02.2024.
//
#include <stdio.h>
#include <Windows.h>
#include "remote_connector.h"

volatile static bool run = true;


BOOL WINAPI CtrlHandler(DWORD fdwCtrlType)
{
  switch (fdwCtrlType)
  {
      // Handle the CTRL-C signal.
    case CTRL_C_EVENT:
      printf("Ctrl-C event\n\n");
      Beep(750, 300);
      run = false;
      return TRUE;

      // CTRL-CLOSE: confirm that the user wants to exit.
    case CTRL_CLOSE_EVENT:
      Beep(600, 200);
      printf("Ctrl-Close event\n\n");
      run = false;
      return TRUE;

      // Pass other signals to the next handler.
    case CTRL_BREAK_EVENT:
      Beep(900, 200);
      printf("Ctrl-Break event\n\n");
      run = false;
      return TRUE;

    case CTRL_LOGOFF_EVENT:
      Beep(1000, 200);
      printf("Ctrl-Logoff event\n\n");
      return FALSE;

    case CTRL_SHUTDOWN_EVENT:
      Beep(750, 500);
      printf("Ctrl-Shutdown event\n\n");
      return FALSE;

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
    printf("Failed to initialize eye tracker reader %u\n%s", initStatus, kErrorDescription);
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