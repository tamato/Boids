###############################################################################
#
# A generalized cmake file for developing cross-platform games.
#
cmake_minimum_required (VERSION 2.8 FATAL_ERROR)
#set (CMAKE_VERBOSE_MAKEFILE ON)

# Every project must have a name.
project ("boids")

################################################################################
# Ensures that we do an out of source build
MACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD MSG)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${CMAKE_BINARY_DIR}" insource)
     GET_FILENAME_COMPONENT(PARENTDIR ${CMAKE_SOURCE_DIR} PATH)
     STRING(COMPARE EQUAL "${CMAKE_SOURCE_DIR}"
     "${PARENTDIR}" insourcesubdir)
    IF(insource OR insourcesubdir)
        MESSAGE(FATAL_ERROR "${MSG}")
    ENDIF(insource OR insourcesubdir)
ENDMACRO(MACRO_ENSURE_OUT_OF_SOURCE_BUILD)

MACRO_ENSURE_OUT_OF_SOURCE_BUILD(
    "${CMAKE_PROJECT_NAME} requires an out of source build."
)

################################################################################
# Add in support for c++11
list( APPEND CMAKE_CXX_FLAGS "-std=c++11 -static-libstdc++")

################################################################################
# Set up the type of build to use. i.e. Debug or Release?
if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This messes up
  # differentiation between debug and release builds.               
  set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

###########################################
# Add GLEW
add_definitions(-DGLEW_STATIC)
if (WIN32)
    set(GLEW_LIBRARY glew32s)
else(WIN32)
    ################################
    # Add GLFW, http://www.glfw.org/docs/latest/build.html
    find_package(PkgConfig REQUIRED)
    pkg_search_module(GLFW REQUIRED glfw3)
    include_directories(${GLFW_INCLUDE_DIRS})
    set(GLEW_LIBRARY GLEW)
endif()

################################################################################
# The core project files
file (GLOB SRCS *.cpp )
file (GLOB HDRS *.h )
file (GLOB COMMON_SOURCE common/*.cpp)

link_directories(${CMAKE_CURRENT_SOURCE_DIR}/lib)
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/external
    )

add_executable( ${PROJECT_NAME} ${SRCS} ${COMMON_SOURCE} ${HDRS} )

target_link_libraries( ${PROJECT_NAME} ${GLFW_STATIC_LIBRARIES} ${GLEW_LIBRARY} )
