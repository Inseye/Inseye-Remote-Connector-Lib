// Last edit: 30.04.24 11:03 by Mateusz Chojnowski mateusz.chojnowski@inseye.com
//
// Copyright (c) Inseye Inc. 2024.
//
// This file is part of Inseye Software Development Kit subject to Inseye SDK License
// See  https://github.com/Inseye/Licenses/blob/master/SDKLicense.txt.
// All other rights reserved.

#ifndef REMOTE_CONNECTOR_H_
#define REMOTE_CONNECTOR_H_

#if !defined(LIB_EXPORT)
#define LIB_EXPORT /* NOTHING */
#if defined(WIN32) || defined(WIN64)
#undef LIB_EXPORT
#if defined(inseye_remote_connector_lib_EXPORTS)
#define LIB_EXPORT __declspec(dllexport)
#else
#define LIB_EXPORT __declspec(dllimport)
#endif  // defined(inseye_remote_connector_lib_EXPORTS)
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

// C only header part
#ifdef __cplusplus
#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include <iostream>
namespace inseye::c {
  extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>

  enum InseyeInitializationStatus {
    kSuccess,
    kFailedToAccessSharedResources,
    kFailedToMapSharedResources,
    kInsFailedToInitializeNamedPipe,
    kInsAllServiceNamedPipesAreBusy,
    kInternalError,
    kServiceVersionToLow,
    kServiceVersionToHigh,
    kCancelled,
    kTimeout,
    kFailure
  };

  enum InseyeGazeEvent { //: uint32_t
    /**
   * Nothing particular happened
   */
    kInsGazeNone = 0,
    /**
   * Left eye is closed or blinked
   */
    kInsGazeBlinkLeft = 1 << 0,
    /**
   * Right eye is closed or blinked
   */
    kInsGazeBlinkRight = 1 << 1,
    /**
   * Both eye are closed or both eye performed blink
   */
    kInsGazeBlinkBoth = 1 << 2,
    /**
   * Saccade occurred
   */
    kInsGazeSaccade = 1 << 3,
    /**
   * Headset was put on by the user
   */
    kInsGazeHeadsetMount = 1 << 4,
    /**
   * Headset was put off by the user
   */
    kInsGazeHeadsetDismount = 1 << 5,
    /**
   * Unknown event that was introduced in later version of service
   */
    kUnknown = kInsGazeHeadsetDismount << 1
  };
  struct InseyeEyeTracker;

  struct InseyeVersion {
    const uint32_t major;
    const uint32_t minor;
    const uint32_t patch;
  };

  extern const LIB_EXPORT struct InseyeVersion kLowestSupportedServiceVersion;
  extern const LIB_EXPORT struct InseyeVersion kHighestSupportedServiceVersion;

  struct InseyeEyeTrackerDataStruct {
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
    enum InseyeGazeEvent gaze_event;
  };

  struct InseyeEyeTracker;

  enum InseyeAsyncOperationState {
    kInsAsyncCreated = 0,
    kInsAsyncRunning = 1,
    kInsAsyncCancelling = 2,
    kInsAsyncCancelled = 3,
    kInsAsyncFaulted = 4,
    kInsCompleted = 5
  };
  /**
   * @brief Checks if desktop service is online and library can make attempt to connect to it.
   * Call to this method doesn't check if the library and service are compatible.
   * @return true if service is running on host machine
   */
  LIB_EXPORT bool CALL_CONV IsServiceAvailable();
  /**
    * @brief Initializes eye tracker reader and writes memory location of
    * SharedMemoryEyeTrackerReader to dereference pointer_address.
    * @param pointer_address address of pointer which will hold information about created
    * shared memory tracker reader memory
    * @param timeout_ms maximum time the function can wait until aborts and returns unsuccessfully
    * @returns Initialization status. Pointer at input address is only populated
    * when function returns kSuccess.
    */
  LIB_EXPORT enum InseyeInitializationStatus CALL_CONV
  CreateEyeTrackerReader(struct InseyeEyeTracker** pointer_address, uint32_t timeout_ms);
  /**
    * @brief Frees all resources allocated during call to CreateEyeTrackerReader
    * and zeroes pointer.
    * @param pointer_address address of pointer to memory allocated with
    * CreateEyeTrackerReader
    */
  LIB_EXPORT void CALL_CONV
  DestroyEyeTrackerReader(struct InseyeEyeTracker** pointer_address);
  /**
   * @brief Checks if there is unread gaze data available in buffer.
   * @return true when unread data is in memory buffer
   */
  LIB_EXPORT bool CALL_CONV
  IsGazeDataAvailable(struct InseyeEyeTracker*);
  /**
   * @brief Moves internal iterator to latest sample.
   * Then if new data is available the data is read and copied to input param.
   * @param out_data output struct that will be changed on successful read.
   * @return true when data was successfully read, otherwise false
   */
  LIB_EXPORT bool CALL_CONV TryReadNextEyeTrackerData(
      struct InseyeEyeTracker*, struct InseyeEyeTrackerDataStruct*);

