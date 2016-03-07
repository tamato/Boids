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
list( APPEND CMAKE_CXX_FLAGS "-std=c++11")

################################################################################
# Set up the type of build to use. i.e. Debug or Release?
if (CMAKE_BUILD_TYPE STREQUAL "")
  # CMake defaults to leaving CMAKE_BUILD_TYPE empty. This messes up
  # differentiation between debug and release builds.               
  set (CMAKE_BUILD_TYPE "Debug" CACHE STRING "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel." FORCE)
endif ()

###########################################
# Add GLAD
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/glad)
set(GLAD_SOURCE ${CMAKE_CURRENT_SOURCE_DIR}/external/glad/glad.c)

###########################################
# Add GLFW, and have it built with the project
set(BUILD_SHARED_LIBS OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE STRING "" FORCE)
set(GLFW_INSTALL OFF CACHE STRING "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE STRING "" FORCE)
add_subdirectory(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include)
# link_directories(${CMAKE_CURRENT_SOURCE_DIR}/build/external/glfw/src)

###########################################
# Add GLM
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external/glm)

################################################################################
# The core project files
file (GLOB SRCS *.cpp )
file (GLOB COMMON_SOURCE common/*.cpp)

add_executable( ${PROJECT_NAME} ${SRCS} ${COMMON_SOURCE} ${GLAD_SOURCE})
target_link_libraries( ${PROJECT_NAME} glfw)

