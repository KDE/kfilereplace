/***************************************************************************
                          resource.h  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
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

#ifndef RESOURCE_H
#define RESOURCE_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

///////////////////////////////////////////////////////////////////
// General application values
#define ID_STATUS_MSG 1001

#define TAB_FILES 0
#define TAB_TEXT 1
#define TAB_DATESIZE 2
#define TAB_OPTIONS 3

#define OPERATION_SEARCH 1
#define OPERATION_REPLACE 2

#define MAX_MESSAGE 128
#define MAX_TEXTLEN 256

const int MaxStringToSearch = 256;
const bool RecursiveOption = true;
const bool BackupOption = true;
const bool CaseSensitiveOption = false;
const bool FollowSymbolicLinksOption = false;
const bool WildcardsOption = false;
const bool VariablesOption = false;
const bool AllStringsMustBeFoundOption = false;
const bool ConfirmFilesOption = false;
const bool ConfirmStringsOption = false;
const bool ConfirmDirectoriesOption = false;
const bool WildcardsInReplaceStringOption = false;
const char WildcardsLetterOption = '?';
const char WildcardsWordOption = '*';
const unsigned int WildcardsExpressionLengthOption = 100;
const bool StopWhenFirstOccurenceOption = false;
const bool IgnoreWhiteSpacesOption = false;
const bool IgnoreHiddenOption = false;

///////////////////////////////////////////////////////////////////
// Create cases for entries and connect them with their functions
#define ON_CMD(id, cmd_class_function) case id:cmd_class_function ;break;
#define ON_STATUS_MSG(id, message) case id:slotStatusHelpMsg(message);break;

///////////////////////////////////////////////////////////////////
//
#if !defined(MIN)
#define MIN(X,Y) (X) < (Y) ? (X) : (Y)
#endif
#if !defined(MAX)
#define MAX(X,Y) (X) > (Y) ? (X) : (Y)
#endif

#endif // RESOURCE_H

