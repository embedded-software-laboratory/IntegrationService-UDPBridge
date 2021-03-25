cmake_minimum_required(VERSION 3.10)
set(CMAKE_CXX_STANDARD 17)

project(is_udp_bridge)

SET(INTEGRATION_SERVICE_INCLUDE_DIRS "/usr/local/include/integration_service")

find_package(fastrtps)

find_package(Boost 1.67.0 EXACT REQUIRED COMPONENTS system)
include_directories(${Boost_INCLUDE_DIRS})
include_directories(${INTEGRATION_SERVICE_INCLUDE_DIRS})
find_package(Threads REQUIRED)

include_directories(./include)

add_definitions(-DEPROSIMA_USER_DLL_EXPORT -DBUILD_SHARED_LIBS)

set(CMAKE_POSITION_INDEPENDENT_CODE ON)
set(BUILD_SHARED_LIBS TRUE)

file(GLOB USER_LIB_SOURCES_CPP "src/udp.cpp")
add_library(is_udp_bridge SHARED ${USER_LIB_SOURCES_CPP})
target_include_directories(is_udp_bridge PUBLIC ${INTEGRATION_SERVICE_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
target_link_libraries(is_udp_bridge Boost::system fastrtps ${CMAKE_DL_LIBS})

