#ifndef REMOTE_CONNECTOR_H_
#define REMOTE_CONNECTOR_H_

#if !defined(LIB_EXPORT)
#define LIB_EXPORT /* NOTHING */
#if defined(WIN32) || defined(WIN64)
#undef LIB_EXPORT
#if defined(Remote_Connector_Dlib_EXPORTS)
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT __declspec(dllimport)
#endif  // defined(Remote_Connector_Dlib_EXPORTS)
#endif  // defined(WIN32) || defined(WIN64)

#if defined(__GNUC__) || defined(__APPLE__) || defined(LINUX)
#if defined(Remote_Connector_Dlib_EXPORTS)
#undef LIB_EXPORT
#define LIB_EXPORT __attribute__((visibility("default")))
#endif  // defined(DLib_EXPORTS)
#endif  // defined(__GNUC__) || defined(__APPLE__) || defined(LINUX)

#endif  // !defined(LIB_EXPORT)

#if !defined(CALL_CONV)
#if defined(WIN32) || defined(WIN64)
#define CALL_CONV __cdecl
#else
#define CALL_CONV __attribute__((__cdecl__))
#endif  // defined(WIN32) || defined(WIN64)
#endif  // !defined(CALL_CONV)

extern "C" {
#include <stdbool.h>
#include <stdint.h>

enum InitializationStatus {
  kSuccess,
  kFailedToAccessSharedResources,
  kFailedToMapSharedResources,
  kBufferSmallerThanMinimumHeaderSize,
  kServiceVersionToLow,
  kServiceVersionToHigh,
  kFailure
};

enum GazeEvent : uint32_t {
  /**
   * Nothing particular happened
   */
  kNone = 0,
  /**
   * Left eye is closed or blinked
   */
  kBlinkLeft = 1 << 0,
  /**
   * Right eye is closed or blinked
   */
  kBlinkRight = 1 << 1,
  /**
   * Both eye are closed or both eye performed blink
   */
  kBlinkBoth = 1 << 2,
  /**
   * Saccade occurred
   */
  kSaccade = 1 << 3,
  /**
   * Headset was put on by the user
   */
  kHeadsetMount = 1 << 4,
  /**
   * Headset was put off by the user
   */
  kHeadsetDismount = 1 << 5,
  /**
   * Unknown event that was introduced in later version of service
   */
  kUnknownGazeEvent = kHeadsetDismount << 1
};

struct SharedMemoryEyeTrackerReader;

typedef struct Version {
  const uint32_t major;
  const uint32_t minor;
  const uint32_t patch;
} Version;

extern const Version kLowestSupportedServiceVersion;
extern const Version kHighestSupportedServiceVersion;

typedef struct EyeTrackerDataStruct {
  /**
   * @brief Data creation timestamp in milliseconds since Unix Epoch.
   */
  uint64_t time;
  /**
   * @brief Left eye horizontal angle position in radians.
   * Angle is measurement of rotation between vector parallel to user left eye
   * gaze direction and normal vector of device (headset) field of view and
   * formed on plane horizontal to device (head) orientation.
   * Value must be in range of (-half of device horizontal field of view, half
   * of device horizontal field of view) where positive value represent rotation
   * of user gaze to the right and negative value correspond to the gaze
   * rotation to the left (from user PoV).
   */
  float left_eye_x;
  /**
   * Left eye vertical angle position in radians.
   * Angle is measurement of rotation between vector parallel to user left eye
   * gaze direction and normal vector of device (headset) field of view and
   * formed on plane vertical to device (head) orientation.
   * Value must be in range of (-half of device vertical field of view, half of
   * device vertical field of view) where positive value represent rotation of
   * user gaze up and negative value correspond to the gaze down
   * (from user PoV).
   */
  float left_eye_y;
  /**
   * @brief Right eye horizontal angle position in radians.
   * Angle is measurement of rotation between vector parallel to user right eye
   * gaze direction and normal vector of device (headset) field of view and
   * formed on plane horizontal to device (head) orientation.
   * Value must be in range of (-half of device horizontal field of view, half
   * of device horizontal field of view) where positive value represent rotation
   * of user gaze to the right and negative value correspond to the gaze
   * rotation to the left (from user PoV).
   */
  float right_eye_x;
  /**
   * @brief Right eye vertical angle position in radians.
   * Angle is measurement of rotation between vector parallel to user left eye
   * gaze direction and normal vector of device (headset) field of view and
   * formed on plane vertical to device (head) orientation.
   * Value must be in range of (-half of device vertical field of view, half of
   * device vertical field of view) where positive value represent rotation of
   * user gaze up and negative value correspond to the gaze down
   * (from user PoV).
   */
  float right_eye_y;
  GazeEvent gaze_event;
} EyeTrackerDataStruct;

struct SharedMemoryEyeTrackerReader;
/**
  * @brief Initializes eye tracker reader and writes memory location of
  * SharedMemoryEyeTrackerReader to dereference pointer_address.
  * @returns Initialization status. Pointer at input address is only populated
  * when function returns kSuccess.
  */
LIB_EXPORT InitializationStatus CALL_CONV
CreateEyeTrackerReader(SharedMemoryEyeTrackerReader** pointer_address);
/**
  * @brief Frees all resources allocated during call to CreateEyeTrackerReader
  * and zeroes pointer.
  * @param pointer_address address of pointer to memory allocated with
  * CreateEyeTrackerReader
  */
LIB_EXPORT void CALL_CONV
DestroyEyeTrackerReader(SharedMemoryEyeTrackerReader** pointer_address);

/**
 * @brief Moves internal pointer to latest sample.
 * Then if new data is available the data is read and copied to input param.
 * @param out_data output struct that will be changed on successful read.
 * @return true when data was successfully read, otherwise false
 */
LIB_EXPORT bool CALL_CONV
TryReadNextEyeTrackerData(SharedMemoryEyeTrackerReader*, EyeTrackerDataStruct*);

/**
 * @brief Checks if there is new data available since last read.
 * Then if new data is available advances internal pointer by one, reads the
 * and the copies to out_data.
 * @param out_data output struct that will be changed on successful read.
 * @return true when data was successfully read, otherwise false
 */
LIB_EXPORT bool CALL_CONV TryReadLatestEyeTrackerData(
    SharedMemoryEyeTrackerReader*, EyeTrackerDataStruct*);
}
#undef CALL_CONV
#undef LIB_EXPORT
#endif  // REMOTE_CONNECTOR_H_