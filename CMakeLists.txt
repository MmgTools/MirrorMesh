CMAKE_MINIMUM_REQUIRED(VERSION 2.8.12)

INCLUDE(CMakeDependentOption)
INCLUDE(cmake/modules/macros.cmake)
INCLUDE(CheckCSourceCompiles)

PROJECT (mirrormesh)

# Must use GNUInstallDirs to install libraries into correct
# locations on all platforms.
include(GNUInstallDirs)

###############################################################################
#####
#####         Release version and date
#####
###############################################################################
SET (CMAKE_RELEASE_VERSION_MAJOR "1")
SET (CMAKE_RELEASE_VERSION_MINOR "0")
SET (CMAKE_RELEASE_VERSION_PATCH "0")
SET (CMAKE_RELEASE_DATE "Oct 20, 2021")

SET (CMAKE_RELEASE_VERSION
  "${CMAKE_RELEASE_VERSION_MAJOR}.${CMAKE_RELEASE_VERSION_MINOR}.${CMAKE_RELEASE_VERSION_PATCH}")

###############################################################################
#####
#####         Main CMake compilation variables
#####
###############################################################################

SET(MIRRORMESH_BINARY_DIR      ${PROJECT_BINARY_DIR}/src/mirrormesh)
SET(MIRRORMESH_SHRT_INCLUDE    mirrormesh )
SET(MIRRORMESH_INCLUDE         ${PROJECT_BINARY_DIR}/include/${MIRRORMESH_SHRT_INCLUDE} )

FILE(MAKE_DIRECTORY ${MIRRORMESH_BINARY_DIR})


# Create mirrormeshversion.h file with the release infos
CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/src/mirrormeshversion.h.in
  ${MIRRORMESH_BINARY_DIR}/mirrormeshversion.h @ONLY)

# To see flags and options of compilation
#SET(CMAKE_VERBOSE_MAKEFILE TRUE)

# Executable path
SET(EXECUTABLE_OUTPUT_PATH ${PROJECT_BINARY_DIR}/bin)
SET(MIRRORMESH_SOURCE_DIR ${PROJECT_SOURCE_DIR}/src)
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)


# Find "exotic" compilers
IF (${CMAKE_C_COMPILER} MATCHES Clang OR ${CMAKE_C_COMPILER} MATCHES clang)
  # using clang
  SET(CMAKE_COMPILER_IS_CLANG TRUE)
ELSEIF(${CMAKE_C_COMPILER} MATCHES Icc OR ${CMAKE_C_COMPILER} MATCHES icc)
  # using icc
  SET(CMAKE_COMPILER_IS_INTEL TRUE)
ENDIF()

# Hide some options
MARK_AS_ADVANCED(CMAKE_OSX_ARCHITECTURES
  CMAKE_OSX_DEPLOYMENT_TARGET
  CMAKE_OSX_SYSROOT)

IF(CMAKE_COMPILER_IS_GNUCC)
  IF(APPLE)
    # Add flags to the compiler to work on old mac
    ADD_DEFINITIONS( -mmacosx-version-min=10.4 -arch x86_64)

    # To avoid pbs with binary files...
    SET(CMAKE_EXE_LINKER_FLAGS "-arch x86_64 ${CMAKE_EXE_LINKER_FLAGS} ")

    # Determine if the processor supports 64bit execution
    EXECUTE_PROCESS(
      COMMAND sysctl hw.cpu64bit_capable
      ERROR_QUIET
      OUTPUT_VARIABLE 64_CMD
      OUTPUT_STRIP_TRAILING_WHITESPACE
      )
    STRING(REGEX REPLACE "^hw.cpu64bit_capable: (.*)" "\\1" 64_BIT "${64_CMD}")
    #  ELSEIF(UNIX)# UNIX must be after APPLE becaus APPLE is UNIX too
  ENDIF()
ENDIF()

IF(NOT CMAKE_COMPILER_IS_CLANG)
  # Compiler options for profiling... but not possible with clang
  OPTION ( PROFILING "Enable/Disable PROFILING" OFF )
  IF(PROFILING)
    ADD_DEFINITIONS(-pg)
    SET(CMAKE_EXE_LINKER_FLAGS "-pg ${CMAKE_EXE_LINKER_FLAGS}")
  ENDIF(PROFILING)
