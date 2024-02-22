# Changelog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

### Added

- new sample project in `c` programming language 

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