include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(Sample main.cpp)
# target_link_libraries(Sample Remote_Connector_Lib)
target_link_libraries(Sample
        Remote_Connector_Dlib)