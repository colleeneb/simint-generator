cmake_minimum_required(VERSION 3.0.2)
cmake_policy(SET CMP0048 NEW)  # project_VERSION* variables populated from project(... VERSION x.x.x) string


project(simint
        VERSION 0.8
        LANGUAGES C CXX)

# Maximum AM and derivative level I have available
include(cmake/MaxAvailable.cmake)

##################################################
# Flags passed in
##################################################
set(SIMINT_VECTOR  "" CACHE STRING "Type of vectorization to use")
set(SIMINT_MAXAM  "${SIMINT_MAXAM_AVAILABLE}" CACHE STRING "Maximum AM to use")
set(SIMINT_MAXDER  "${SIMINT_MAXDER_AVAILABLE}" CACHE STRING "Maximum derivative level")

option(SIMINT_STANDALONE "Create as much of a standalone library as possible")
option(ENABLE_TESTS "Enable simint unit tests" ON)
option(ENABLE_FORTRAN "Enable fortran interface" OFF)

##################################################
# Basic options for simint
##################################################
list(APPEND SIMINT_C_INCLUDES "${CMAKE_SOURCE_DIR}")
list(APPEND SIMINT_C_INCLUDES "${CMAKE_BINARY_DIR}/stage") # some CMake-generated files go here

# Default to "Release"
if("${CMAKE_BUILD_TYPE}" STREQUAL "")
  set(CMAKE_BUILD_TYPE "Release")
endif()

# Compile the fortran interface?
if(ENABLE_FORTRAN)
  message(STATUS "Enabling Fortran 2003 interface")
  enable_language(Fortran)
endif()

##################################################
# Read the default flags and generated flags file
##################################################
include(cmake/DefaultFlags.cmake)

message(STATUS "                  Maximum AM: ${SIMINT_MAXAM}")
message(STATUS "          Maximum Derivative: ${SIMINT_MAXDER}")
message(STATUS "               CMAKE_C_FLAGS: ${CMAKE_C_FLAGS}")
message(STATUS "             CMAKE_CXX_FLAGS: ${CMAKE_CXX_FLAGS}")
message(STATUS "           SIMINT_C_INCLUDES: ${SIMINT_C_INCLUDES}")
message(STATUS "              SIMINT_C_FLAGS: ${SIMINT_C_FLAGS}")

if(ENABLE_FORTRAN)
    message(STATUS "        SIMINT_Fortran_FLAGS: ${SIMINT_Fortran_FLAGS}")
endif()

message(STATUS "           SIMINT_LINK_FLAGS: ${SIMINT_LINK_FLAGS}")

# Configure & install the vector_config header
configure_file("simint/vectorization/vector_config.h.in"
               "${CMAKE_BINARY_DIR}/stage/simint/vectorization/vector_config.h"
               @ONLY)

foreach(def ${SIMINT_CONFIG_DEFINES})
  file(APPEND "${CMAKE_BINARY_DIR}/stage/simint/vectorization/vector_config.h"
              "#define ${def}\n")
endforeach()


# Export configuration files
include(GNUInstallDirs)
include(CMakePackageConfigHelpers)

# Install config headers
install(DIRECTORY ${CMAKE_BINARY_DIR}/stage/
        DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/)


set(PN ${PROJECT_NAME})

    # GNUInstallDirs "DATADIR" wrong here; CMake search path wants "share".
set(CMAKECONFIG_INSTALL_DIR "share/cmake/simint")

configure_package_config_file(cmake/simintConfig.cmake.in
                              "${CMAKE_CURRENT_BINARY_DIR}/simintConfig.cmake"
                              INSTALL_DESTINATION ${CMAKECONFIG_INSTALL_DIR})
write_basic_package_version_file(${CMAKE_CURRENT_BINARY_DIR}/simintConfigVersion.cmake
                                 VERSION ${simint_VERSION}
                                 COMPATIBILITY AnyNewerVersion)
install(FILES ${CMAKE_CURRENT_BINARY_DIR}/simintConfig.cmake
              ${CMAKE_CURRENT_BINARY_DIR}/simintConfigVersion.cmake
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})
install(EXPORT "simintTargets"
        NAMESPACE "simint::"
        DESTINATION ${CMAKECONFIG_INSTALL_DIR})

# Main code subdirectory
add_subdirectory(simint)

# Make the test executables
if(ENABLE_TESTS)
    enable_language(CXX)
    add_subdirectory(test)
endif()

# Build examples
add_subdirectory(examples)
