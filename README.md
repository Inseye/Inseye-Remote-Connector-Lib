# About

This repository contains source code of a library that connects
to [Inseye-Remote-Connector-Desktop](https://github.com/Inseye/Inseye-Remote-Connector-Desktop) and serves inseye gaze
data to any application.

# License

This repository is part of Inseye Software Development Kit.

By using content of this repository you agree SDK [License](LICENSE)

# Documentation

The project is build using `cmake` and is split into three separate targets:
- `lib`, main build target building the library, stored in [lib](./lib) directory
- `sample_c`, an example of use in `c` programming language, stored in [sample_c](./sample_c)
- `sample_cpp`, an example of use in `cpp` programming language, stored in [sample_cpp](./sample_cpp)

## Building the project

Example showing hot to build `sample cpp` (using cmake delivered with Microsoft Visual C++ toolchains).

`cmake -DCMAKE_BUILD_TYPE=Release -S ./ -B ./cmake-b-release`

then:

` cmake --build .\cmake-b-release\ --target sample_cpp`

## The library

The library builds into single .dll/.lib file. There is single header required to include - [remote_connector.h](./lib/remote_connector.h).
The library connects to [Inseye-Remote-Connector-Desktop](https://github.com/Inseye/Inseye-Remote-Connector-Desktop) running on the same machine and consumes gaze data from it.

