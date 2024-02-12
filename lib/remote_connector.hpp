//
// Created by Mateusz on 06.02.2024.
//
#ifndef REMOTE_CONNECTOR_HPP
#define REMOTE_CONNECTOR_HPP

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

#include <string>
#include <cstdint>
#include <functional>
#include <memory>
#include <iostream>

extern "C++" {
namespace inseye {

typedef struct Version {
  constexpr Version(const uint32_t major, const uint32_t minor,
                    const uint32_t patch) noexcept
      : major(major), minor(minor), patch(patch) {}

  Version(const Version&) noexcept;
  Version(const Version&&) noexcept;

  Version& operator=(Version&&) noexcept;

  bool operator==(const Version&) const;

  bool operator!=(const Version&) const;

  bool operator<(const Version&) const;

  bool operator>(const Version&) const;

  bool operator>=(const Version&) const;

  bool operator<=(const Version&) const;
  [[nodiscard]] uint32_t GetMajor() const noexcept;
  [[nodiscard]] uint32_t GetMinor() const noexcept;
  [[nodiscard]] uint32_t GetPatch() const noexcept;

 private:
  uint32_t major;
  uint32_t minor;
  uint32_t patch;
  friend std::ostream& operator<<(std::ostream& os, Version const& p);
} Version;

inline std::ostream& operator<<(std::ostream& os, Version const& p) {
  os << (long)p.GetMajor() << "." << (long)p.GetMinor() << "."
     << (long)p.GetPatch();
  return os;
}

class CombinedException final : std::runtime_error {
  const uint32_t status_;

 public:
  explicit CombinedException(const std::string& message, uint32_t status)
      : runtime_error(message), status_(status) {}

  explicit CombinedException(const char* message, uint32_t status)
      : runtime_error(message), status_(status) {}
  const char* what() { return std::runtime_error::what(); }
  /**
   * @brief Error status code.
   * @return integral value that corresponds to Initialization status from "remote_connector.h"
   */
  [[nodiscard]] const uint32_t& GetStatusCode() const { return status_; }
};

extern const Version lowest_supported_service_version;
extern const Version highest_supported_service_version;

enum class GazeEvent : uint32_t {
  /**
   * Nothing particular happened
   */
  None = 0,
  /**
   * Left eye is closed or blinked
   */
  BlinkLeft = 1 << 0,
  /**
   * Right eye is closed or blinked
   */
  BlinkRight = 1 << 1,
  /**
   * Both eye are closed or both eye performed blink
   */
  BlinkBoth = 1 << 2,
  /**
   * Saccade occurred
   */
  Saccade = 1 << 3,
  /**
   * Headset was put on by the user
   */
  HeadsetMount = 1 << 4,
  /**
   * Headset was put off by the user
   */
  HeadsetDismount = 1 << 5,
  /**
   * Unknown event that was introduced in later version of service
   */
  Unknown = HeadsetDismount << 1
};

inline std::ostream& operator<<(std::ostream& os, GazeEvent event) {
  switch (event) {
    case GazeEvent::None:
      os << "None";
      break;
    case GazeEvent::BlinkLeft:
      os << "Blink Left";
      break;
    case GazeEvent::BlinkRight:
      os << "Blink Right";
      break;
    case GazeEvent::BlinkBoth:
      os << "Blink Both";
      break;
    case GazeEvent::Saccade:
      os << "Saccade";
      break;
    case GazeEvent::HeadsetMount:
      os << "HeadsetMount";
      break;
    case GazeEvent::HeadsetDismount:
      os << "HeadsetDismount";
      break;
    case GazeEvent::Unknown:
      os << "Unknown";
      break;
    default:
      os << "Unknown (Invalid value of: " << (uint32_t)event << ")";
  }
  return os;
}

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

struct SharedMemoryEyeTrackerReaderImplementation;

class LIB_EXPORT SharedMemoryEyeTrackerReader final {
  std::unique_ptr<
      SharedMemoryEyeTrackerReaderImplementation,
      std::function<void(SharedMemoryEyeTrackerReaderImplementation*)>>
      implementatation_pointer_;

 public:
  /**
  * @brief Initializes eye tracker reader.
  * @exception inseye::CombinedException thrown when initialization fails.
  */
  SharedMemoryEyeTrackerReader();

  SharedMemoryEyeTrackerReader(SharedMemoryEyeTrackerReader&) = delete;

  SharedMemoryEyeTrackerReader(SharedMemoryEyeTrackerReader&&) noexcept;
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
};
}  // namespace inseye
}
#undef LIB_EXPORT
#endif  // REMOTE_CONNECTOR_HPP