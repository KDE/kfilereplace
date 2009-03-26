/***************************************************************************
                          whatthis.h  -  description
                             -------------------
    begin                : fri 12 march CEST 2004
    copyright            : (C) 2004 Emiliano Gulmini
    email                : <emi_barbarossa@yahoo.it>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef WHATTHIS_H
#define WHATTHIS_H

// QT
#include <qstring.h>

// KDE
#include <klocale.h>

namespace whatthisNameSpace
{
  //KFileReplaceView messages
  const QString lvResultWhatthis = i18n("Shows the statistics of your operations. Note that the columns content changes depending on what kind of operation you are performing.");

  const QString lvStringsWhatthis = i18n("Shows a list of strings to search for (and if you specified it, a list of strings to replace with). Use the \"add strings\" dialog to edit your string list or double click on a string.");

  //KNewProjectDlg messages
  const QString cbLocationWhatthis = i18n("Base folder for operations of search/replace. Insert path string here by hand or use the search button.");

  const QString cbFilterWhatthis = i18n("Shell-like wildcards. Example: \"*.html;*.txt;*.xml\".");

  const QString edSizeMinWhatthis = i18n("Insert the minimum file size you want to search, or leave it unchecked if you do not want a minimum size limit.");

  const QString edSizeMaxWhatthis = i18n("Insert the maximum file size you want to search, or leave it unchecked if you do not want a maximum size limit.");

  const QString edDateMinWhatthis = i18n("Insert the minimum value for file access date that you want to search, or leave it unchecked if you do not want a minimum limit.");

  const QString edDateMaxWhatthis = i18n("Insert the maximum value for file access date that you want to search, or leave it unchecked if you do not want a maximum limit.");

  const QString cbDateValidWhatthis = i18n("Select  \"writing\" if you want to use the date of the last modification, or  \"reading\" to use the date of the last access.");

  const QString chbDateMinWhatthis = i18n("Minimum value for access date.");

  const QString chbDateMaxWhatthis = i18n("Maximum value for access date.");

  const QString leSearchWhatthis = i18n("Insert here the string to search for.");

  const QString leReplaceWhatthis = i18n("Insert here the string to replace with.");

  //KOptionsDlg messages
  const QString chbCaseSensitiveWhatthis = i18n("Enable this option if your search is case sensitive.");

  const QString chbRecursiveWhatthis = i18n("Enable this option to search in sub folders too.");

  const QString chbHaltOnFirstOccurrenceWhatthis = i18n("Enable this option when you are searching for a string and you are only interested to know if the string is present or not in the current file.");

  const QString chbIgnoreWhitespacesWhatthis ="";

  const QString chbFollowSymLinksWhatthis = i18n("If set, then if kfilereplace encounters a symbolic link it treats it like a normal folder or file.");

  const QString chbIgnoreHiddenWhatthis = i18n("Enable this option to ignore hidden files or folders.");

  const QString chbIgnoreFilesWhatthis = i18n("If this option is enabled, KFR will show even the names of the files in which no string has been found or replaced.");

  const QString chbRegularExpressionsWhatthis = i18n("Allows you to apply Qt-like regular expressions on the search string. Note that a complex regular expression could affect speed performance.");

  const QString chbVariablesWhatthis = i18n("Enable \"commands\". For example: if search string is \"user\" and replace string is the command \"[$user:uid$]\", KFR will substitute \"user\" with the uid of the user.");

  const QString chbBackupWhatthis = i18n("Enable this option if you want leave original files untouched.");

  const QString chbConfirmStringsWhatthis = i18n("Enable this option if you want to be asked for single string replacement confirmation.");

  //KFileReplacePart
  const QString fileSimulateWhatthis = i18n("Enable this option to perform replacing as a simulation, i.e. without making any changes to files.");

  const QString optionsRegularExpressionsWhatthis = chbRegularExpressionsWhatthis;

  const QString optionsBackupWhatthis = chbBackupWhatthis;

  const QString optionsCaseWhatthis = chbCaseSensitiveWhatthis;

  const QString optionsVarWhatthis = chbVariablesWhatthis;

  const QString optionsRecursiveWhatthis = chbRecursiveWhatthis;

  //KAddStringDlg
  const QString rbSearchOnlyWhatthis = i18n("Select search-only mode.");

  const QString rbSearchReplaceWhatthis = i18n("Select search-and-replace mode.");

  const QString edSearchWhatthis = i18n("Insert here a string you want search for.");

  const QString edReplaceWhatthis = i18n("Insert here the string that KFR will use to replace the search string.");
}
#endif
