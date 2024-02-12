//
// Created by Mateusz on 05.02.2024.
//

#include <iostream>
#include <chrono>

#include <thread>
#include "windows.h"
#include "remote_connector.h"
#include "remote_connector.hpp"

static bool run = true;

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

template<typename T>
void print_data(T & eye_tracker_data) {
  std::cout << "Successfully read data\n";
  std::cout << "Time: " << eye_tracker_data.time << "\n";
  std::cout << "Left X: " << eye_tracker_data.left_eye_x << "\n";
  std::cout << "Left Y: " << eye_tracker_data.left_eye_y << "\n";
  std::cout << "Right X: " << eye_tracker_data.right_eye_x << "\n";
  std::cout << "Right Y: " << eye_tracker_data.right_eye_y << "\n";
  std::cout << "Event: " << eye_tracker_data.gaze_event;
}

void test_cpp_lib() {
  using namespace std::chrono_literals;
  try {
    inseye::SharedMemoryEyeTrackerReader reader;
    inseye::EyeTrackerDataStruct eyeTrackerData;
    SetConsoleCtrlHandler(CtrlHandler, TRUE);
    while (run) {
      if (reader.TryReadNextEyeTrackerData(eyeTrackerData)) {
        print_data(eyeTrackerData);
      } else {
        std::cout << "Failed to read data";
        std::this_thread::sleep_for(5s);
      }
      std::cout << std::endl;
    }
  }
  catch (inseye::CombinedException& ref){
    std::cout << ref.what() << std::endl;
  }
}

void test_c_lib() {
  using namespace std::chrono_literals;
  SharedMemoryEyeTrackerReader* reader_ptr = nullptr;
  auto initStatus = CreateEyeTrackerReader(&reader_ptr);
  if (initStatus != kSuccess)
  {
    printf("Failed to initialize eye tracker reader %u\n", initStatus);
    return;
  }
  printf("Reader successfully initialized.\n");
  inseye::SharedMemoryEyeTrackerReader reader;
  ::EyeTrackerDataStruct eyeTrackerData;
  SetConsoleCtrlHandler(CtrlHandler, TRUE);
  while (run) {
    if (TryReadLatestEyeTrackerData(reader_ptr, &eyeTrackerData)) {
      print_data(eyeTrackerData);
    } else {
      std::cout << "Failed to read data";
      std::this_thread::sleep_for(5s);
    }
    std::cout << std::endl;
  }
  DestroyEyeTrackerReader(&reader_ptr);
}

int main(int argc, char *argv[]) {
  // test_c_lib();
  test_cpp_lib();
}