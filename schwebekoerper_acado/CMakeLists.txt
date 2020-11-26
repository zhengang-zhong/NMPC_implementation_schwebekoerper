#
# Project settings
#

# Minimum required version of cmake
CMAKE_MINIMUM_REQUIRED( VERSION 2.8 )

# Project name and programming languages used
PROJECT( MY_COOL_PROJECT CXX )

if(UNIX)
		SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  -std=c++0x")	
endif()


# CMake module(s) path
SET( CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${PROJECT_SOURCE_DIR} )

#
# Prerequisites
#
FIND_PACKAGE( ACADO REQUIRED )

#
# Include directories
#
INCLUDE_DIRECTORIES( . ${ACADO_INCLUDE_DIRS} )

#
# Build an executable
#
ADD_EXECUTABLE(        schwebekoerper schwebekoerper.cpp )
# Linux and OS X
TARGET_LINK_LIBRARIES( schwebekoerper ${ACADO_SHARED_LIBRARIES} )
# Windows
#TARGET_LINK_LIBRARIES( double_integrator ${ACADO_STATIC_LIBRARIES} )
SET_TARGET_PROPERTIES( schwebekoerper
                       PROPERTIES RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )
