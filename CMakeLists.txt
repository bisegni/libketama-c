cmake_policy(VERSION 2.8)
cmake_minimum_required(VERSION 2.6)

cmake_policy(SET CMP0011 NEW)
cmake_policy(SET CMP0042 NEW)

project (ketama)

SET( ${PROJECT_NAME}_MAJOR_VERSION 0 )
SET( ${PROJECT_NAME}_MINOR_VERSION 1 )
SET( ${PROJECT_NAME}_PATCH_LEVEL 0 )

option(BUILD_FORCE_32 "Set to ON to enable 32 bit compilation" OFF)
option(BUILD_FORCE_STATIC "Set static compilation" OFF)

IF( BUILD_FORCE_32 )
MESSAGE(STATUS "Enabling 32 bit Compilation")
set (CMAKE_C_FLAGS "-m32")
set (CMAKE_CXX_FLAGS "-m32")
set (CMAKE_LINK_FLAGS "-m32")
ENDIF()

ADD_DEFINITIONS(-O3 )

IF(NOT BUILD_PREFIX)
set (BUILD_PREFIX usr/local)
ENDIF()

message(STATUS "Using ${CMAKE_INSTALL_PREFIX} as installation directory")

SET(libketama_src   libketama/ketama.c
                    libketama/md5.c)
IF(BUILD_FORCE_STATIC)
MESSAGE(STATUS "Bild static library")
ADD_LIBRARY(ketama STATIC ${libketama_src})
ELSE()
MESSAGE(STATUS "Bild dynamic library")
ADD_LIBRARY(ketama SHARED ${libketama_src})
ENDIF()

INSTALL(FILES libketama/ketama.h DESTINATION include/libketama)

INSTALL(TARGETS ketama
DESTINATION "lib"
PERMISSIONS OWNER_WRITE OWNER_READ GROUP_READ WORLD_READ)
