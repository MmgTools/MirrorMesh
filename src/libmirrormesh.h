/* =============================================================================
**  This file is part of the mirrormesh software package for parallel tetrahedral
**  mesh modification.
**  Copyright (c) Bx INP/Inria/UBordeaux, 2017-
**
**  mirrormesh is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  mirrormesh is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with mirrormesh (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the mirrormesh distribution only if you accept them.
** =============================================================================
*/

/**
 * \file libmirrormesh.h
 * \brief API headers for the mirrormesh library
 * \author
 * \version
 * \date 11 2016
 * \copyright
 */

#ifndef _MIRRORMESHLIB_H
#define _MIRRORMESHLIB_H

#include "libmirrormeshtypes.h"
#include "metis.h"

#if defined(c_plusplus) || defined(__cplusplus)
extern "C" {
#endif

/**
 * \param mesh pointer toward a MMG5_Mesh mesh structure
 *       (that can be initialized using the Mmg API)
 *
 * \param nx number of mirrors along x-direction
 * \param ny number of mirrors along y-direction
 * \param nz number of mirrors along z-direction
 *
 * \return \ref MMG5_SUCCESS if success, \ref MMG5_LOWFAILURE if fail but we can
 * save a conformal mesh \ref MMG5_STRONGFAILURE if fail and
 * we can't save a conformal mesh.
 *
 * Mesh mirroring: replicates a mesh by mirroring along each direction.
 *
 * \remark Fortran interface:
 * >   SUBROUTINE MIRRORMESH_mirror(mesh,nx,ny,nz,retval)\n
 * >     MMG5_DATA_PTR_T,INTENT(INOUT) :: mesh\n
 * >     INTEGER, INTENT(IN)           :: nx,ny,nz\n
 * >     INTEGER, INTENT(OUT)          :: retval\n
 * >   END SUBROUTINE\n
 *
 **/
int MIRRORMESH_mirror(MMG5_pMesh mesh,int nx, int ny, int nz);


#if defined(c_plusplus) || defined(__cplusplus)
}
#endif

#endif
