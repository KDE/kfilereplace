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
#include <qstring.h>
namespace whatthisNameSpace 
{
  //KFileReplaceView messages
  const QString lwResultWhatthis = "Show the statistics of your operations. Note that the columns content"
                                   "change depending on what operaton you are doing.";

  const QString lwStringsWhatthis = "Show a list of searching strings (and replacing strings if there are)."
                                    "Use tha \"add strings\" dialog to edit your string list.";

  
  //KNewProjectDlg messages
  const QString cbLocationWhatthis = "Insert a search path here by hand or use the search button.";

  const QString cbFilterWhatthis = "Shell like file searching filters. Example: \"*.html;*.txt;*.xml\".";

  const QString edSizeMinWhatthis = "Insert the minimum file size you want to search, or leave it unchecked if you don't want minimum size limit.";

  const QString edSizeMaxWhatthis = "Insert the maximum file size you want to search, or leave it unchecked if you don't want maximum size limit.";

  const QString edDateMinWhatthis = "Insert the minimum file access date you want to search, or leave it unchecked if you don't want minimum file access date limit.";

  const QString edDateMaxWhatthis = "Insert the maximum file access date you want to search, or leave it unchecked if you don't want maximum file access date limit.";

  const QString cbDateValidWhatthis = "Select \"Writing\" if you want to use the date of the last modification, or \"reading\" to use the the date of the last access."
                                     "read access to the file";
  const QString chbDateMinWhatthis = "Minimum access date.";
  const QString chbDateMaxWhatthis = "Maximum access date.";
  
  //KOptionsDlg messages
  const QString chbCaseSensitiveWhatthis = "Enable this option if your search is case sensitive.";

  const QString chbRecursiveWhatthis ="Enable this option to search in sub folders too.";

  const QString chbHaltOnFirstOccurrenceWhatthis = "Enable this option when you are searching for a string and you are only interested to know if the string is present or not in the current file.";

  const QString chbIgnoreWhitespacesWhatthis = "";

  const QString chbFollowSymLinksWhatthis = "If kfilereplace encounters a symbolic link treats it like a normal directory or file.";

  const QString chbIgnoreHiddenWhatthis = "Enable this option to ignore hidden files or directories.";

  const QString chbWildcardsWhatthis = "Enable shell-like wildcards ";
  const QString chbVariablesWhatthis = "Enable \"variables\". For example: if replace string is \"[$user:id$]\", we get user id.";

  const QString chbBackupWhatthis = "Enable this option to create a \".OLD\" file instead of overwriting the current one.";


  //KFileReplacePart
  const QString fileSimulateWhatthis = "Enable this option to perform the same operation as the replace one, without make any changes in files.";

  const QString optionsWildcardsWhatthis = chbWildcardsWhatthis;

  const QString optionsBackupWhatthis = chbBackupWhatthis;

  const QString optionsCaseWhatthis = chbCaseSensitiveWhatthis;

  const QString optionsVarWhatthis = chbVariablesWhatthis;

  const QString optionsRecursiveWhatthis = chbRecursiveWhatthis;
}  
#endif
