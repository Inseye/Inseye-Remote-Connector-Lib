include_directories(${CMAKE_CURRENT_BINARY_DIR})
add_executable(sample_c main.c)
# target_link_libraries(Sample Remote_Connector_Lib)
target_link_libraries(sample_c
        Remote_Connector_Dlib)