# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [0.1.0] - 2024-04-30

### Added

- new functions in api that read last eye tracking data without modifying internal iterator
  + `TryReadLastEyeTrackerData` for `c`
  + `inseye::EyeTracker::TryReadLastEyeTrackerData` for `c++`

- new functions in api that query if gaze data can be read
  + `IsGazeDataAvailable` for `c`
  + `inseye::EyeTracker::IsGazeDataAvailable` for `c++`

- new function `GetLastErrorDescripton` that allows retrieving error description when library fail to create eye tracker 

- new function in api to query if desktop service is available `IsServiceAvailable`

- new sample project in `c` programming language

### Changed

- warnings and pedantic treated as compilation errors

### Removed

- removed `remote_connector.hpp`, all `c++` declarations were moved to [remote_connector.h](./lib/remote_connector.h) 

### Fixed

- the library can be now actually linked into c program (was not possible before)



## [0.0.1] - 2024-02-12

### Added

- first iteration of library used to communicate with Windows Service
- windows library compiled to dynamic library
- library allows to connect to Windows service and obtain gaze data
- support for service semantic versioning
- [C](./lib/remote_connector.h) and [C++](./lib/remote_connector.hpp) headers for new library
- library builds with both MingGW (GCC) and Visual C++
- sample project used to test library