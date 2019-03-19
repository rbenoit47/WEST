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
#ifndef WESTATS_UTILS
#define WESTATS_UTILS

void getDirContent(char ***list, int *listLength, const char *dirname, int fullpath, int getSelfAndParent);

void getMc2DynamicFilenameList(char ***list, int *listLength, const char *modelDirectory, int dynamicsTimeStepNumber);

void getMc2PhysicsFilename(char **filename, const char *modelDirectory, int physicsTimeStepNumber);

void computeUVFile(char **dynamicsFilenameList, int dynamicsFilenameListLength, char *physicsFilename, int dynamicsLevelIp1, char *uvFilename);

void copyGeophyFields(char *sourceFilename, char *destFilename);
#endif
