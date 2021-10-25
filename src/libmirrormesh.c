/* =============================================================================
**  This file is part of the mirrormesh software package for the tetrahedral
**  mesh modification.
**  Copyright (c) Bx INP/CNRS/Inria/UBordeaux/UPMC, 2004-
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
 * \file libmirrormesh.c
 * \brief Main API function for the MIRRORMESH library.
 * \author Algiane Froehly (Inria)
 * \version 1
 * \date 10 2012
 * \copyright GNU Lesser General Public License.
 *
 * Main API functions for MIRRORMESH library
 *
 */
#include "mirrormesh.h"

/**
 * \param mesh mesh structure
 * \param dim working dimension
 * \param npinit number of points to mirror
 * \param nmir number of mirrors
 * \param axis direction for mirroring
 * \param epsilon tolerance to consider 2 points as duplicated
 *
 * \return npend the new number of points.
 *
 * Apply mirroring along \a axis direction to an array of points.
 *
 * Mirroring is applied \a nmir times along the \a axis direction.
 * The upper boundary of the mesh bounding box is used as symmetry plane.
 *
 */
static
int MIRRORMESH_mirror_points_1d(MMG5_pMesh mesh,int dim, int npinit,int nmir,
                                int axis,double eps) {
  MMG5_pPoint pptn, ppt;
  double f[3],delta[3],dist[3],sqnorm;
  int imir,k,mir_mask[3],i;

  for ( i=0; i<dim; ++i) {
    delta[i] = mesh->info.max[i] - mesh->info.min[i];
    mir_mask[i] = 0;
  }
  mir_mask[axis] = 1;

  for (imir=1; imir<=nmir; ++imir) {
    for (k=1; k<=npinit; ++k) {
      /* Copy point */
      pptn = &mesh->point[k+imir*npinit];
      ppt  = &mesh->point[k+(imir-1)*npinit];
      memcpy(pptn,ppt,sizeof(MMG5_Point));

      /* Initialization of the array to store 'true' indices of duplicated points */
      pptn->tmp = mesh->point[k].tmp+imir*(npinit);
      pptn->tag &= ~MG_NUL;
    }

    /* compute the reflexion of the points coordinates: formula for
       a mirroring of point P along the wall of coor W is is: Pn =
       W + W-P = 2W - P. We want to write it under the form Pn =
       mir_mask[axis]*f + P, s.a. if mir_mask[axis]==0, then
       coordinates along `axis` are preserved and if
       mir_mask[axis]==1, Pn = f + P. Thus,  Pn = 2(W-P) + P.
       It gives f = 2*(bb_max+(imir-1)*bb_delta+bb_xmax-P). */
    for (k=1; k<=npinit; ++k) {
      for ( i=0; i<dim; ++i) {
        f[i] = 2.*((imir-1)*delta[i]+mesh->info.max[i]-mesh->point[k+(imir-1)*(npinit)].c[i]);
      }

      sqnorm = 0.;

      pptn = &mesh->point[k+imir*npinit];
      ppt  = &mesh->point[k+(imir-1)*npinit];

      for ( i=0; i<dim; ++i) {

        /* Update coordinates */
        pptn->c[i] = mir_mask[i] * f[i] + ppt->c[i];

        /* Check the distance between the point and its image to mark
         * duplicated point */
        dist[i] = pptn->c[i] - ppt->c[i];
        sqnorm += dist[i];
      }

      if ( sqnorm < eps ) {
        /* if points are too close, store the index to use */
        pptn->tmp =  k+(imir-1)*npinit;
        pptn->tag |= MG_NUL;
      }
    }
  }
  return (nmir+1) * npinit;

}

/**
 * \param mesh mesh structure
 * \param dim working dimension
 * \param nmir number of mirrors in each direction
 * \param eps tolerance to consider 2 points as duplicated
 *
 * \return 1 if success
 *
 * Apply mirroring to an array of points.
 *
 * The upper boundary of the mesh bounding box is used as symmetry plane.
 *
 */
