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

class QWidget;
class QListView;
class KFileReplaceView;
class KResultView;

struct RepDirArg
{
  KResultView *qlvResult;
  QListView *qlvStrings;
  QString szDir,
          szFilter;
  bool bMinSize,
       bMaxSize;
  uint nMinSize,
       nMaxSize;
  int nTypeOfAccess;
  bool bMinDate,
       bMaxDate;
  QDate qdMinDate,
        qdMaxDate;
  bool bCaseSensitive,
       bRecursive,
       bFollowSymLinks,
       bAllStringsMustBeFound,
       bBackup,
       bWildcards;
  char cWildcardsLetters,
       cWildcardsWords;
  bool bWildcardsInReplaceStrings;
  int nMaxExpressionLength;
  bool bVariables,
       bConfirmFiles,
       bConfirmStrings,
       bConfirmDirs,
       bHaltOnFirstOccur,
       bIgnoreWhitespaces,
       bIgnoreHidden,
       bSimulation,
       bOwnerUserBool,
       bOwnerGroupBool,
       bOwnerUserMustBe,
       bOwnerGroupMustBe;
  QString strOwnerUserType,
          strOwnerGroupType,
          strOwnerUserValue,
          strOwnerGroupValue;

  QWidget *mainwnd;
  KFileReplaceView *view;
};

struct Settings
{ // General
  bool bCaseSensitive,
       bRecursive,
       bIgnoreWhitespaces,
       bFollowSymLinks,
       bAllStringsMustBeFound,
       bIgnoreHidden;

  // Wildcards
  bool bWildcards;      
  char cWildcardsLetters,
       cWildcardsWords;
  bool bWildcardsInReplaceStrings;
  int nMaxExpressionLength;

  // Replace
  bool bBackup,
       bVariables,
       bConfirmDirs,
       bConfirmFiles,
       bConfirmStrings,
       bHaltOnFirstOccur; // Stop on first occur when searching
};

#endif // APISTRUCT_H