ENDIF(NOT CMAKE_COMPILER_IS_CLANG)

###############################################################################
#####
#####         Choose executable target to compile
#####
###############################################################################
IF(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
  MESSAGE(STATUS "Setting build type to 'Release' as none was specified.")
  seT(CMAKE_BUILD_TYPE Release CACHE STRING "Choose the type of build." FORCE)
  # Set the possible values of build type for cmake-gui
  SET_PROPERTY(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release"
    "MinSizeRel" "RelWithDebInfo")
  SET(CMAKE_CONFIGURATION_TYPES ${CMAKE_BUILD_TYPE} )
ENDIF()

OPTION ( LIBMIRRORMESH_STATIC "Compile static library" ON)
OPTION ( LIBMIRRORMESH_SHARED "Compile dynamic library" OFF)

# Explicitly set the DNDEBUG flag in case the user or a parent project overrides
# it.
if (NOT CMAKE_BUILD_TYPE MATCHES Debug)
    add_definitions(-DNDEBUG)
endif()

############################################################################
#####
#####         Scotch
#####
############################################################################
# Find SCOTCH library?
SET(SCOTCH_DIR "" CACHE PATH "Installation directory for scotch")
LIST(APPEND CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake/modules)

# add Scotch library?
OPTION ( USE_SCOTCH "Use SCOTCH TOOL for renumbering" ON )

IF ( USE_SCOTCH )

  FIND_PACKAGE(SCOTCH)

  IF ( NOT SCOTCH_FOUND )
    MESSAGE ( WARNING "Scotch library not found:"
      "Using scotch reduce the execution time of mmg3d "
      "(see https://gforge.inria.fr/frs/?group_id=248 to download it)."
      "If you have already installed Scotch and want to use it, "
      "please set the CMake variable or environment variable SCOTCH_DIR "
      "to your scotch directory.")
  ENDIF ( )

ENDIF ( )


############################################################################
#####
#####         VTK (to parse (p)vtp/(p)vtu files )
#####
############################################################################
OPTION ( USE_VTK "Use VTK I/O" ON )

IF ( USE_VTK )
  FIND_PACKAGE(VTK QUIET)

  IF ( NOT VTK_FOUND )
    MESSAGE ( WARNING "VTK library not found: vtk I/O will not be available.")
  ENDIF ( )
ENDIF ( )

###############################################################################
#####
#####         Add dependent options
#####
###############################################################################

IF( SCOTCH_FOUND )
  SET(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DUSE_SCOTCH")
  MESSAGE(STATUS
    "Compilation with scotch: ${SCOTCH_LIBRARIES}")
  SET( LIBRARIES ${LIBRARIES} ${SCOTCH_LIBRARIES})
ENDIF()


IF ( VTK_FOUND )
  ENABLE_LANGUAGE ( CXX )
  ADD_DEFINITIONS(-DUSE_VTK)
  MESSAGE ( STATUS "Compilation with VTK: add vtp and vtu I/O." )
  INCLUDE ( ${VTK_USE_FILE} )
  SET( LIBRARIES  ${LIBRARIES} "-lstdc++" ${VTK_LIBRARIES} )
ENDIF ( )

############################################################################
#####
#####        MMG (for mesh data structure)
#####
############################################################################
INCLUDE ( ExternalProject )

# forward scotch configuration
SET ( SCOTCH_CFG -DUSE_SCOTCH=${USE_SCOTCH} -DSCOTCH_DIR=${SCOTCH_DIR}
  -DSCOTCH_scotch.h_DIRS=${SCOTCH_scotch.h_DIRS}
  -DSCOTCH_scotch_LIBRARY=${SCOTCH_scotch_LIBRARY}
  -DSCOTCH_scotcherrexit_LIBRARY=${SCOTCH_scotcherrexit_LIBRARY} )

# forward VTK config
SET ( VTK_CFG -DUSE_VTK=${USE_VTK} -DVTK_DIR=${VTK_DIR} )

# forward compiler
SET ( COMPILER_CFG -DCMAKE_CXX_COMPILER=${CMAKE_CXX_COMPILER}
  -DCMAKE_C_COMPILER=${CMAKE_C_COMPILER}
  -DCMAKE_Fortran_COMPILER=${CMAKE_Fortran_COMPILER} )

# forward flags
SET ( FLAGS_CFG -DCMAKE_CXX_FLAGS=${CMAKE_CXX_FLAGS}
  -DCMAKE_C_FLAGS=${CMAKE_C_FLAGS}
  -DCMAKE_Fortran_FLAGS=${CMAKE_Fortran_FLAGS}
  -DCMAKE_EXE_LINKER_FLAGS=${CMAKE_EXE_LINKER_FLAGS} )

OPTION ( DOWNLOAD_MMG "Download and build automatically Mmg." ON )

IF ( DOWNLOAD_MMG )
  UNSET(MMG_DIR CACHE)
  UNSET(MMG_BUILDDIR CACHE)

  # Use pointmap
  OPTION ( USE_POINTMAP "Use map for point tracking" OFF )

 # Mmg
  EXTERNALPROJECT_ADD ( Mmg
    URL ${CMAKE_CURRENT_SOURCE_DIR}/dependencies/Mmg-5.6.0-beta1.zip
    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    INSTALL_COMMAND  echo "Mmg installation DISABLED"
    CMAKE_ARGS ${MMG_ARGS} -DBUILD=MMG3D -DUSE_ELAS=OFF ${COMPILER_CFG} ${FLAGS_CFG}
    ${SCOTCH_CFG} ${VTK_CFG} -DCMAKE_BUILD_TYPE=${CMAKE_BUILD_TYPE}
    -DBUILD=MMG -DBUILD_SHARED_LIBS=${LIBMIRRORMESH_SHARED} -DUSE_POINTMAP=${USE_POINTMAP})

  EXTERNALPROJECT_GET_PROPERTY ( Mmg BINARY_DIR )
  SET ( MMG_BINARY_DIR ${BINARY_DIR} )
  EXTERNALPROJECT_GET_PROPERTY ( Mmg SOURCE_DIR )
  SET ( MMG_SOURCE_DIR ${SOURCE_DIR} )

  EXTERNALPROJECT_ADD_STEP ( Mmg reconfigure
    COMMAND ${CMAKE_COMMAND} -E remove ${MMG_BINARY_DIR}/CMakeCache.txt
    DEPENDEES update
    DEPENDERS configure )

  INCLUDE_DIRECTORIES(${MMG_BINARY_DIR}/include)

  IF( LIBMIRRORMESH_SHARED )
    SET(MMG3D_LIBRARY ${MMG_BINARY_DIR}/lib/libmmg3d${CMAKE_SHARED_LIBRARY_SUFFIX})
  ELSE()
    # default behaviour is to link static libs
    SET(MMG3D_LIBRARY ${MMG_BINARY_DIR}/lib/libmmg3d${CMAKE_STATIC_LIBRARY_SUFFIX})
  ENDIF()
  MESSAGE(STATUS
    "Compilation with Mmg: ${MMG3D_LIBRARY}")
  SET( LIBRARIES ${MMG3D_LIBRARY} ${LIBRARIES})

  # Additionnal directories to access the Mmg sources
  INCLUDE_DIRECTORIES(${MMG_BINARY_DIR}/src/common)
  INCLUDE_DIRECTORIES(${MMG_SOURCE_DIR}/src/mmg3d)
  INCLUDE_DIRECTORIES(${MMG_SOURCE_DIR}/src/common)

ELSE ( )
  UNSET ( USE_POINTMAP CACHE )

  SET(MMG_DIR "" CACHE PATH "Installation directory for mmg")
  SET(MMG_BUILDDIR "" CACHE PATH "Build directory for mmg")

  MESSAGE ( STATUS "Manual installation of Mmg: please, specify the MMG_DIR and MMG_BUILDDIR CMake variables" )

  FIND_PACKAGE(MMG)

  IF(NOT MMG_FOUND )

    MESSAGE ( FATAL_ERROR "ERROR: The installation directory for mmg is required:"
      "(see https://github.com/MmgTools/mmg and  download the branch develop)."
      "If you have already installed Mmg and want to use it, "
      "please set the CMake variable or environment variable MMG_DIR "
      "to your mmg directory and the CMake variable or environment variable"
      " MMG_BUILDDIR "
      "to your mmg build directory.")
  ELSE()

    INCLUDE_DIRECTORIES(${MMG_INCLUDE_DIRS})

    MESSAGE(STATUS
      "Compilation with Mmg: ${MMG_LIBRARIES}")

    SET( LIBRARIES ${MMG_LIBRARIES} ${LIBRARIES})

    # Additionnal directories to access the Mmg sources
    INCLUDE_DIRECTORIES(${MMG_BUILDDIR_INTERNAL}/src/common)
    IF ( MMG_DIR )
      INCLUDE_DIRECTORIES(${MMG_DIR}/src/mmg3d)
      INCLUDE_DIRECTORIES(${MMG_DIR}/src/common)
    ELSE ( )
      MESSAGE ( FATAL_ERROR "ERROR: The source directory for mmg is required:"
        "(see https://github.com/MmgTools/mmg and  download the branch develop)."
        "If you have already installed Mmg and want to use it, "
        "please set the CMake variable or environment variable MMG_DIR "
        "to your mmg directory.")
    ENDIF ( )
  ENDIF ( )
ENDIF()


############################################################################
#####
#####         Fortran header: libmirrormeshf.h
#####
############################################################################

ADD_EXECUTABLE(genheader_mirrormesh ${PROJECT_SOURCE_DIR}/scripts/genheader.c)

GENERATE_FORTRAN_HEADER ( mirrormeshtypes
  ${MIRRORMESH_SOURCE_DIR} libmirrormeshtypes.h
  ${MIRRORMESH_SHRT_INCLUDE}
  ${MIRRORMESH_BINARY_DIR} libmirrormeshtypesf.h
  )

GENERATE_FORTRAN_HEADER ( mirrormesh
  ${MIRRORMESH_SOURCE_DIR} libmirrormesh.h
  ${MIRRORMESH_SHRT_INCLUDE}
  ${MIRRORMESH_BINARY_DIR} libmirrormeshf.h
  )

###############################################################################
#####
#####         Sources and libraries
#####
###############################################################################
# Header files
INCLUDE_DIRECTORIES(${MIRRORMESH_SOURCE_DIR})

# Source files
FILE(
  GLOB
  mirrormesh_library_files
  ${MIRRORMESH_SOURCE_DIR}/*.c
  ${MIRRORMESH_SOURCE_DIR}/*.cpp
  )
LIST(REMOVE_ITEM mirrormesh_library_files
  ${MIRRORMESH_SOURCE_DIR}/mirrormesh.c
  ${REMOVE_FILE})
FILE(
  GLOB
  mirrormesh_main_file
  ${MIRRORMESH_SOURCE_DIR}/mirrormesh.c
  )

IF (NOT WIN32)
  FIND_LIBRARY(M_LIB m)
  SET( LIBRARIES ${LIBRARIES} ${M_LIB})
ENDIF()

############################################################################
#####
##### RPATH for MacOSX
#####
############################################################################

SET(CMAKE_MACOSX_RPATH 1)

############################################################################
#####
#####         Compile mirrormesh libraries
#####
############################################################################
SET(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

IF ( LIBMIRRORMESH_STATIC )
  ADD_AND_INSTALL_LIBRARY ( lib${PROJECT_NAME}_a STATIC
    "${mirrormesh_library_files}" ${PROJECT_NAME} )

  IF ( DOWNLOAD_MMG )
    Add_Dependencies(lib${PROJECT_NAME}_a Mmg)
  ENDIF ( )

  IF ( DOWNLOAD_METIS )
    Add_Dependencies(lib${PROJECT_NAME}_a Metis)
  ENDIF ( )

ENDIF()

# Compile shared library
IF ( LIBMIRRORMESH_SHARED )
  ADD_AND_INSTALL_LIBRARY ( lib${PROJECT_NAME}_so SHARED
    "${mirrormesh_library_files}" ${PROJECT_NAME} )

  IF ( DOWNLOAD_MMG )
    Add_Dependencies(lib${PROJECT_NAME}_so Mmg)
  ENDIF ( )

  IF ( DOWNLOAD_METIS )
    Add_Dependencies(lib${PROJECT_NAME}_so Metis)
  ENDIF ( )

ENDIF()

# mirrormesh header files needed for library
SET( mirrormesh_headers
  ${MIRRORMESH_SOURCE_DIR}/libmirrormesh.h
  ${MIRRORMESH_BINARY_DIR}/libmirrormeshf.h
  ${MIRRORMESH_SOURCE_DIR}/libmirrormeshtypes.h
  ${MIRRORMESH_BINARY_DIR}/libmirrormeshtypesf.h
  ${MIRRORMESH_BINARY_DIR}/mirrormeshversion.h
  )
IF (NOT WIN32 OR MINGW)
  LIST(APPEND mirrormesh_headers  ${MIRRORMESH_BINARY_DIR}/git_log_mirrormesh.h )
ENDIF()


# Install header files in /usr/local or equivalent
INSTALL(FILES ${mirrormesh_headers} DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/mirrormesh COMPONENT headers )

# Copy header files in project directory at every build step
COPY_HEADERS_AND_CREATE_TARGET ( ${MIRRORMESH_SOURCE_DIR} ${MIRRORMESH_BINARY_DIR} ${MIRRORMESH_INCLUDE} )


install(EXPORT MirrormeshTargets
  FILE MirrormeshTargets.cmake
  NAMESPACE Mirrormesh::
  DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/mirrormesh
  )

IF (LIBMIRRORMESH_STATIC OR LIBMIRRORMESH_SHARED)
  SET(LIBMIRRORMESH_INTERNAL ON )
ELSE()
  SET(LIBMIRRORMESH_INTERNAL OFF )
ENDIF()

###############################################################################
#####
#####         Compile MIRRORMESH executable
#####
###############################################################################
ADD_AND_INSTALL_EXECUTABLE ( ${PROJECT_NAME}
  "${mirrormesh_library_files}" ${mirrormesh_main_file} )

IF ( DOWNLOAD_MMG )
  Add_Dependencies(mirrormesh Mmg)
ENDIF ( )

IF ( DOWNLOAD_METIS )
  Add_Dependencies(mirrormesh Metis)
ENDIF ( )

############################################################################
#####
#####         Print git hash to source file
#####
############################################################################

IF (NOT WIN32 OR MINGW)

  ADD_CUSTOM_TARGET(GenerateGitHash
    COMMAND ./git_log_mirrormesh.sh ${PROJECT_SOURCE_DIR} ${MIRRORMESH_BINARY_DIR}
    WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}/scripts/
    COMMENT "Getting git commit hash"
  )
  ADD_DEPENDENCIES(${PROJECT_NAME} GenerateGitHash)
  IF( LIBMIRRORMESH_STATIC )
    ADD_DEPENDENCIES(lib${PROJECT_NAME}_a GenerateGitHash)
  ENDIF ()
  IF( LIBMIRRORMESH_SHARED )
    ADD_DEPENDENCIES(lib${PROJECT_NAME}_so GenerateGitHash)
  ENDIF ()

  INCLUDE_DIRECTORIES(${MIRRORMESH_BINARY_DIR})

ENDIF ()

###############################################################################
#####
#####         Create API Documentation
#####
###############################################################################
FIND_PACKAGE(Doxygen)
IF(DOXYGEN_FOUND)

  # MIRRORMESH Documentation
  CONFIGURE_FILE(${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile.in
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile @ONLY)

  ADD_CUSTOM_TARGET(mirrormesh_doc
    COMMAND ${DOXYGEN_EXECUTABLE}
    ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/Doxyfile
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/doc/doxygen/
    COMMENT "Generating MirrorMesh API documentation with Doxygen.
    Open the doc/doxygen/html/index.html file to see it." VERBATIM
    )


  if ( NOT TARGET doc )
    ADD_CUSTOM_TARGET(doc
      DEPENDS mirrormesh_doc
      COMMENT "Generating MirrorMesh API documentation with Doxygen.
     Open the doc/doxygen/html/index.html file to see it" VERBATIM
      )
  else()
    add_dependencies(doc mirrormesh_doc)
  endif()
ENDIF ( DOXYGEN_FOUND )

OPTION ( BUILD_TESTING "Enable / Disable tests" OFF )

INCLUDE( ${PROJECT_SOURCE_DIR}/cmake/testing/mirrormesh_tests.cmake )