static
int MIRRORMESH_mirror_points(MMG5_pMesh mesh,int dim,
                             int nmir[3],double eps) {
  int i,nmirtot;

  /* Get initial number of points */
  int npinit = mesh->npi;

  /* Compute total number of mirrors */
  nmirtot = 1;
  for (i=0; i<dim; ++i ) {
    nmirtot *= (nmir[i]+1);
  }

  /* Reallocation of point array */
  MMG5_ADD_MEM(mesh,(nmirtot*npinit-(mesh->npmax)) *sizeof(MMG5_Point),
                   "larger point array",return 0);

  MMG5_SAFE_RECALLOC(mesh->point,(mesh->npmax+1),nmirtot*npinit+1,
                     MMG5_Point,"larger point array",return 0);

  mesh->npmax = nmirtot*npinit;

  /* Bounding box computation */
  if ( !MMG5_boundingBox(mesh) ) {
    return 0;
  }

  int npcur = npinit;

  int k;
  for (k=1; k<=npinit; ++k) {
    /* Initialization of the array to store 'true' indices of duplicated points */
    mesh->point[k].tmp = k;
  }

  for (i=0; i<dim; ++i ) {
    npcur = MIRRORMESH_mirror_points_1d(mesh,dim,npcur,nmir[i],i,eps);
  }
  mesh->np = npcur;

  return 1;
}

/**
 * \param mesh mesh structure
 * \param dim working dimension
 * \param nmir number of mirrors in each direction
 *
 * \return 1 if success
 *
 * Apply mirroring to an array of points.
 *
 * The upper boundary of the mesh bounding box is used as symmetry plane.
 *
 */
