/***************************************************************************
                          kernel.h  -  description                              
                             -------------------                                         
    begin                : Sat Sep 25 1999                                           
    copyright            : (C) 1999 by François Dupoux                         
    email                : dupoux@dupoux.com                                     
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   * 
 *                                                                         *
 ***************************************************************************/

#ifndef WORKING_H
#define WORKING_H

class QListViewItem;

#define REPLACE_ERROR       -1							
#define REPLACE_SUCCESS     0							
#define REPLACE_SKIPDIR     1							
#define REPLACE_CANCEL      2
#define REPLACE_FILESKIPPED 3
					
void *ReplaceThread(void *param);
void *SearchThread(void *param);
int ReplaceDirectory(const char *szDir, RepDirArg* argu, bool bReplace);
bool IsFileGoodSizeProperties(const char *szFileName, bool bMinSize, bool bMaxSize, uint nMinSize, uint nMaxSize);
bool IsFileGoodDateProperties(const char *szFileName, int nTypeOfAccess, bool bMinDate, bool bMaxDate, QDate qdMinDate, QDate qdMaxDate);
int ReplaceFile(QListViewItem *lvi, const char *szDir, const char *szOldFile, const char *szNewFile, int *nNbReplacements, RepDirArg* argu);
int GetDiskFreeSpaceForFile(uint *nAvailDiskSpace, const char *szFilename);			
int SearchFile(QListViewItem *lvi, const char *szOldFile, int *nNbReplacements, bool *bAllStringsFound, RepDirArg* argu, bool bHaltOnFirstOccur);
bool HasFileGoodOwners(const char *szFile, RepDirArg *argu);

#endif // WORKING_H


