set(SOURCES
        remote_connector.cpp
        remote_connector.h
        endianess_helpers.hpp
        shared_memory_header.hpp
        shared_memory_header.cpp
        version.hpp
        eye_tracker_data_struct.hpp
        named_pipe_communicator.cpp
        named_pipe_communicator.hpp
)
add_library(inseye_remote_connector_lib SHARED ${SOURCES})
if(MSVC)
    target_compile_options(inseye_remote_connector_lib PRIVATE /W4 /WX)
else()
    target_compile_options(inseye_remote_connector_lib PRIVATE -Wall -Wextra -Wpedantic -Werror)
endif ()

include(GenerateExportHeader)

target_include_directories(inseye_remote_connector_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
set_target_properties(inseye_remote_connector_lib PROPERTIES PUBLIC_HEADER remote_connector.h)
install(TARGETS inseye_remote_connector_lib PUBLIC_HEADER DESTINATION include)
