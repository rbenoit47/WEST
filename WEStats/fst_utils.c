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
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "fst_utils.h"


/*
 Note: est valide pour les floats seulement?!?!
*/
int copyFstField(int iin, int iout, int argDatev, char *argEtiket, int argIp1, int argIp2, int argIp3, char *argTypvar, char *argNomvar, char *argNewEtiket, int *outIg1, int *outIg2, int *outIg3)
   {
   int ier;
   int key;

   int ni, nj, nk;
   int ip1, ip2, ip3, ig1, ig2, ig3, ig4;
   int  dateo, datev, datyp, deet,nbits, npak, npas,swa, lng;
   int dltf, ubc, extra1, extra2, extra3;
   char etiket[16], nomvar[8], typvar[4], grtyp[2];
   void *work = NULL;
   float *data = NULL;

   strcpy(etiket, "            ");
   strcpy(nomvar, "    ");
   strcpy(typvar, "  ");
   strcpy(grtyp, " ");

   key = c_fstinf(iin, &ni, &nj, &nk, argDatev, argEtiket, argIp1, argIp2, argIp3, argTypvar, argNomvar);
   if ( key < 0 )
      {
      fprintf(stderr, "c_fstinf failed! ier=%d (%s:%d)\n", key, __FILE__, __LINE__);
      return key;
      }

   ier = c_fstprm(key, &dateo, &deet, &npas, &ni, &nj, &nk, &nbits,
                  &datyp, &ip1, &ip2, &ip3, typvar, nomvar, etiket,
                  grtyp, &ig1, &ig2, &ig3, &ig4, &swa, &lng, &dltf,
                  &ubc, &extra1, &extra2, &extra3);
   if ( ier < 0 )
      {
      fprintf(stderr, "c_fstprm failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      return ier;
      }

   data = (float *) malloc(ni*nj*nk*sizeof(float));
   if ( data == NULL )
      {
      fprintf(stderr, "Malloc failed! (%s:%d)\n", __FILE__, __LINE__);
      return -24;
      }

   ier = c_fstluk(data, key, &ni, &nj, &nk);

   if ( ier < 0 )
      {
      fprintf(stderr, "c_fstluk failed! ier=%d (%s:%d)\n", ier, __FILE__, __LINE__);
      return ier;
      }

   nbits = -32;

   if (argNewEtiket != NULL)
      strncpy(etiket, argNewEtiket, 16);
   ier = c_fstecr(data, work, nbits, iout, dateo, deet, npas, ni, nj, nk, ip1, ip2, ip3, typvar, nomvar, etiket, grtyp, ig1, ig2, ig3, ig4, datyp, "false");

   free(data);

   *outIg1 =ig1;
   *outIg2 =ig2;
   *outIg3 =ig3;
   return ier;
   }

