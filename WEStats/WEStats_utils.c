/*
    WEST   Wind simulation application
    Copyright (C) 2010-2011  "Her Majesty the Queen in Right of Canada"
    Author: Anthony Chartier
    Contact: service.rpn@ec.gc.ca

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Lesser General Public
    License as published by the Free Software Foundation; either
    version 2.1 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Lesser General Public License for more details.

    You should have received a copy of the GNU Lesser General Public
    License along with this library; if not, write to the Free Software
    Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
    
*/
#include <ctype.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#include "WEStats_utils.h"
#include "fst_utils.h"

#define MAX_CAR 256 

char *trimPathname(char *s)
   {
   int end = strlen(s)-1;

   while ( isspace(s[end]) )
      {
      s[end] = '\0';
      end--;
      }

   return s;
   }

/*
  getDirContent
  
  Get a list of all files (and directories) located in the specified directory.
  
  Params :
    [out] list       : the list of entries in the directory dirname.
    [out] listLength : the number of entries found.
    [in]  dirname    : the directory to search.
    [in]  fullpath   : Convenience option. If true, add dirname to each entries.
    [in]  getSelfAndParent : Convenience option. If true, add "." and ".." to the list.
*/
void getDirContent(char ***list, int *listLength, const char *dirname, int fullpath, int getSelfAndParent)
   {
   DIR *dir = NULL;
   struct dirent *entry = NULL;
   char **tmpList = NULL;
   int tmpListLength = 0;

   *list = NULL;
   *listLength = 0;   

   dir = opendir(dirname);
   if (dir == NULL)
      {
      fprintf(stderr, "Can't open directory %s! (%s:%d)\n", dirname, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   
   *list = NULL;
   *listLength = 0;
   while ( (entry = readdir(dir)) != NULL )
      {
      char newFilename[512] = "";
   
      if ( !getSelfAndParent && (!strcmp(entry->d_name, ".") || !strcmp(entry->d_name, "..")))
         continue;
      
      tmpListLength++;
   
      tmpList = (char **) realloc(tmpList, tmpListLength * sizeof(char *));
      if (tmpList == NULL)
         {
         fprintf(stderr, "Realloc failed! (%s:%d)\n", __FILE__, __LINE__);
         exit(EXIT_FAILURE);
         }
      
      
      if ( fullpath )
         {
         if (newFilename == NULL) 
            {
            fprintf(stderr, "Malloc failed! (%s:%d)\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
            }
         sprintf(newFilename, "%s/%s", dirname, entry->d_name);
         }
      else
         {
         if (newFilename == NULL) 
            {
            fprintf(stderr, "Malloc failed! (%s:%d)\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
            }
         sprintf(newFilename, "%s", entry->d_name);
         }
      tmpList[tmpListLength-1] = strdup(newFilename);
      if (tmpList[tmpListLength-1] == NULL)
         {
         fprintf(stderr, "Strdup failed! (%s:%d)\n", __FILE__, __LINE__);
         exit(EXIT_FAILURE);
         }
        
      }
   
   closedir(dir);

   *list = tmpList;
   *listLength = tmpListLength;      
   }




/*
 getMc2DynamicFilenameList
 
 Get the list of all Mc2 dynamic files. All uu, vv, etc will be fetch from these files.

 Params:
   [out] list                   : the list of files. Null if no files are found.
   [out] listLength             : the number of files found.
   [in]  modelDirectory         : the directory where the mc2 model have been runned.
   [in]  dynamicsTimeStepNumber : 
   [in]  physicsTimeStepNumber  :
*/
void getMc2DynamicFilenameList(char ***list, int *listLength, const char *modelDirectory, int dynamicsTimeStepNumber)
   {
   char **tmpList = NULL;
   int tmpListLength = 0;
   char **contentList = NULL;
   int    contentListLength = 0;

   int i;

   getDirContent(&contentList, &contentListLength, modelDirectory, 1, 0);   
   for( i=0 ; i<contentListLength ; i++ )
      {        
      FILE *f = NULL;
      char filename[512] = "";

      sprintf(filename, "%s/output/dm1998010100-00-00_%06dp.fst", contentList[i], dynamicsTimeStepNumber);

      f = fopen(filename, "r");
      if ( f ) 
         {
         tmpListLength++;

         tmpList = (char **) realloc(tmpList, tmpListLength * sizeof(char *));
         if (tmpList == NULL)
            {
            fprintf(stderr, "Realloc failed! (%s:%d)\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
            }
         tmpList[tmpListLength-1] = strdup(filename);
         if (tmpList[tmpListLength-1] == NULL)
            {
            fprintf(stderr, "Strdup failed! (%s:%d)\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
            }      
      
         fclose(f);
         }

      free(contentList[i]);
      }
   free(contentList);
   
   
   *list = tmpList;
   *listLength = tmpListLength;
   }



void getMc2PhysicsFilename(char **physicsFilename, const char *modelDirectory, int physicsTimeStepNumber)
   {
   char *tmpFilename = NULL;
   char **contentList = NULL;
   int    contentListLength = 0;

   int i;

   getDirContent(&contentList, &contentListLength, modelDirectory, 1, 0);   
   for( i=0 ; i<contentListLength ; i++ )
      {
      FILE *f = NULL;
      char filename[512] = "";

      sprintf(filename, "%s/output/pm1998010100-00-00_000%03dp.fst", contentList[i], physicsTimeStepNumber);

      f = fopen(filename, "r");
      if ( f ) 
         {
         tmpFilename = strdup(filename);
         if (tmpFilename == NULL)
            {
            fprintf(stderr, "Strdup failed! (%s:%d)\n", __FILE__, __LINE__);
            exit(EXIT_FAILURE);
            }
         fclose(f);
         break;
         }

      free(contentList[i]);
      }
   free(contentList);

   *physicsFilename = tmpFilename;
   }

void computeUVFile(char **dynamicsFilenameList, int dynamicsFilenameListLength, char *physicsFilename, int dynamicsLevelIp1, char *uvFilename)
   {
   int ier;
   int i;
   int gridAndTopoExtracted = 0;
   float *data = NULL;

   int bidon;

   int iout = 76;
   int iin  = 77;

   int key;
   int ni, nj, nk;
   int ip1, ip2, ip3, ig1, ig2, ig3, ig4;
   int  dateo, datev, datyp, deet,nbits, npak, npas,swa, lng;
   int dltf, ubc, extra1, extra2, extra3;
   char etiket[16], nomvar[8], typvar[4], grtyp[2];
   void *work = NULL;

   int refIg1, refIg2, refIg3;

   char logFilename[MAX_CAR+1] = {0};
   FILE *logfile = fopen(logFilename, "ryy");

   strcpy(etiket, "            ");
   strcpy(nomvar, "    ");
   strcpy(typvar, "  ");
   strcpy(grtyp, " ");

   // Create logfile for computeUVFile

   strcpy(logFilename, uvFilename);
   logFilename[strlen(logFilename)-4] = '\0';
   sprintf(logFilename, "%s.log", logFilename);
   if ( logfile )
      {   
      // Delete the file if it exist
      fprintf(stderr, "Warning! Removing %s\n", logFilename);
      fclose(logfile);
      unlink(logFilename);
      }
   logfile = fopen(logFilename, "w");

   fprintf(logfile, "Creating file %s", uvFilename);
   ier = c_fnom(iout, uvFilename, "STD+RND+R/W", 0);
   if (ier < 0)
      {
      fprintf(stderr, "C_fnom failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstouv(iout, "RND");
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstouv failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   //////
   // Get stuff from the dynamics files.
   for ( i=0 ; i<dynamicsFilenameListLength ; i++)
      {
      char  *dynamicsFilename = strdup(dynamicsFilenameList[i]);

      fprintf(logfile, "Opening file:%s... ", dynamicsFilename);
      ier = c_fnom(iin, dynamicsFilename, "STD+RND+R/O", 0);
      if (ier < 0)
          {
          fprintf(stderr, "C_fnom failed! file=%s ier=%d (%s:%d)\n", dynamicsFilename, ier, __FILE__, __LINE__);
          exit(EXIT_FAILURE);
          }

      ier = c_fstouv(iin, "RND");
      if ( ier < 0)
          {
          fprintf(stderr, "C_fstouv failed! file=%s ier=%d (%s:%d)\n", dynamicsFilename, ier, __FILE__, __LINE__);
          exit(EXIT_FAILURE);
          }
      fprintf(logfile, "Done\n");

      if ( !gridAndTopoExtracted )
         {
         fprintf(logfile, "Extracting Topography and grid... ");
         // Get the topography - ME
         ier = copyFstField(iin, iout, -1,"            ", -1, -1, -1, "  ", "ME  ", NULL, &refIg1, &refIg2, &refIg3);
         if ( ier < 0 )
            {
            fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
            exit(EXIT_FAILURE);
            }

         // Get grid info -- ^^
         ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", "^^  ", NULL, &bidon, &bidon, &bidon);
         if ( ier < 0 )
            {
            fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
            exit(EXIT_FAILURE);
            }

         // Get grid info -- >>
         ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", ">>  ", NULL, &bidon, &bidon, &bidon);
         if ( ier < 0 )
            {
            fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
            exit(EXIT_FAILURE);
            }

         gridAndTopoExtracted = 1;
         fprintf(logfile, "Done");
         }

      // Get surface pressure - P0
      fprintf(logfile, "\tCopying P0... ");
      ier = copyFstField(iin, iout, -1,"            ", -1, -1, -1, "  ", "P0  ", NULL, &bidon, &bidon, &bidon);
      if ( ier < 0 )
         {
         fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
         exit(EXIT_FAILURE);
         }
      fprintf(logfile, "Done\n");

      // Get wind - UU
      fprintf(logfile, "\tCopying UU... ");
      ier = copyFstField(iin, iout, -1,"            ", dynamicsLevelIp1, -1, -1, "  ", "UU  ", NULL, &bidon, &bidon, &bidon);
      if ( ier < 0 )
         {
         fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
         exit(EXIT_FAILURE);
         }
      fprintf(logfile, "Done\n");

      // Get wind - VV
      fprintf(logfile, "\tCopying VV... ");
      ier = copyFstField(iin, iout, -1,"            ", dynamicsLevelIp1, -1, -1, "  ", "VV  ", NULL, &bidon, &bidon, &bidon);
      if ( ier < 0 )
         {
         fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
         exit(EXIT_FAILURE);
         }
      fprintf(logfile, "Done\n");

      fprintf(logfile, "Closing file:%s... ", dynamicsFilename);
      ier = c_fstfrm(iin);
      if ( ier < 0)
          {
          fprintf(stderr, "C_fstfrm failed! file=%s ier=%d (%s:%d)\n", dynamicsFilename, ier, __FILE__, __LINE__);
          exit(EXIT_FAILURE);
          }

      ier = c_fclos(iin);
      if ( ier < 0)
          {
          fprintf(stderr, "C_fclos failed! file=%s ier=%d (%s:%d)\n", dynamicsFilename, ier, __FILE__, __LINE__);
          exit(EXIT_FAILURE);
          }
      fprintf(logfile, "Done\n");

      free(dynamicsFilename);
      }


   ///////
   // Get stuff from the physics file.
   fprintf(logfile, "Opening physics file: %s...", physicsFilename);
   ier = c_fnom(iin, physicsFilename, "STD+RND+R/O", 0);
   if (ier < 0)
      {
      fprintf(stderr, "C_fnom failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstouv(iin, "RND");
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstouv failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   fprintf(logfile, "Done\n");

   // Get the land-sea mask - MG
   fprintf(logfile, "\tCopying MG... ");
   ier = copyFstField(iin, iout, -1,"            ", -1, -1, -1, "  ", "MG  ", NULL, &bidon, &bidon, &bidon);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   fprintf(logfile, "Done\n");

   // Get the roughness - 2b
   fprintf(logfile, "\tCopying 2B... ");
   ier = copyFstField(iin, iout, -1,"            ", -1, -1, -1, "  ", "2B  ", NULL, &refIg1, &refIg2, &refIg3);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   fprintf(logfile, "Done\n");

   // Get grid info - ^^
   fprintf(logfile, "\tCopying ^^... ");
   ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", "^^  ", NULL, &bidon, &bidon, &bidon);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   fprintf(logfile, "Done\n");

   // Get grid info - >>
   fprintf(logfile, "\tCopying >>... ");
   ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", ">>  ", NULL, &bidon, &bidon, &bidon);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   fprintf(logfile, "Done\n");

   fprintf(logfile, "Closing: %s... ", physicsFilename);
   ier = c_fstfrm(iin);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstfrm failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fclos(iin);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fclos failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   fprintf(logfile, "Done\n");

   fprintf(logfile, "Closing: %s... ", uvFilename);
   ier = c_fstfrm(iout);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstfrm failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fclos(iout);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fclos failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   fprintf(logfile, "Done\n");

   fclose(logfile);
   }


void copyGeophyFields(char *sourceFilename, char *destFilename)
   {
   int iin  = 76;
   int iout = 77;

   int refIg1;
   int refIg2;
   int refIg3;

   int ier;
   int bidon;

   // Open source file
   ier = c_fnom(iin, sourceFilename, "STD+RND+R/O", 0);
   if (ier < 0)
      {
      fprintf(stderr, "C_fnom failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstouv(iin, "RND");
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstouv failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   // Destination source file
   ier = c_fnom(iout, destFilename, "STD+RND+R/W", 0);
   if (ier < 0)
      {
      fprintf(stderr, "C_fnom failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstouv(iout, "RND");
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstouv failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }


   // Copy ME and it's ^^, >>
   ier = copyFstField(iin, iout, -1,"            ", -1, -1, -1, "  ", "ME  ", NULL, &refIg1, &refIg2, &refIg3);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", "^^  ", NULL, &bidon, &bidon, &bidon);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", ">>  ", NULL, &bidon, &bidon, &bidon);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }


   // Copy MG and 2B and it's ^^, >>
   ier = copyFstField(iin, iout, -1,"            ", -1, -1, -1, "  ", "MG  ", NULL, &refIg1, &refIg2, &refIg3);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   ier = copyFstField(iin, iout, -1,"            ", -1, -1, -1, "  ", "2B  ", NULL, &refIg1, &refIg2, &refIg3);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", "^^  ", NULL, &bidon, &bidon, &bidon);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", ">>  ", NULL, &bidon, &bidon, &bidon);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstfrm(iin);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstfrm failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fclos(iin);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fclos failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstfrm(iout);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstfrm failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fclos(iout);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fclos failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   }


void copyRoughness(char *sourceFilename, char *destFilename)
   {
   int iin  = 76;
   int iout = 77;

   int refIg1;
   int refIg2;
   int refIg3;

   int ier;
   int bidon;

   // Open source file
   ier = c_fnom(iin, sourceFilename, "STD+RND+R/O", 0);
   if (ier < 0)
      {
      fprintf(stderr, "C_fnom failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstouv(iin, "RND");
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstouv failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   // Destination source file
   ier = c_fnom(iout, destFilename, "STD+RND+R/W", 0);
   if (ier < 0)
      {
      fprintf(stderr, "C_fnom failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstouv(iout, "RND");
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstouv failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }


   // Copy LU and it's ^^, >>
   ier = copyFstField(iin, iout, -1,"            ", -1, -1, -1, "  ", "LU  ", NULL, &refIg1, &refIg2, &refIg3);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", "^^  ", NULL, &bidon, &bidon, &bidon);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   ier = copyFstField(iin, iout, -1,"            ", refIg1, refIg2, refIg3, "  ", ">>  ", NULL, &bidon, &bidon, &bidon);
   if ( ier < 0 )
      {
      fprintf(stderr, "CopyFstField failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstfrm(iin);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstfrm failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fclos(iin);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fclos failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fstfrm(iout);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fstfrm failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }

   ier = c_fclos(iout);
   if ( ier < 0)
      {
      fprintf(stderr, "C_fclos failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      exit(EXIT_FAILURE);
      }
   }








