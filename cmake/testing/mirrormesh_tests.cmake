## =============================================================================
##  This file is part of the mmg software package for the tetrahedral
##  mesh modification.
##**  Copyright (c) Bx INP/Inria/UBordeaux/UPMC, 2004- .
##
##  mmg is free software: you can redistribute it and/or modify it
##  under the terms of the GNU Lesser General Public License as published
##  by the Free Software Foundation, either version 3 of the License, or
##  (at your option) any later version.
##
##  mmg is distributed in the hope that it will be useful, but WITHOUT
##  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
##  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
##  License for more details.
##
##  You should have received a copy of the GNU Lesser General Public
##  License and of the GNU General Public License along with mmg (in
##  files COPYING.LESSER and COPYING). If not, see
##  <http://www.gnu.org/licenses/>. Please read their terms carefully and
##  use this copy of the mmg distribution only if you accept them.
## =============================================================================

###############################################################################
#####
#####         Continuous Integration
#####
###############################################################################

IF( BUILD_TESTING )
  include( CTest )

  SET ( MIRRORMESH_CI_TESTS ${CMAKE_SOURCE_DIR}/cmake/testing )

  # Simple test: must already pass (-d option allows to cover chkmsh function)
  ADD_TEST(NAME mirrormesh_Simple
    COMMAND $<TARGET_FILE:${PROJECT_NAME}> -v 5
    ${MIRRORMESH_CI_TESTS}/0.mesh
    -out ${CMAKE_BINARY_DIR}/mirrormesh_0.o.mesh)

ENDIF()
