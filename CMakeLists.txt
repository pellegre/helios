# MC++ CMake file
# Esteban Pellegrino

# ---- Project stuff

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)
project(MC++)

# ---- Version Information

set( ${PROJECT_NAME}_MAJOR_VERSION 0 ) 
set( ${PROJECT_NAME}_MINOR_VERSION 1 ) 

# ---- Compiler settings

set(CMAKE_CXX_FLAGS "-g -O0 -Wall -DGTEST_USE_OWN_TR1_TUPLE=1")

# ---- External dependencies

FIND_PATH(Blitz_INCLUDE_DIR blitz/blitz.h)
FIND_LIBRARY(Blitz_LIBRARY NAMES blitz )

IF(BLITZ_FOUND)
  SET( Blitz_LIBRARIES ${Blitz_LIBRARY} )
ENDIF(BLITZ_FOUND)

MARK_AS_ADVANCED(Blitz_INCLUDE_DIR Blitz_LIBRARY)

# ---- Executable

set(PNGWRITER DevUtils/pngwriter.cpp)
SET_SOURCE_FILES_PROPERTIES(${PNGWRITER} PROPERTIES COMPILE_FLAGS -DNO_FREETYPE)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

set(MCFILES ${PNGWRITER}
            Log/Log.cpp
            DevUtils/Utils.cpp
            Geometry/Geometry.cpp
            Geometry/Universe.cpp               
            Geometry/Cell.cpp
            Geometry/Surface.cpp
            Geometry/Surfaces/CylinderOnAxis.cpp    
            Geometry/Surfaces/CylinderOnAxisOrigin.cpp                                         
            Parser/Parser.cpp
            Parser/XMLParser/tinystr.cpp
            Parser/XMLParser/tinyxml.cpp                  
            Parser/XMLParser/tinyxmlerror.cpp                  
            Parser/XMLParser/tinyxmlparser.cpp  
            Parser/XMLParser/XmlParser.cpp    
            )
            
add_executable(mc++ 
               ${MCFILES}          
               Main.cpp
               )

target_link_libraries (mc++ ${Blitz_LIBRARY} ${PNG_LIBRARY})

# ---- GTest
ADD_SUBDIRECTORY (DevUtils/gtest)
SET_SOURCE_FILES_PROPERTIES(DevUtils/gtest PROPERTIES COMPILE_FLAGS -DGTEST_USE_OWN_TR1_TUPLE=1)

enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

# ---- Unit Tests

add_executable( testmc++ 
                ${MCFILES}
                DevUtils/MainTest.cpp 
               )
               
target_link_libraries(testmc++ ${Blitz_LIBRARY} ${PNG_LIBRARY} gtest gtest_main)

message(STATUS "${PROJECT_NAME} version  ${${PROJECT_NAME}_VERSION}")
message(STATUS "Build type:         ${CMAKE_BUILD_TYPE}")
