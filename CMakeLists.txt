cmake_minimum_required(VERSION 3.16)
project(SmartHomeNetwork VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Platform-specific settings
if(WIN32)
    set(PLATFORM_LIBS ws2_32)
else()
    set(PLATFORM_LIBS pthread)
endif()

# Include directories
include_directories(${PROJECT_SOURCE_DIR}/include)

# --- Sources that are implemented ---
set(SERVER_SOURCES
    src/server/Server.cpp
    src/utils/SocketSystem.cpp
    src/protocol/HttpProtocol.cpp
)

# --- Server executable ---
add_executable(server
    src/server/main_server.cpp
    ${SERVER_SOURCES}
)
target_link_libraries(server ${PLATFORM_LIBS})

# TODO: Uncomment as modules are implemented
# set(DEVICE_SOURCES
#     src/devices/Device.cpp
#     src/devices/Light.cpp
#     src/devices/Thermostat.cpp
#     src/devices/SecurityCamera.cpp
# )
# set(NETWORK_SOURCES
#     src/network/SubnetManager.cpp
#     src/network/RoutingTable.cpp
#     src/network/ARPTable.cpp
#     src/network/NetworkConfig.cpp
# )
# add_executable(client src/client/main_client.cpp ...)
# enable_testing()
# add_executable(test_server tests/test_server.cpp ...)