  /**
   * @brief Checks if there is new data available since last read.
   * Then if new data is available advances internal iterator by one, reads the
   * and the copies to out_data.
   * @param out_data output struct that will be changed on successful read.
   * @return true when data was successfully read, otherwise false
   */
  LIB_EXPORT bool CALL_CONV TryReadLatestEyeTrackerData(
      struct InseyeEyeTracker*, struct InseyeEyeTrackerDataStruct*);
  /**
   * @brief Reads eye tracker data stored at current internal iterator position
   * (previously returned with TryReadNextEyeTrackerData or
   * TryReadLatestEyeTrackerData).
   * This function doesn't change internal iterator position.
   * Multiple calls to this function return the same value until internal
   * iterator is not modified with TryReadNextEyeTrackerData,
   * TryReadCurrentEyeTrackerData or until internal buffer is overwritten with
   * new value.
   * @return true when data was successfully read, otherwise false
   */
  LIB_EXPORT bool CALL_CONV TryReadLastEyeTrackerData(
      struct InseyeEyeTracker*, struct InseyeEyeTrackerDataStruct*);
  /**
   * @brief Returns last error description. It's thread local null terminated
   * ANSI string up to 1024 bytes length.
   * Do not free memory returned from this function.
   * @return Address of first string character.
   */
  LIB_EXPORT char* CALL_CONV GetLastErrorDescription();
#ifdef __cplusplus
  } // namespace inseye::c
} // extern "C"

// CPP header part
namespace inseye {
  using GazeEvent = inseye::c::InseyeGazeEvent;
  using EyeTrackerDataStruct = inseye::c::InseyeEyeTrackerDataStruct;
  struct LIB_EXPORT Version : public inseye::c::InseyeVersion {

    bool operator==(const inseye::Version& other) const;

    bool operator!=(const inseye::Version& other) const;

    bool operator<(const inseye::Version& other) const;

    bool operator>(const inseye::Version& other) const;

    bool operator>=(const inseye::Version& other) const;

    bool operator<=(const inseye::Version& other) const;

    friend std::ostream& operator<<(std::ostream& os, Version const& p);
  };

  extern const struct Version lowestSupportedServiceVersion;
  extern const struct Version highestSupportedServiceVersion;

  std::ostream& operator<<(std::ostream& os, GazeEvent event);

  class LIB_EXPORT EyeTracker final {
   private:
        inseye::c::InseyeEyeTracker*
        implementation_pointer_;

   public:
    EyeTracker() = delete;
    /**
    * @brief Initializes eye tracker reader.
    */
    explicit EyeTracker(int32_t timeout_ms) {
      inseye::c::InseyeEyeTracker* ptr = nullptr;
      if (CreateEyeTrackerReader(&ptr, timeout_ms) != inseye::c::InseyeInitializationStatus::kSuccess) {
        throw std::runtime_error(inseye::c::GetLastErrorDescription());
      }
      implementation_pointer_ = ptr;
    }

    EyeTracker(EyeTracker&) = delete;

    EyeTracker(EyeTracker&&) noexcept;

    ~EyeTracker() noexcept;

    /**
     * @brief Checks if there is unread gaze data available for read.
     * @return true when there is unread gaze data available
     */
    [[nodiscard]] bool IsGazeDataAvailable() const noexcept;
    /**
     * @brief Moves internal pointer to latest sample.
     * Then if new data is available the data is read and copied to input param.
     * @param out_data output struct that will be changed on successful read.
     * @return true when data was successfully read, otherwise false
     */
    bool TryReadLatestEyeTrackerData(EyeTrackerDataStruct& out_data) noexcept;
    /**
     * @brief Checks if there is new data available since last read.
     * Then if new data is available advances internal pointer by one, reads the
     * and the copies to out_data.
     * @param out_data output struct that will be changed on successful read.
     * @return true when data was successfully read, otherwise false
     */
    bool TryReadNextEyeTrackerData(EyeTrackerDataStruct& out_data) noexcept;
    /**
     * @brief Reads eye tracker data stored at current internal iterator position
     * (previously returned with TryReadNextEyeTrackerData or
     * TryReadLatestEyeTrackerData).
     * This function doesn't change internal iterator position.
     * Multiple calls to this function return the same value until internal
     * iterator is not modified with TryReadNextEyeTrackerData,
     * TryReadCurrentEyeTrackerData or until internal buffer is overwritten with
     * new value.
     * @return true when data was successfully read, otherwise false
     */
    bool TryReadLastEyeTrackerData(EyeTrackerDataStruct& out_data) const noexcept;
  };
} // namespace inseye
#undef CALL_CONV
#undef LIB_EXPORT
#endif
#endif  // REMOTE_CONNECTOR_H_