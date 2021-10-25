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
 * \file mirrormesh.h
 * \brief internal functions headers for mirrormesh
 * \author Cécile Dobrzynski (Bx INP/Inria)
 * \author Algiane Froehly (Inria)
 * \version 5
 * \copyright GNU Lesser General Public License.
 */

#ifndef _MIRRORMESH_H
#define _MIRRORMESH_H

#include "mmg3d.h"
#include "mirrormeshversion.h"

#ifdef __cplusplus
extern "C" {
#endif

int MIRRORMESH_parsar(int argc,char *argv[],MMG5_pMesh,MMG5_pSol,MMG5_pSol,int*,int*,int* );
int MIRRORMESH_usage( char * );
int MIRRORMESH_mirror(MMG5_pMesh mesh,int nx,int ny,int nz);

#ifdef __cplusplus
}
#endif

#endif
