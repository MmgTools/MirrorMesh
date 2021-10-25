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
 * \file mirrormesh.c
 * \brief main file for the mirrormesh application
 * \author Algiane Froehly (Inria)
 * \version 1
 * \copyright GNU Lesser General Public License.
 */

#include "mirrormesh.h"
#include "mirrormeshversion.h"

mytime         MMG5_ctim[TIMEMAX];

/**
 *
 * Print elapsed time at end of process.
 *
 */
static void MIRRORMESH_endcod() {
  char   stim[32];

  chrono(OFF,&MMG5_ctim[0]);
  printim(MMG5_ctim[0].gdif,stim);
  fprintf(stdout,"\n   ELAPSED TIME  %s\n",stim);
}

int MIRRORMESH_usage(char *prog) {

  fprintf(stdout,"\nUsage: %s [-v [n]] [opts..] filein [fileout]\n",prog);

  fprintf(stdout,"\n** Generic options :\n");
  fprintf(stdout,"-h        Print this message\n");
  fprintf(stdout,"-v [n]    Tune level of verbosity, [-1..10]\n");
  fprintf(stdout,"-m [n]    Set maximal memory size to n Mbytes\n");

  fprintf(stdout,"\n**  File specifications\n");
  fprintf(stdout,"-in  file  input triangulation\n");
  fprintf(stdout,"-out file  output triangulation\n");

  fprintf(stdout,"\n**  Parameters\n");
  fprintf(stdout,"-nx       Number of mirrors along x-axis (default is 1) \n");
  fprintf(stdout,"-ny       Number of mirrors along y-axis (default is 1) \n");
  fprintf(stdout,"-nz       Number of mirrors along z-axis (default is 1) \n");
  fprintf(stdout,"\n\n");

  return 1;
}


