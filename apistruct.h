/***************************************************************************
                          apistruct.h  -  description
                             -------------------
    begin                : Sat Sep 25 1999
    copyright            : (C) 1999 by François Dupoux
                                 (C) 2004 by Emiliano Gulmini
    email                : dupoux@dupoux.com
                               emi_barbarossa@yahoo.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef APISTRUCT_H
#define APISTRUCT_H

#include <qdatetime.h>

#include <sys/param.h>

class QWidget;
class QListView;
class KFileReplaceView;
class KResultView;

struct RepDirArg
{
  KResultView *qlvResult;
  QListView *qlvStrings;
  QString szDir;
  QString szFilter;
  bool bMinSize;
  bool bMaxSize;
  uint nMinSize;
  uint nMaxSize;
  int nTypeOfAccess;
  bool bMinDate;
  bool bMaxDate;
  QDate qdMinDate;
  QDate qdMaxDate;
  bool bCaseSensitive;
  bool bRecursive;
  bool bFollowSymLinks;
  bool bAllStringsMustBeFound;
  bool bBackup;
  bool bWildcards;
  char cWildcardsLetters;
  char cWildcardsWords;
  bool bWildcardsInReplaceStrings;
  int nMaxExpressionLength;
  bool bVariables;
  bool bConfirmFiles;
  bool bConfirmStrings;
  bool bConfirmDirs;
  bool bHaltOnFirstOccur;
  bool bIgnoreWhitespaces;
  bool bIgnoreHidden;

  bool bSimulation;

  bool bOwnerUserBool;
  bool bOwnerGroupBool;
  bool bOwnerUserMustBe;
  bool bOwnerGroupMustBe;
  QString strOwnerUserType;
  QString strOwnerGroupType;
  QString strOwnerUserValue;
  QString strOwnerGroupValue;

  QWidget *mainwnd;
  KFileReplaceView *view;
};

struct Settings
{        // General
        bool bCaseSensitive;
        bool bRecursive;
        bool bIgnoreWhitespaces;
        bool bFollowSymLinks;
        bool bAllStringsMustBeFound;
        bool bIgnoreHidden;

        // Wildcards
        bool bWildcards;
        char cWildcardsLetters;
        char cWildcardsWords;
        bool bWildcardsInReplaceStrings;
        int nMaxExpressionLength;

        // Replace
        bool bBackup;
        bool bVariables;
        bool bConfirmDirs;
        bool bConfirmFiles;
        bool bConfirmStrings;
        bool bHaltOnFirstOccur; // Stop on first occur when searching
};

#endif // APISTRUCT_H
