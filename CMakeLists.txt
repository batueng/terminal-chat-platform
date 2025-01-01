cmake_minimum_required(VERSION 3.10)

# Project name
project(TerminalChatPlatform)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug)
endif()

# Add debug flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")  # Enable debug symbols and disable optimizations

# Find Boost
find_package(Boost REQUIRED COMPONENTS thread system)

# Include directories
include_directories(${Boost_INCLUDE_DIRS})

# Add executable for the server
add_executable(server
    MessageServer.cpp
    Session.cpp
    UserSocket.cpp
    ServerSocket.cpp
    ResponseHandler.cpp
    protocol.h
    errors.h
)

# Link Boost libraries to the server executable
target_link_libraries(server ${Boost_LIBRARIES})

# Add executable for the client
add_executable(tcp
    Client.cpp
    graphics.cpp
    ClientSocket.cpp
    RequestHandler.cpp
    protocol.h
)

# Link Boost libraries to the client executable
target_link_libraries(tcp ${Boost_LIBRARIES})