static
int MIRRORMESH_mirror_cells(MMG5_pMesh mesh,int dim,int *nmir) {
  MMG5_pTetra ptn, pt;
  int i,nmirtot,neend,naend,ntend;

  /* Get initial number of tetra, tria and edges */
  int neinit = mesh->nei;
  int ntinit = mesh->nti;
  int nainit = mesh->nai;

  /* Compute total number of mirrors */
  nmirtot = 1;
  for (i=0; i<dim; ++i ) {
    nmirtot *= (nmir[i]+1);
  }

  /* Reallocation of tetra */
  MMG5_ADD_MEM(mesh,(nmirtot*neinit-(mesh->nemax))*sizeof(MMG5_Tetra),
                   "larger tetra array",return 0);

  MMG5_SAFE_RECALLOC(mesh->tetra,(mesh->nemax+1),nmirtot*neinit+1,
                     MMG5_Tetra,"larger tetra array",return 0);
  mesh->nemax = mesh->ne = nmirtot*neinit+1;

  /* Reallocation of triangles */
  MMG5_ADD_MEM(mesh,(nmirtot*ntinit-(mesh->nt))*sizeof(MMG5_Tria),
                   "larger triangle array",return 0);

  MMG5_SAFE_RECALLOC(mesh->tria,(mesh->nt+1),nmirtot*ntinit+1,
                     MMG5_Tria,"larger triangle array",return 0);
  mesh->nt = nmirtot*ntinit+1;

  /* Reallocation of edges */
  MMG5_ADD_MEM(mesh,(nmirtot*nainit-(mesh->na))*sizeof(MMG5_Edge),
                   "larger edge array",return 0);

  MMG5_SAFE_RECALLOC(mesh->edge,(mesh->na+1),nmirtot*nainit+1,
                     MMG5_Edge,"larger edge array",return 0);
  mesh->na = nmirtot*nainit+1;

  int npcur  = mesh->npi;

  /* Tetra */
  int necur  = neinit;
  int ntcur  = ntinit;
  int nacur  = nainit;

  int idim;
  for (idim=0; idim<dim; ++idim ) {
    int imir;
    for ( imir = 0; imir < nmir[idim]; ++imir) {

      /* Tetra replication */
      int k;
      for (k=1; k<=necur; ++k) {
        pt  = &mesh->tetra[k];

        int8_t dup = 1;
        if ( pt->v[0]>0 ) {
          for ( i=0; i<4; ++i) {
            if ( MG_VOK(&mesh->point[pt->v[i]+(imir+1)*(npcur)]) ) {
              dup = 0;
              break;
            }
          }
        }

        ptn = &mesh->tetra[k+(imir+1)*(necur)];
        memcpy(ptn,pt,sizeof(MMG5_Tetra));

        if ( dup ) {
          /* Duplicated element */
          ptn->v[0] = 0;
        }
        else {
          for ( i=0; i<4; ++i) {
            ptn->v[i] = mesh->point[pt->v[i]+(imir+1)*(npcur)].tmp;
          }
          if ( imir%2==0 ) {
            /* Reorientation */
            int tmp = ptn->v[3];
            ptn->v[3] = ptn->v[2];
            ptn->v[2] = tmp;
          }
        }
      }

      /* Tria replication */
      for (k=1; k<=ntcur; ++k) {
        MMG5_pTria ptt  = &mesh->tria[k];

        int8_t dup = 1;
        if ( ptt->v[0]>0 ) {
          for ( i=0; i<3; ++i) {
            if ( MG_VOK(&mesh->point[ptt->v[i]+(imir+1)*(npcur)]) ) {
              dup = 0;
              break;
            }
          }
        }

        MMG5_pTria pttn = &mesh->tria[k+(imir+1)*(ntcur)];
        memcpy(pttn,ptt,sizeof(MMG5_Tria));

        if ( dup ) {
          /* Duplicated element */
          pttn->v[0] = 0;
        }
        else {
          for ( i=0; i<3; ++i) {
            pttn->v[i] = mesh->point[ptt->v[i]+(imir+1)*(npcur)].tmp;
          }
          if ( imir%2==1 ) {
            /* Reorientation */
            int tmp = pttn->v[2];
            pttn->v[2] = pttn->v[1];
            pttn->v[1] = tmp;
          }
        }
      }

      /* Edges replication */
      for (k=1; k<=nacur; ++k) {
        MMG5_pEdge pa  = &mesh->edge[k];

        int8_t dup = 1;
        if ( pa->a>0 ) {
          if ( MG_VOK(&mesh->point[pa->a+(imir+1)*(npcur)]) ) {
            dup = 0;
          }
          if ( MG_VOK(&mesh->point[pa->b+(imir+1)*(npcur)]) ) {
            dup = 0;
          }
        }

        MMG5_pEdge pan = &mesh->edge[k+(imir+1)*(nacur)];
        memcpy(pan,pa,sizeof(MMG5_Edge));

        if ( dup ) {
          /* Duplicated element */
          pan->a = 0;
        }
        else {
          pan->a = mesh->point[pa->a+(imir+1)*(npcur)].tmp;
          pan->b = mesh->point[pa->b+(imir+1)*(npcur)].tmp;
          if ( imir%2==1 ) {
            /* Reorientation */
            int tmp = pan->b;
            pan->b = pan->a;
            pan->a = tmp;
          }
        }
      }
    }

    necur *= (nmir[idim]+1);
    ntcur *= (nmir[idim]+1);
    nacur *= (nmir[idim]+1);
    npcur *= (nmir[idim]+1);
  }
  mesh->ne = necur;
  mesh->nt = ntcur;
  mesh->np = npcur;
  mesh->na = nacur;

  return 1;
}

/**
 * \param mesh pointer toward the mesh structure
 *
 * \return 1 if success, 0 if fail.
 *
 * Pack the sparse triangles
 * Don't preserve numbering order.
 *
 */
