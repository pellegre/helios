# MC++ CMake file
# Esteban Pellegrino

# ---- Project stuff

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)
project(MC++)

# ---- Version Information

set( ${PROJECT_NAME}_MAJOR_VERSION 0 ) 
set( ${PROJECT_NAME}_MINOR_VERSION 1 ) 

# ---- Compiler settings for different builds
set(CMAKE_AR xiar)
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -xHOST -O3 -ipo -no-prec-div -static-intel") 
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall")

# ---- External dependencies

FIND_PATH(BLITZ_INCLUDE_DIR blitz/blitz.h)
FIND_LIBRARY(BLITZ_LIBRARY NAMES blitz )

IF(BLITZ_FOUND)
  SET( BLITZ_LIBRARIES ${BLITZ_LIBRARY} )
ENDIF(BLITZ_FOUND)

MARK_AS_ADVANCED(BLITZ_INCLUDE_DIR BLITZ_LIBRARY)

# ---- Monte Carlo library

set(MCFILES Common/Common.cpp
            Log/Log.cpp
            Geometry/Geometry.cpp
            Geometry/Universe.cpp               
            Geometry/Cell.cpp
            Geometry/Surface.cpp
            Geometry/GeometricFeature.cpp
            Geometry/Surfaces/CylinderOnAxis.cpp    
            Geometry/Surfaces/CylinderOnAxisOrigin.cpp
            Material/Material.cpp
            Material/MaterialContainer.cpp
            Material/MacroXs.cpp                                                        
            Parser/Parser.cpp
            Parser/XMLParser/tinystr.cpp
            Parser/XMLParser/tinyxml.cpp                  
            Parser/XMLParser/tinyxmlerror.cpp                  
            Parser/XMLParser/tinyxmlparser.cpp  
            Parser/XMLParser/XmlParser.cpp    
            Parser/XMLParser/XmlParserGeometry.cpp                
            )
            
add_library(mc++ ${MCFILES})

target_link_libraries (mc++ ${BLITZ_LIBRARY})

# ---- Unit Test

add_subdirectory (DevUtils/Gtest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(testmc++ DevUtils/Utils.cpp DevUtils/Testing/Main.cpp)
               
target_link_libraries(testmc++ mc++ ${Blitz_LIBRARY} ${PNG_LIBRARY} gtest gtest_main)
set(cxx_use_own_tuple "${CMAKE_CXX_FLAGS} -DGTEST_USE_OWN_TR1_TUPLE=1")
set_target_properties (testmc++ PROPERTIES COMPILE_FLAGS ${cxx_use_own_tuple})

# ---- Simple plotter

set(PNGWRITER DevUtils/PngPlotter/pngwriter.cpp)
set_source_files_properties(${PNGWRITER} PROPERTIES COMPILE_DEFINITIONS NO_FREETYPE)
set_source_files_properties(DevUtils/PngPlotter/Main.cpp PROPERTIES COMPILE_DEFINITIONS NO_FREETYPE)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

add_executable(plottermc++ ${PNGWRITER} DevUtils/PngPlotter/Main.cpp)
               
target_link_libraries(plottermc++ mc++ ${Blitz_LIBRARY} ${PNG_LIBRARY})

# ---- Instal stuff

install(TARGETS plottermc++ RUNTIME DESTINATION bin)

message(STATUS "${PROJECT_NAME} version  ${${PROJECT_NAME}_VERSION}")
message(STATUS "Build type:         ${CMAKE_BUILD_TYPE}")