int MIRRORMESH_parsar(int argc,char *argv[],MMG5_pMesh mesh,
                      MMG5_pSol met,MMG5_pSol ls,int *nx, int *ny, int *nz) {
  MMG5_pSol tmp = NULL;
  int     i;
  char    namein[128];

  /* Second step: read all other arguments. */
  i = 1;
  while ( i < argc ) {
    if ( *argv[i] == '-' ) {
      switch(argv[i][1]) {
      case '?':
        MIRRORMESH_usage(argv[0]);
        return 0;

      case 'h':
        MIRRORMESH_usage(argv[0]);
        return 0;

      case 'i':
        if ( !strcmp(argv[i],"-in") ) {
          if ( ++i < argc && isascii(argv[i][0]) && argv[i][0]!='-') {
            if ( !MMG3D_Set_inputMeshName(mesh, argv[i]) )
              return 0;

          }else{
            fprintf(stderr,"Missing filname for %c%c\n",argv[i-1][1],argv[i-1][2]);
            MIRRORMESH_usage(argv[0]);
            return 0;
          }
        }
        else {
          fprintf(stderr,"Unexpected argument option %s\n",argv[i]);
          MIRRORMESH_usage(argv[0]);
          return 0;
        }
        break;

      case 'm':
        if ( !strcmp(argv[i],"-m") ) {
          /* memory */
          if ( ++i < argc && isdigit(argv[i][0]) ) {
            if ( !MMG3D_Set_iparameter(mesh,met,MMG3D_IPARAM_mem,atoi(argv[i])) )
              return 0;
          }
          else {
            fprintf(stderr,"Missing argument option %c\n",argv[i-1][1]);
            MIRRORMESH_usage(argv[0]);
            return 0;
          }
        }
        else {
          fprintf(stderr,"Unexpected argument option %s\n",argv[i]);
          MIRRORMESH_usage(argv[0]);
          return 0;
        }
        break;
      case 'n':
        if ( !strcmp(argv[i],"-nx") ) {
          if ( ++i < argc && isdigit(argv[i][0]) ) {
            *nx = atoi(argv[i]);
          }
          else if ( i == argc ) {
            fprintf(stderr,"Missing argument option %s\n",argv[i-1]);
            MIRRORMESH_usage(argv[0]);
            return 0;
          }
          else {
            fprintf(stderr,"Missing argument option %s\n",argv[i-1]);
            MIRRORMESH_usage(argv[0]);
            i--;
            return 0;
          }
        }
        else if ( !strcmp(argv[i],"-ny") ) {
          if ( ++i < argc && isdigit(argv[i][0]) ) {
            *ny = atoi(argv[i]);
          }
          else if ( i == argc ) {
            fprintf(stderr,"Missing argument option %s\n",argv[i-1]);
            MIRRORMESH_usage(argv[0]);
            return 0;
          }
          else {
            fprintf(stderr,"Missing argument option %s\n",argv[i-1]);
            MIRRORMESH_usage(argv[0]);
            i--;
            return 0;
          }
        }
        else if ( !strcmp(argv[i],"-nz") ) {
          if ( ++i < argc && isdigit(argv[i][0]) ) {
            *nz = atoi(argv[i]);
          }
          else if ( i == argc ) {
            fprintf(stderr,"Missing argument option %s\n",argv[i-1]);
            MIRRORMESH_usage(argv[0]);
            return 0;
          }
          else {
            fprintf(stderr,"Missing argument option %s\n",argv[i-1]);
            MIRRORMESH_usage(argv[0]);
            i--;
            return 0;
          }
        }
        else {
          fprintf(stderr,"Unexpected argument option %s\n",argv[i]);
          MIRRORMESH_usage(argv[0]);
          return 0;
        }
        break;

      case 'o':
        if ( !strcmp(argv[i],"-out") ) {
          if ( ++i < argc && isascii(argv[i][0])  && argv[i][0]!='-') {
            if ( !MMG3D_Set_outputMeshName(mesh,argv[i]) )
              return 0;
          }else{
            fprintf(stderr,"Missing filname for %c%c%c\n",
                    argv[i-1][1],argv[i-1][2],argv[i-1][3]);
            MIRRORMESH_usage(argv[0]);
            return 0;
          }
        }
        else {
          fprintf(stderr,"Unrecognized option %s\n",argv[i]);
          MIRRORMESH_usage(argv[0]);
          return 0;
        }
        break;
      case 'v':
        if ( ++i < argc ) {
          if ( isdigit(argv[i][0]) ||
               (argv[i][0]=='-' && isdigit(argv[i][1])) ) {
            if ( !MMG3D_Set_iparameter(mesh,met,MMG3D_IPARAM_verbose,atoi(argv[i])) )
              return 0;
          }
          else {
            i--;
            fprintf(stderr,"Missing argument option %s\n",argv[i]);
          }
        }
        else {
          fprintf(stderr,"Missing argument option %s\n",argv[i-1]);
          MIRRORMESH_usage(argv[0]);
          return 0;
        }
        break;
      default:
        fprintf(stderr,"Unrecognized option %s\n",argv[i]);
        MIRRORMESH_usage(argv[0]);
        return 0;
      }
    }
    else {
      if ( mesh->namein == NULL ) {
        if ( !MMG3D_Set_inputMeshName(mesh,argv[i]) )
          return 0;
        if ( mesh->info.imprim == -99 ) {
          if ( !MMG3D_Set_iparameter(mesh,met,MMG3D_IPARAM_verbose,5) )
            return 0;
        }
      }
      else if ( mesh->nameout == NULL ) {
        if ( !MMG3D_Set_outputMeshName(mesh,argv[i]) )
          return 0;
      }
      else {
        fprintf(stdout,"Argument %s ignored\n",argv[i]);
        MIRRORMESH_usage(argv[0]);
        return 0;
      }
    }
    i++;
  }

  /* check file names */
  if ( mesh->info.imprim == -99 ) {
    fprintf(stdout,"\n  -- PRINT (0 10(advised) -10) ?\n");
    fflush(stdin);
    MMG_FSCANF(stdin,"%d",&i);
    if ( !MMG3D_Set_iparameter(mesh,met,MMG3D_IPARAM_verbose,i) )
      return 0;
  }

  if ( mesh->namein == NULL ) {
    fprintf(stdout,"  -- INPUT MESH NAME ?\n");
    fflush(stdin);
    MMG_FSCANF(stdin,"%127s",namein);
    if ( !MMG3D_Set_inputMeshName(mesh,namein) )
      return 0;
  }

  if ( mesh->nameout == NULL ) {
    if ( !MMG3D_Set_outputMeshName(mesh,"") )
      return 0;
  }

  return 1;
}

