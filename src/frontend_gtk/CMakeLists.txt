cmake_minimum_required(VERSION 2.8.12)

if(CMAKE_CONFIGURATION_TYPES)
	set(CMAKE_CONFIGURATION_TYPES "Debug;Release" CACHE STRING "My multi config types" FORCE)
elseif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
	set(CMAKE_BUILD_TYPE RELEASE)
endif()

set(CMAKE_C_FLAGS, "-Wall")
set(CMAKE_CXX_FLAGS, "-Wall")
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

project(md5_gtk)

find_package (PkgConfig REQUIRED)
pkg_check_modules (GTK3 REQUIRED gtk+-3.0)

## Include headers
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/../)
include_directories (${GTK3_INCLUDE_DIRS})

## md5_gtk 
set(MD5_GTK_SRCS "${CMAKE_CURRENT_SOURCE_DIR}/../md5.c" "${CMAKE_CURRENT_SOURCE_DIR}/../md5.h" "${CMAKE_CURRENT_SOURCE_DIR}/main.c")
add_executable(${PROJECT_NAME} ${MD5_GTK_SRCS})

add_definitions (${GTK3_CFLAGS_OTHER})
target_link_libraries(${PROJECT_NAME} ${GTK3_LIBRARIES})

# Install
if(UNIX)
	set(CMAKE_INSTALL_PREFIX /usr/local)
	install(TARGETS ${PROJECT_NAME} DESTINATION bin)
elseif(WIN32)
	set(CMAKE_INSTALL_PREFIX "C:/Program Files (x86)/")
	install(TARGETS ${PROJECT_NAME} DESTINATION MD5_gtk)
endif()

