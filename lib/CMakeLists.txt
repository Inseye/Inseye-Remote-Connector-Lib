set(SOURCES
        remote_connector.cpp
        remote_connector.hpp
        remote_connector.h
        endianess_helpers.hpp
)
add_library(Remote_Connector_Dlib SHARED ${SOURCES})

include(GenerateExportHeader)
generate_export_header(Remote_Connector_Dlib)

target_include_directories(Remote_Connector_Dlib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

install(TARGETS Remote_Connector_Dlib DESTINATION Remote_Connector_Dlib-bin)
install(FILES remote_connector.h remote_connector.hpp DESTINATION Remote_Connector_Dlib-include)
