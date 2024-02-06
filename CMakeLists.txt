cmake_minimum_required(VERSION 3.27)
project(Remote_Connector_Lib)

set(CMAKE_CXX_STANDARD 23)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

add_subdirectory(lib)
add_subdirectory(sample)

# USE_FOLDERS group cmake generated projects into one (CMakePredefinedTargets) folder
set_property(GLOBAL PROPERTY USE_FOLDERS ON)