static
int MIRRORMESH_pack_edges(MMG5_pMesh mesh) {
  MMG5_pEdge   pt,pt1;
  int          k;

  if ( !mesh->na ) {
    return 1;
  }

  if ( mesh->edge ) {
    k = 1;
    do {
      pt = &mesh->edge[k];
      if ( !pt->a ) {
        while ( (!mesh->edge[mesh->na].a) && mesh->na > k ) {
          --mesh->na;
        }
        if ( mesh->na == k ) {
          break;
        }

        pt1 = &mesh->edge[mesh->na];
        assert( pt && pt1 && pt1->a );
        memcpy(pt,pt1,sizeof(MMG5_Edge));
        pt1->a = 0;
      }
    }
    while ( ++k < mesh->na );

    if ( !mesh->edge[mesh->na].a ) {
      --mesh->na;
    }

    /* Recreate nil chain */
    if ( mesh->na >= mesh->namax-1 )
      mesh->nanil = 0;
    else
      mesh->nanil = mesh->na + 1;

    if ( mesh->nanil )
      for(k=mesh->nanil; k<mesh->namax-1; k++)
        mesh->edge[k].a = 0;
  }
  mesh->nai = mesh->na;

  return 1;
}

/**
 * \param mesh pointer toward the mesh structure
 *
 * \return 1 if success, 0 if fail.
 *
 * Pack the sparse triangles
 * Don't preserve numbering order.
 *
 */
static
int MIRRORMESH_pack_tria(MMG5_pMesh mesh) {
  MMG5_pTria   pt,pt1;
  int          k;

  if ( !mesh->nt ) {
    return 1;
  }

  if ( mesh->tria ) {
    k = 1;
    do {
      pt = &mesh->tria[k];
      if ( !MG_EOK(pt) ) {
        while ( (!MG_EOK(&mesh->tria[mesh->nt])) && mesh->nt > k ) {
          --mesh->nt;
        }
        if ( mesh->nt == k ) {
          break;
        }
        pt1 = &mesh->tria[mesh->nt];
        assert( pt && pt1 && MG_EOK(pt1) );
        memcpy(pt,pt1,sizeof(MMG5_Tria));
        pt1->v[0] = 0;
      }
    }
    while ( ++k < mesh->nt );
  }

  if ( !MG_EOK(&mesh->tria[mesh->nt]) ) {
    --mesh->nt;
  }

  mesh->nti = mesh->nt;

  return 1;
}

/**
 * \param mesh pointer toward the mesh structure
 *
 * \return 1 if success, 0 if fail.
 *
 * Pack the sparse tetra.
 * Don't preserve numbering order.
 *
 */
int MIRRORMESH_pack_tetra(MMG5_pMesh mesh) {
  MMG5_pTetra   pt,pt1;
  int           k;

  if ( !mesh->ne ) {
    return 1;
  }

  if ( mesh->tetra ) {
    k = 1;
    do {
      pt = &mesh->tetra[k];
      if ( !MG_EOK(pt) ) {
        while ( (!MG_EOK(&mesh->tetra[mesh->ne])) && mesh->ne > k ) {
          --mesh->ne;
        }
        if ( mesh->ne == k ) {
          break;
        }
        pt1 = &mesh->tetra[mesh->ne];
        assert( pt && pt1 && MG_EOK(pt1) );
        memcpy(pt,pt1,sizeof(MMG5_Tetra));
        /* Delete element */
        pt1->v[0] = 0;
      }
    }
    while ( ++k < mesh->ne );

    if ( !MG_EOK(&mesh->tetra[mesh->ne]) ) {
      --mesh->ne;
    }

    /* Recreate nil chain */
    if ( mesh->ne >= mesh->nemax-1 )
      mesh->nenil = 0;
    else
      mesh->nenil = mesh->ne + 1;

    if ( mesh->nenil )
      for(k=mesh->nenil; k<mesh->nemax-1; k++)
        mesh->tetra[k].v[0] = 0;
  }
  mesh->nei = mesh->ne;

  return 1;
}

