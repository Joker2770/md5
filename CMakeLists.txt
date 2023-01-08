cmake_minimum_required(VERSION 2.9.0)

project(md5)

set(SrcPath "${CMAKE_SOURCE_DIR}/src")
file(GLOB SRCS "${SrcPath}/*.c" "${SrcPath}/*.h")
add_executable(${PROJECT_NAME} ${SRCS})

set(CMAKE_INSTALL_PREFIX /usr/local)
install(TARGETS ${PROJECT_NAME} DESTINATION bin)