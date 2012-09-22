# MC++ CMake file
# Esteban Pellegrino

# ---- Project stuff

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)
project(MC++)

# ---- Version Information

set( ${PROJECT_NAME}_MAJOR_VERSION 0 ) 
set( ${PROJECT_NAME}_MINOR_VERSION 1 ) 

# ---- Compiler settings

set(CMAKE_CXX_FLAGS "-g -O0 -Wall")

# ---- External dependencies

FIND_PATH(Blitz_INCLUDE_DIR blitz/blitz.h)
FIND_LIBRARY(Blitz_LIBRARY NAMES blitz )

IF(BLITZ_FOUND)
  SET( Blitz_LIBRARIES ${Blitz_LIBRARY} )
ENDIF(BLITZ_FOUND)

MARK_AS_ADVANCED(Blitz_INCLUDE_DIR Blitz_LIBRARY)

# ---- Executable

add_executable(mc++ 
               Log/Log.cpp
               Geometry/Cell.cpp
               Geometry/Surface.cpp
               Geometry/Geometry.cpp
               Parser/Parser.cpp
               Parser/XMLParser/tinystr.cpp
               Parser/XMLParser/tinyxml.cpp                  
               Parser/XMLParser/tinyxmlerror.cpp                  
               Parser/XMLParser/tinyxmlparser.cpp  
               Parser/XMLParser/XmlParser.cpp                               
               Main.cpp
               )

target_link_libraries (mc++ ${Blitz_LIBRARY})

# ---- Output

message(STATUS "${PROJECT_NAME} version  ${${PROJECT_NAME}_VERSION}")
message(STATUS "Build type:         ${CMAKE_BUILD_TYPE}")