/**
 * \param mesh pointer toward the mesh structure
 *
 * \return 1 if success, 0 if fail.
 *
 * Pack the sparse mesh elements (tria, tetra and edges).
 *
 */
static
int MIRRORMESH_packMesh(MMG5_pMesh mesh) {

  if ( !MIRRORMESH_pack_tetra(mesh) ) return 0;

  if ( !MIRRORMESH_pack_tria(mesh) ) return 0;

  if ( !MIRRORMESH_pack_edges(mesh) ) return 0;

  return 1;
}

int MIRRORMESH_mirror(MMG5_pMesh mesh,int nx, int ny, int nz) {
  mytime ctim[TIMEMAX];
  char   stim[32];

  /** In debug mode, check that all structures are allocated */
  assert ( mesh );
  assert ( mesh->point );

  if ( mesh->info.imprim >= 0 ) {
    fprintf(stdout,"\n  %s\n   MODULE MIRRORMESH: %s (%s)\n  %s\n",
            MG_STR,MIRRORMESH_VERSION_RELEASE,MIRRORMESH_RELEASE_DATE,MG_STR);
  }

  tminit(ctim,TIMEMAX);
  chrono(ON,&(ctim[0]));

  /* Check options */
  if ( nx < 0 || ny < 0 || nz < 0) {
    fprintf(stdout,"\n  ## ERROR: NUMBER OF MIRRORINGS MUST BE POSOTIVE.\n");
    return MMG5_LOWFAILURE;
  }

  /* Point mirroring */
  if ( mesh->info.imprim > 0 ) {
    fprintf(stdout,"\n  -- PHASE 1 : POINT MIRRORING\n");
  }
  chrono(ON,&(ctim[4]));

  int nmir[3];
  nmir[0] = nx;
  nmir[1] = ny;
  nmir[2] = nz;

  /* Working dimension */
  const int dim = 3;
  /* Tolerance over coordinates to consider a point as replicated */
  const double eps = 1e-14;

  int ier = MIRRORMESH_mirror_points(mesh,dim,nmir,eps);

  chrono(OFF,&(ctim[4]));
  printim(ctim[4].gdif,stim);
  if ( mesh->info.imprim > 0 )
    fprintf(stdout,"  -- PHASE 1 COMPLETED.     %s\n",stim);

  /* Cells mirroring */
  if ( mesh->info.imprim > 0 ) {
    fprintf(stdout,"\n  -- PHASE 2 : ELEMENT MIRRORING\n");
  }
  chrono(ON,&(ctim[5]));

  int iermesh = MIRRORMESH_mirror_cells(mesh,dim,nmir);
  if ( iermesh < 0 ) {
    fprintf(stderr,"  ## Error: unable to mirror the mesh.\n");
    return MMG5_LOWFAILURE;
  }

  chrono(OFF,&(ctim[5]));
  printim(ctim[5].gdif,stim);
  if ( mesh->info.imprim > 0 )
    fprintf(stdout,"  -- PHASE 2 COMPLETED.     %s\n",stim);


  /* Mesh compression */
  if ( mesh->info.imprim > 0 ) {
    fprintf(stdout,"\n  -- PHASE 3 : MESH PACKING\n");
  }
  chrono(ON,&(ctim[6]));

  iermesh = MIRRORMESH_packMesh(mesh);
  if ( iermesh < 0 ) {
    fprintf(stderr,"  ## Error: unable to pack the final mesh.\n");
    return MMG5_LOWFAILURE;
  }

  chrono(OFF,&(ctim[6]));
  printim(ctim[6].gdif,stim);
  if ( mesh->info.imprim > 0 )
    fprintf(stdout,"  -- PHASE 3 COMPLETED.     %s\n",stim);

  return MMG5_SUCCESS;
}
