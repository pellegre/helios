# MC++ CMake file
# Esteban Pellegrino

# ---- Project stuff

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)
project(Helios++)

# ---- Version Information

set( ${PROJECT_NAME}_MAJOR_VERSION 0 ) 
set( ${PROJECT_NAME}_MINOR_VERSION 1 ) 

# ---- Compiler settings for different builds

set(CMAKE_AR xiar)
set(CMAKE_CXX_FLAGS_RELEASE "-xHOST -O3 -ipo -no-prec-div -static-intel") 
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -Wall")

if(CMAKE_BUILD_TYPE STREQUAL "release")
  set(COMPILER_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
else(CMAKE_BUILD_TYPE STREQUAL "debug")
  set(COMPILER_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
endif()

# ---- Configuration stuff

list(APPEND CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/CMake/")

# Version
set(PROJECT_VERSION "${${PROJECT_NAME}_MAJOR_VERSION}.${${PROJECT_NAME}_MINOR_VERSION}")

# Git stuff
include(GetGitRevisionDescription)
get_git_head_revision(GIT_REFSPEC GIT_SHA1)

# Compilation date
macro (TODAY RESULT)
  if (WIN32)
    execute_process(COMMAND "date" "/T" OUTPUT_VARIABLE ${RESULT})
    string(REGEX REPLACE "(..)/(..)/(....).*" "\\3/\\1/\\2" ${RESULT} ${${RESULT}})
  elseif (UNIX)
    execute_process(COMMAND "date" "+%m/%d/%Y-%r" OUTPUT_VARIABLE ${RESULT})
    string(REGEX REPLACE "(..)/(..)/(....)-(...........).*" "\\1/\\2/\\3 - \\4" ${RESULT} ${${RESULT}})
  else ()
    message(SEND_ERROR "date not implemented")
    set (${RESULT} 000000)
  endif ()
endmacro ()
today(COMPILATION_DATE)

CONFIGURE_FILE(${CMAKE_SOURCE_DIR}/CMake/Config.hpp.cmake ${CMAKE_SOURCE_DIR}/Common/Config.hpp)

# ---- External dependencies

FIND_PATH(BLITZ_INCLUDE_DIR blitz/blitz.h)
FIND_LIBRARY(BLITZ_LIBRARY NAMES blitz )

IF(BLITZ_FOUND)
  SET( BLITZ_LIBRARIES ${BLITZ_LIBRARY} )
ENDIF(BLITZ_FOUND)

MARK_AS_ADVANCED(BLITZ_INCLUDE_DIR BLITZ_LIBRARY)

FIND_LIBRARY(TRNG_LIBRARY NAMES trng4 )

IF(TRNG_FOUND)
  SET( TRNG_LIBRARIES ${TRNG_LIBRARY} )
ENDIF(TRNG_FOUND)

MARK_AS_ADVANCED(TRNG_INCLUDE_DIR TRNG_LIBRARY)

# Intel TBB
find_package(TBB REQUIRED)
# Here goes the actual project
include_directories(${TBB_INCLUDE_DIRS})
link_directories(${TBB_LIBRARY_DIRS})

# Boost
find_package( Boost REQUIRED COMPONENTS program_options )
include_directories(${Boost_INCLUDE_DIRS})
link_directories(${Boost_LIBRARIES_DIRS})

# ---- Monte Carlo library

set(MCFILES Common/Common.cpp
            Common/Log/Log.cpp
            Transport/Particle.cpp
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
            Parser/XMLParser/XmlParserMaterial.cpp                       
            )
            
add_library(mc++ ${MCFILES})

target_link_libraries (mc++ ${BLITZ_LIBRARY} ${TRNG_LIBRARY})

# ---- Unit Test

add_subdirectory (DevUtils/Gtest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(testmc++ DevUtils/Utils.cpp DevUtils/Testing/Main.cpp)
               
target_link_libraries(testmc++ mc++ ${Blitz_LIBRARY} ${TRNG_LIBRARY} gtest gtest_main)
set(cxx_use_own_tuple "${CMAKE_CXX_FLAGS} -DGTEST_USE_OWN_TR1_TUPLE=1")
set_target_properties (testmc++ PROPERTIES COMPILE_FLAGS ${cxx_use_own_tuple})

# ---- Simple plotter

set(PNGWRITER DevUtils/PngPlotter/pngwriter.cpp)
set_source_files_properties(${PNGWRITER} PROPERTIES COMPILE_DEFINITIONS NO_FREETYPE)
set_source_files_properties(DevUtils/PngPlotter/Main.cpp PROPERTIES COMPILE_DEFINITIONS NO_FREETYPE)
set_source_files_properties(DevUtils/PngPlotter/PngPlotter.cpp PROPERTIES COMPILE_DEFINITIONS NO_FREETYPE)

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIR})

add_executable(plottermc++ ${PNGWRITER} 
                           DevUtils/PngPlotter/Main.cpp
                           DevUtils/PngPlotter/PngPlotter.cpp
                           )

target_link_libraries(plottermc++ mc++ ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Blitz_LIBRARY} ${PNG_LIBRARY} ${TRNG_LIBRARY} ${TBB_LIBRARIES})

# ---- Helios

add_executable(helios++ Main.cpp)
               
target_link_libraries(helios++ mc++ ${Blitz_LIBRARY} ${TRNG_LIBRARY})

# ---- Instal stuff

install(TARGETS plottermc++ RUNTIME DESTINATION bin)
install(TARGETS helios++ RUNTIME DESTINATION bin)

message(STATUS "${PROJECT_NAME} version  ${${PROJECT_NAME}_VERSION}")
message(STATUS "Build type:         ${CMAKE_BUILD_TYPE}")