/**
 * \param argc number of command line arguments.
 * \param argv command line arguments.
 * \return \ref MMG5_SUCCESS if success.
 * \return \ref MMG5_LOWFAILURE if failed but a conform mesh is saved.
 * \return \ref MMG5_STRONGFAILURE if failed and we can't save the mesh.
 *
 * Main program for MMG3D executable: perform mesh adaptation.
 *
 */
int main(int argc,char *argv[]) {

  MMG5_pMesh      mesh;
  MMG5_pSol       sol,met,disp,ls;
  int             ier,ierSave,fmtin,fmtout;
  char            stim[32],*ptr;

  fprintf(stdout,"  -- MIRRORMESH, Release %s (%s) \n",
          MIRRORMESH_VERSION_RELEASE,MIRRORMESH_RELEASE_DATE);
  fprintf(stdout,"     %s\n",MIRRORMESH_COPYRIGHT);
  fprintf(stdout,"     %s %s\n",__DATE__,__TIME__);

  MMG3D_Set_commonFunc();

  /* Print timer at exit */
  atexit(MIRRORMESH_endcod);

  tminit(MMG5_ctim,TIMEMAX);
  chrono(ON,&MMG5_ctim[0]);


  /* assign default values */
  mesh = NULL;
  met  = NULL;
  disp = NULL;
  ls   = NULL;

  MMG3D_Init_mesh(MMG5_ARG_start,
                  MMG5_ARG_ppMesh,&mesh,MMG5_ARG_ppMet,&met,
                  MMG5_ARG_ppDisp,&disp,
                  MMG5_ARG_ppLs,&ls,
                  MMG5_ARG_end);

  /* reset default values for file names */
  if ( !MMG3D_Free_names(MMG5_ARG_start,
                         MMG5_ARG_ppMesh,&mesh,MMG5_ARG_ppMet,&met,
                         MMG5_ARG_ppDisp,&disp,
                         MMG5_ARG_ppLs,&ls,
                         MMG5_ARG_end) )
    return MMG5_STRONGFAILURE;


  /* Set default metric size */
  if ( !MMG3D_Set_solSize(mesh,met,MMG5_Vertex,0,MMG5_Scalar) )
    MMG5_RETURN_AND_FREE(mesh,met,ls,disp,MMG5_STRONGFAILURE);

  /* command line */
  int nx,ny,nz;
  nx = ny = nz = 1;
  if ( !MIRRORMESH_parsar(argc,argv,mesh,met,ls,&nx,&ny,&nz) )
    return MMG5_STRONGFAILURE;

  /* load data */
  if ( mesh->info.imprim >= 0 )
    fprintf(stdout,"\n  -- INPUT DATA\n");
  chrono(ON,&MMG5_ctim[1]);

  /* read mesh/sol files */
  ptr   = MMG5_Get_filenameExt(mesh->namein);
  fmtin = MMG5_Get_format(ptr,MMG5_FMT_MeditASCII);

  switch ( fmtin ) {

  case ( MMG5_FMT_GmshASCII ): case ( MMG5_FMT_GmshBinary ):
    ier = MMG3D_loadMshMesh(mesh,sol,mesh->namein);
    break;

  case ( MMG5_FMT_VtkVtu ):
    ier = MMG3D_loadVtuMesh(mesh,sol,mesh->namein);
    break;

  case ( MMG5_FMT_VtkVtk ):
    ier = MMG3D_loadVtkMesh(mesh,sol,mesh->namein);
    break;

  case ( MMG5_FMT_MeditASCII ): case ( MMG5_FMT_MeditBinary ):
    ier = MMG3D_loadMesh(mesh,mesh->namein);
    break;

  default:
    fprintf(stderr,"  ** I/O AT FORMAT %s NOT IMPLEMENTED.\n",MMG5_Get_formatName(fmtin) );
    MMG5_RETURN_AND_FREE(mesh,met,ls,disp,MMG5_STRONGFAILURE);
  }

  if ( ier<1 ) {
    if ( ier==0 ) {
      fprintf(stderr,"  ** %s  NOT FOUND.\n",mesh->namein);
      fprintf(stderr,"  ** UNABLE TO OPEN INPUT FILE.\n");
    }
    MMG5_RETURN_AND_FREE(mesh,met,ls,disp,MMG5_STRONGFAILURE);
  }

  /* Check input data */
  if ( mesh->info.lag > -1 ) {
    if ( met->namein ) {
      fprintf(stdout,"  ## WARNING: MESH ADAPTATION UNAVAILABLE IN"
              " LAGRANGIAN MODE. METRIC IGNORED.\n");
      MMG5_RETURN_AND_FREE(mesh,met,ls,disp,MMG5_STRONGFAILURE);
    }
  }
  else if ( mesh->info.iso ) {
     if ( ls == NULL || ls->m == NULL ) {
      fprintf(stderr,"\n  ## ERROR: NO ISOVALUE DATA.\n");
      MMG5_RETURN_AND_FREE(mesh,met,ls,disp,MMG5_STRONGFAILURE);
    }
  }

  chrono(OFF,&MMG5_ctim[1]);
  if ( mesh->info.imprim >= 0 ) {
    printim(MMG5_ctim[1].gdif,stim);
    fprintf(stdout,"  -- DATA READING COMPLETED.     %s\n",stim);
  }

  ier = MIRRORMESH_mirror(mesh,nx,ny,nz);

  if ( ier != MMG5_STRONGFAILURE ) {
    /** Save files at medit or Gmsh format */
    chrono(ON,&MMG5_ctim[1]);
    if ( mesh->info.imprim > 0 )
      fprintf(stdout,"\n  -- WRITING DATA FILE %s\n",mesh->nameout);

    ptr    = MMG5_Get_filenameExt(mesh->nameout);
    fmtout = MMG5_Get_format(ptr,fmtin);
    switch ( fmtout ) {
    case ( MMG5_FMT_GmshASCII ): case ( MMG5_FMT_GmshBinary ):
      ierSave = MMG3D_saveMshMesh(mesh,met,mesh->nameout);
      break;
    case ( MMG5_FMT_VtkVtu ):
      ierSave = MMG3D_saveVtuMesh(mesh,met,mesh->nameout);
      break;
    case ( MMG5_FMT_VtkVtk ):
      ierSave = MMG3D_saveVtkMesh(mesh,met,mesh->nameout);
      break;
    default:
      ierSave = MMG3D_saveMesh(mesh,mesh->nameout);
      if ( !ierSave ) {
        MMG5_RETURN_AND_FREE(mesh,met,ls,disp,MMG5_STRONGFAILURE);
      }
      if ( met && met->np ) {
        ierSave = MMG3D_saveSol(mesh,met,met->nameout);
      }
      break;
    }

    if ( !ierSave )
      MMG5_RETURN_AND_FREE(mesh,met,ls,disp,MMG5_STRONGFAILURE);

    chrono(OFF,&MMG5_ctim[1]);
    if ( mesh->info.imprim > 0 )
      fprintf(stdout,"  -- WRITING COMPLETED\n");
  }

  /* free mem */
  MMG5_RETURN_AND_FREE(mesh,met,ls,disp,ier);
}
