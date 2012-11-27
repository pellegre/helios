# MC++ CMake file
# Esteban Pellegrino

# ---- Project stuff

cmake_minimum_required(VERSION 2.6.0 FATAL_ERROR)
project(Helios++)

# ---- Version Information

set( ${PROJECT_NAME}_MAJOR_VERSION 0 ) 
set( ${PROJECT_NAME}_MINOR_VERSION 1 ) 

# ---- Compiler settings for different builds

# Include root directory
include_directories(".")

if ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "GNU")
	message("-- Setting flags for gnu/g++ compiler")
	# Using GNU compiler
	if(CMAKE_BUILD_TYPE STREQUAL "release")
	  set(CMAKE_CXX_FLAGS_RELEASE "-ansi -Wall -fopenmp -O3")
	  set(COMPILER_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
	else(CMAKE_BUILD_TYPE STREQUAL "debug")
	  set(CMAKE_CXX_FLAGS_DEBUG "-g -ansi -Wall -fopenmp -O1")
	  set(COMPILER_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
	endif()
elseif ("${CMAKE_CXX_COMPILER_ID}" STREQUAL "Intel")
	message("-- Setting flags for intel compiler")
	# Using intel compiler
	if(CMAKE_BUILD_TYPE STREQUAL "release")
	  set(CMAKE_AR xiar)
	  set(CMAKE_CXX_FLAGS_RELEASE "-ansi -openmp -xHOST -O3 -ipo -no-prec-div -static-intel") 
	  set(COMPILER_FLAGS ${CMAKE_CXX_FLAGS_RELEASE})
	else(CMAKE_BUILD_TYPE STREQUAL "debug")
	  set(CMAKE_CXX_FLAGS_DEBUG "-ansi -g -O1 -Wall -openmp")
	  set(COMPILER_FLAGS ${CMAKE_CXX_FLAGS_DEBUG})
	endif()
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
            Environment/McEnvironment.cpp   
            Environment/McModule.cpp
            Environment/Simulation.cpp
            Environment/Settings/Settings.cpp  
            Transport/Particle.cpp
            Transport/Distribution/Distribution.cpp
            Transport/ParticleSource.cpp                       
            Transport/Source.cpp           
            Geometry/Geometry.cpp
            Geometry/Universe.cpp               
            Geometry/Cell.cpp
            Geometry/GeometricFeature.cpp
            Geometry/Surface.cpp
            Geometry/Surfaces/CylinderOnAxis.cpp    
            Geometry/Surfaces/CylinderOnAxisOrigin.cpp
            Geometry/Surfaces/SphereOnOrigin.cpp
            Material/Material.cpp
            Material/Materials.cpp
            Material/Isotope.cpp            
            Material/MacroXs/MacroXs.cpp  
            Material/Grid/MasterGrid.cpp         
            Material/AceTable/AceMaterial.cpp  
            Material/AceTable/AceModule.cpp
            Material/AceTable/AceReaction/MuSampler.cpp   
            Material/AceTable/AceReaction/AceReactionBase.cpp                                         
            Material/AceTable/AceReaction/InelasticScattering.cpp
            Material/AceTable/AceReaction/FissionReaction.cpp 
            Material/AceTable/AceReaction/EnergySampler.cpp
            Material/AceTable/AceReaction/EnergyLaws/AceEnergyLaw.cpp 
            Material/AceTable/AceReaction/EnergyLaws/EnergyLaw1.cpp                                                                           
            Material/AceTable/AceReaction/EnergyLaws/EnergyLaw4.cpp
            Material/AceTable/AceReaction/EnergyLaws/EnergyLaw7.cpp
            Material/AceTable/AceReaction/EnergyLaws/EnergyLaw9.cpp 
            Material/AceTable/AceReaction/EnergyLaws/EnergyLaw11.cpp                                   
            Material/AceTable/AceReaction/EnergyLaws/EnergyLaw44.cpp
            Material/AceTable/AceReaction/EnergyLaws/EnergyLaw61.cpp                                                                                                    
			Material/AceTable/AceReader/ACETable.cpp
			Material/AceTable/AceReader/ACEReader.cpp
			Material/AceTable/AceReader/CrossSection.cpp
			Material/AceTable/AceReader/Blocks/NUBlock.cpp
			Material/AceTable/AceReader/Blocks/DLWBlock.cpp
			Material/AceTable/AceReader/Blocks/ANDBlock.cpp
			Material/AceTable/AceReader/Blocks/ACEBlock.cpp
			Material/AceTable/AceReader/Blocks/ESZBlock.cpp
			Material/AceTable/AceReader/Blocks/NRBlock.cpp
			Material/AceTable/AceReader/NeutronReaction.cpp
			Material/AceTable/AceReader/AceUtils.cpp
			Material/AceTable/AceReader/Conf.cpp
			Material/AceTable/AceReader/NeutronTable.cpp
			Material/AceTable/AceReader/TyrDistribution.cpp
			Material/AceTable/AceReader/ReactionContainer.cpp
			Material/AceTable/AceReader/PrintMessage.cpp
			Material/AceTable/AceReader/EnergyDistribution.cpp
			Material/AceTable/AceReader/AngularDistribution.cpp 
			Tallies/Histogram.cpp
			Tallies/Tally.cpp			                                                                             
            Parser/Parser.cpp
            Parser/XMLParser/tinystr.cpp
            Parser/XMLParser/tinyxml.cpp                  
            Parser/XMLParser/tinyxmlerror.cpp                  
            Parser/XMLParser/tinyxmlparser.cpp
            Parser/XMLParser/ticpp.cpp  
            Parser/XMLParser/tinyxmlparser.cpp                
            Parser/XMLParser/XmlParser.cpp    
            Parser/XMLParser/XmlParserGeometry.cpp
            Parser/XMLParser/XmlParserMaterial.cpp 
            Parser/XMLParser/XmlParserSource.cpp                                        
            )
            
add_library(helios ${MCFILES})

target_link_libraries (helios ${BLITZ_LIBRARY} ${TRNG_LIBRARY})

# ---- Unit Test

add_subdirectory (DevUtils/Gtest)
enable_testing()
include_directories(${gtest_SOURCE_DIR}/include ${gtest_SOURCE_DIR})

add_executable(testmc++ DevUtils/Utils.cpp DevUtils/Testing/Main.cpp)
               
target_link_libraries(testmc++ helios ${Blitz_LIBRARY} ${TRNG_LIBRARY} gtest gtest_main)
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

target_link_libraries(plottermc++ helios ${Boost_PROGRAM_OPTIONS_LIBRARY} ${Blitz_LIBRARY} ${PNG_LIBRARY} ${TRNG_LIBRARY} ${TBB_LIBRARIES})

# ---- Helios

add_executable(helios++ Main.cpp)
               
target_link_libraries(helios++ helios ${Blitz_LIBRARY} ${TRNG_LIBRARY} ${TBB_LIBRARIES})

# ---- Instal stuff

install(TARGETS plottermc++ RUNTIME DESTINATION bin)
install(TARGETS helios++ RUNTIME DESTINATION bin)

message(STATUS "${PROJECT_NAME} version  ${${PROJECT_NAME}_VERSION}")
message(STATUS "Build type:         ${CMAKE_BUILD_TYPE}")
