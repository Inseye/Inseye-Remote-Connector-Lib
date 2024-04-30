#include <iostream>
#include <chrono>
#include <thread>
#include "windows.h"
#include "remote_connector.h"

static bool run = true;

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

int main(int argc, char *argv[]) {
  using namespace std::chrono_literals;
  try {
    inseye::EyeTracker reader(-1);
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
  catch (std::runtime_error& ref){
    std::cout << ref.what() << std::endl;
  }
}