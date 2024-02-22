include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(sample_cpp main.cpp)
# target_link_libraries(Sample Remote_Connector_Lib)
target_link_libraries(sample_cpp
        Remote_Connector_Dlib)