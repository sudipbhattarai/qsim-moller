# Setup the project
#
cmake_minimum_required(VERSION 3.21 FATAL_ERROR)
project(QSIM-MOLLER)


#ROOT path
# set(CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/cmake/modules")
find_package(ROOT REQUIRED)
include(${ROOT_USE_FILE})

# this is to ensure we find the header we need
# include_directories(${CMAKE_CURRENT_BINARY_DIR}/include)

#----------------------------------------------------------------------------
# Find Geant4 package, activating all available UI and Vis drivers by default
# You can set WITH_GEANT4_UIVIS to OFF via the command line or ccmake/cmake-gui
# to build a batch mode only executable
#
# option(WITH_GEANT4_UIVIS "Build example with Geant4 UI and Vis drivers" ON)
if(WITH_GEANT4_UIVIS)
  find_package(Geant4 REQUIRED ui_all vis_all)
else()
  find_package(Geant4 REQUIRED)
endif()

#----------------------------------------------------------------------------
# Setup Geant4 include directories and compile definitions
# Setup include directory for this project
#
include(${Geant4_USE_FILE})

#----------------------------------------------------------------------------
#debugging symbols, complains
#
# set(CMAKE_CXX_FLAGS "-g -Wall") # add -w to suppress warnings

#----------------------------------------------------------------------------
# Generate dictionaries as well (so we can write objects to ROOT files)
#
include_directories(${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR}/include ${PROJECT_SOURCE_DIR}/src)
# include_directories(${PROJECT_SOURCE_DIR}/include ${ROOT_INCLUDE_DIR} ${PROJECT_SOURCE_DIR}/src)


#----------------------------------------------------------------------------
# Locate sources and headers for this project
# NB: headers are included so they will show up in IDEs
#
file(GLOB sources ${PROJECT_SOURCE_DIR}/src/*.cc)
file(GLOB headers ${PROJECT_SOURCE_DIR}/include/*.hh)

#----------------------------------------------------------------------------
# Add the executable, and link it to the Geant4 libraries
#
#add_executable(qsim qsim.cc ${sources} ${headers} qsimDict.cxx)
add_executable(qsim qsim.cc ${sources})
target_link_libraries(qsim ${Geant4_LIBRARIES} ${ROOT_LIBRARIES})

#----------------------------------------------------------------------------
# Copy all scripts to the build directory, i.e. the directory in which we
# build qsim. This is so that we can run the executable directly because it
# relies on these scripts being in the current working directory.
#
FILE( GLOB QSIM_GEO "geometry/*" )
set(QSIM_SCRIPTS
    macros/vis.mac
    vis/OIX.mac
    vis/Qt.mac
    vis/vis.mac
    vis/X.mac
    macros/runexample.mac
    macros/gui.mac
)

foreach(_script ${QSIM_SCRIPTS})
  configure_file(
    ${PROJECT_SOURCE_DIR}/${_script}
    ${PROJECT_BINARY_DIR}/${_script}
    COPYONLY
    )
endforeach()

#----------------------------------------------------------------------------
# For internal Geant4 use - but has no effect if you build this
# example standalone
#
add_custom_target(QSIM-MOLLER DEPENDS qsim)

#----------------------------------------------------------------------------
# Install the executable to 'bin' directory under CMAKE_INSTALL_PREFIX
#
# install(TARGETS qsim DESTINATION bin)


