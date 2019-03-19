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
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>

#define MAX_CAR 256

int mymain(int argc, char **argv)
   {
   char **dynamicFilenamelist = NULL;
   int    dynamicFilenamelistLength = 0;
   char  *physicsFilename = NULL;
   char   tmpUVFilename[MAX_CAR+1] = "";
   FILE  *tmpUVFile = NULL;
   FILE  *fBidon = NULL;

   int i;
   int ier;
   
   char  argNamelistFilename[MAX_CAR+1] = {0};

   char  nmlModelDirectory[MAX_CAR+1] = {0};
   int   nmlDynamicsTimeStepNumber;
   int   nmlDynamicsLevelIP1;
   char  nmlMesoGeophyFile[MAX_CAR+1] = {0};
   char  nmlWindClimateTable[MAX_CAR+1] = {0};
   int   nmlAveragingLength;
   float nmlStatsLevel;
   int   nmlMaxWindSpeed;
   int   nmlDelete_intermediate_files;
   char  nmlStatsFile[MAX_CAR+1] = {0};

   // Read the namelist file passed as an argument.
   if ( argc != 2 )
      {
      fprintf(stderr, "Wrong number of arguments!\n");
      fprintf(stderr, "Usage:\n");
      fprintf(stderr, "\tex: %s namelistFilename\n", argv[0]);
      exit(EXIT_FAILURE);
      }
   strcpy(argNamelistFilename, argv[1]);
   read_namelist_(argNamelistFilename, nmlModelDirectory, &nmlDynamicsTimeStepNumber, &nmlDynamicsLevelIP1,
                  nmlMesoGeophyFile, nmlWindClimateTable, &nmlAveragingLength, &nmlStatsLevel,
                  &nmlMaxWindSpeed, &nmlDelete_intermediate_files, nmlStatsFile);

//   trimPathname(nmlModelDirectory);
//   trimPathname(nmlMesoGeophyFile);
//   trimPathname(nmlWindClimateTable);
//   trimPathname(nmlStatsFile);

   // Get dynamics files and physic file. And make some verifications.
   getMc2DynamicFilenameList(&dynamicFilenamelist, &dynamicFilenamelistLength, nmlModelDirectory, nmlDynamicsTimeStepNumber);
   getMc2PhysicsFilename(&physicsFilename, nmlModelDirectory, 0);

   if ( dynamicFilenamelistLength == 0 )
      {
      fprintf(stderr, "No Mc2 dynamics files found in %s!\n", nmlModelDirectory);
      exit(EXIT_FAILURE);
      }
   if ( physicsFilename == NULL )
      {
      fprintf(stderr, "No Mc2 physic file found in !\n", nmlModelDirectory);
      exit(EXIT_FAILURE);
      }

   // Create a temporary file containing UU, VV, P0 field from dynamics files
   // and ME?, MG 2B from physic file
   strcpy(tmpUVFilename, nmlStatsFile);
   tmpUVFilename[strlen(tmpUVFilename)-4] = '\0';
   sprintf(tmpUVFilename, "%s_tmpUV.fst", tmpUVFilename);

   tmpUVFile = fopen(tmpUVFilename, "r");
   if ( tmpUVFile )
      {
      // Delete the file if it exist
      fprintf(stderr, "Warning! Removing %s\n", tmpUVFilename);
      fclose(tmpUVFile);
      unlink(tmpUVFilename);
      }
   computeUVFile(dynamicFilenamelist, dynamicFilenamelistLength, physicsFilename, nmlDynamicsLevelIP1, tmpUVFilename);

   // Make sure that we start with a clean stats files (output file).
   fBidon = fopen(nmlStatsFile, "r");
   if ( fBidon )
      {
      // Delete the file if it exist
      fprintf(stderr, "Warning! Removing %s\n", nmlStatsFile);
      fclose(fBidon);
      unlink(nmlStatsFile);
      }

   //////
   // Run WEStats!
   char region[] = "nn  ";
   int etats = dynamicFilenamelistLength;
   char tmpRoseFilename[MAX_CAR+1] = "";
   char logFilename[MAX_CAR+1] = "";

   sprintf(tmpRoseFilename, "tmpRose_%d.fst", getpid());

   // Create the log filename and delete it if it already exists
   strcpy(logFilename, nmlStatsFile);
   if ( !strcmp(logFilename+strlen(logFilename)-4, ".fst") )
      {
      strcpy(logFilename+strlen(logFilename)-4, ".log");
      }
   else
      {
      strcat(logFilename, ".log");
      }
   fBidon = fopen(logFilename, "r");
   if ( fBidon )
      {
      // Delete the file if it exist
      fprintf(stderr, "Warning! Removing %s\n", logFilename);
      fclose(fBidon);
      unlink(logFilename);
      }
   // The logfile must already exist...
   fBidon = fopen(logFilename, "w");
   fclose(fBidon);

   // Copy some extra fields
   ier = stats_we_(tmpUVFilename, nmlStatsFile, nmlWindClimateTable, tmpRoseFilename,
                   region,  &etats,
                   &nmlDynamicsLevelIP1, &nmlStatsLevel, &nmlAveragingLength,
                   &nmlMaxWindSpeed, &nmlDynamicsTimeStepNumber,
                   logFilename);

   copyGeophyFields(tmpUVFilename, nmlStatsFile);
   copyRoughness(nmlMesoGeophyFile, nmlStatsFile);


   // Do some cleanup
   for ( i=0 ; i<dynamicFilenamelistLength ; i++)
      free(dynamicFilenamelist[i]);
   free(dynamicFilenamelist);
   free(physicsFilename);
   if ( nmlDelete_intermediate_files )
      {     
      unlink(tmpUVFilename);
      unlink(tmpRoseFilename);
      }

   return EXIT_SUCCESS;
   }

