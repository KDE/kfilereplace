/***************************************************************************
                          kernel.h  -  description
                             -------------------
    begin                : Tue 24/02/2004
    copyright            : (C) 1999 by François Dupoux
                           (C) 2003 Andras Mantia <amantia@kde.org>
                           (C) 2004 Emiliano Gulmini <emi_barbarossa@yahoo.it>
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

#ifndef KERNEL_H
#define KERNEL_H

class QListViewItem;

const int replaceError = -1;
const int replaceSuccess = 0;
const int replaceSkipDir = 1;
const int replaceCancel = 2;
const int replaceFileSkipped = 3;

class Kernel
{
 private:
   Kernel(){}
   Kernel(const Kernel& k){}
 public:
   static Kernel* instance() { return (new Kernel); }
   void *replaceThread(void *param);
   void *searchThread(void *param);
   int replaceDirectory(const QString& szDir, RepDirArg* argu, bool bReplace);
   bool isFileGoodSizeProperties(const QString& szFileName, bool bMinSize, bool bMaxSize, uint nMinSize, uint nMaxSize);
   bool isFileGoodDateProperties(const QString& szFileName, int nTypeOfAccess, bool bMinDate, bool bMaxDate, QDate qdMinDate, QDate qdMaxDate);
   int replaceFile(QListViewItem *lvi, const QString& szDir, const QString& szOldFile, const QString& szNewFile, int *nNbReplacements, RepDirArg* argu);
   int diskFreeSpaceForFile(uint *nAvailDiskSpace, const QString &szFilename);
   int searchFile(QListViewItem *lvi, const QString &szOldFile, int *nNbReplacements, bool *bAllStringsFound, RepDirArg* argu, bool bHaltOnFirstOccur);
   bool hasFileGoodOwners(const QString &szFile, RepDirArg *argu);
};

#endif // KERNEL_H


