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
  const QString lwResultWhatthis = "The list which shows all the results of the last operation. You can "
                                   "see all the files which were successfully found/replaced in green, and files where there was an "
                                   "error in red. You can see in the <b>Result</b> column, the number of strings found/replaced for "
                                   "each file. In blue, there is the general string which was found, and in yellow, if you are using "
                                   "wildcards or variables, you can see the real text which was found from a string with wildcards";

  const QString lwStringsWhatthis = "In the strings list, the user must add all the strings which must "
                                    "be searched and replaced inside the files. If you are searching, you can add no string in this list. Then "
                                    "all files of the directory, will be shown if the filter is valid (*.htm* for example). This list is "
                                    "managed by the <b>Strings</b> menu. Before making any replace, and sometimes before the search, you need to "
                                    "add string in this list. You can use the <b>+</b> icon of the toolbar to do it. You can delete a string, "
                                    "empty the list, save the strings of the list into a file, and load them from a file. This last option is "
                                    "very useful when you makes the same search/replace operation all the time, and you need to reuse the same "
                                    "strings. You can use wildcards and variables options in the strings. Please, read help for more details.";

  
  //KNewProjectDlg messages
  const QString cbLocationWhatthis = "This is the directory where the search or the replace operation will be done.";

  const QString cbFilterWhatthis = "Fix the filter of the files you want to search. For example, write \"*.htm\" to search or replace "
                                   "all web pages. You can put more than an only filter, when using ';'. For example \"*.html;*.txt;*.xml\"";

  const QString edSizeMinWhatthis = "Minimal size of files. For example, if you put 1024 KB, all files which size is less than 1 MB will not be taken";

  const QString edSizeMaxWhatthis = "Maximal size of files. For example, if you put 2048 KB, all files which size is more than 2 MB will not be taken";

  const QString edDateMinWhatthis = "Minimal date of files in YYYY/MM/DD format. For example, if you put 2000/01/31, all files which were "
                                   "(modified/read) before the 31 January 2000 will not be taken";

  const QString edDateMaxWhatthis = "Maximal date of files in YYYY/MM/DD format. For example, if you put 1999/12/31, all files which were "
                                   "(modified/read) after the 31 December 1999 will not be taken";

  const QString cbDateValidWhatthis = "Select \"Writing\" if you want to use the date of the last modification, or \"reading\" to use the last "
                                     "read access to the file";
  
  //KOptionsDlg messages
  const QString chbCaseSensitiveWhatthis = "If enabled, lowers and uppers are different. If you are "
                                         "searching for \"Linux\" and this option is on, \"LINUX\" will not be found.";

  const QString chbRecursiveWhatthis ="Work in all subfolders of the main folder of the project";

  const QString chbHaltOnFirstOccurrenceWhatthis = "When searching, KFileReplace reads all the data of each "
                                                 "file to know how many times each strings appears in the files. If you need not"
                                                 "to have this details, you can enable this options to make the search faster. "
                                                 "Then the search will be finished in a file when the first occurrence of a string will be found.";

  const QString chbIgnoreWhitespacesWhatthis = "Ignore some characters when searching a string: \\r (carriage return), \\n (line feed) \\t (tab), "
                                             "and multi-spaces. It's very "
                                             "useful in HTML code, and with the end of the lines. For example, if you are searching for "
                                             "\"Linux is fast\" in a file, and there is a line feed between "
                                             "\"Linux\" and \"is\", then the string will not be found. This options can solve the problem.";

  const QString chbFollowSymLinksWhatthis = "If a folder symbolic link is found, it will be opened.";

  const QString chbAllStringsMustBeFoundWhatthis = "The file will be found/replaced only if all strings from the list are found in the file.";

  const QString chbIgnoreHiddenWhatthis = "If this option is enabled, hidden files and folders (files whose names begin with a "
                                        "point as .kde) will not be found / replaced.";

  const QString edLengthWhatthis = "This is the maximal length of the search made to find wildcards in a text. For example, if the length is 200, then "
                                   "the '*' symbol cannot code for an expression which length is more than 200 letters (bytes)";

  const QString chbWildcardsInReplaceStrWhatthis = "If enabled, the contents of the wildcards found in the search string will be copied into the "
                                                   "replace string. For example, if you search for \"The * is under my bed\" and the text is \"The "
                                                   "cat is under my bed\", and the replace string "
                                                   "is \"I have a *\", then the text will be replaced with \"I have a cat\". If this options is "
                                                   "disabled, you will have \"I have a *\".";

  const QString chbVariablesWhatthis = "If enabled, KFileReplace will replace variables with their values in the replace string. For example "
                                       "if the replace string is \"The current time is [$datetime:mm/dd/yyyy$]\", then the date will be written.";

  const QString chbBackupWhatthis = "If enabled, create backup of replaced files before any modifications. Then you can restore the old data "
                                    "if there is an error during the replace operation. A copy of the original files will be created, with the .OLD extension.";


  //KFileReplacePart
  const QString fileSimulateWhatthis = "The same operation as the replace one, but do not make any changes in files. "
                                       "This is not a simple search, because you will see the exact changes that could "
                                       "be done (with regexp or variables for example.)";

  const QString optionsWildcardsWhatthis = "Enable use of the wildcards (* for expressions, and ? for single character if not modified "
                                           "in the wildcards options). For example, you can search for <b>KMsgBox::message(*)</b> "
                                           "and replace with <b>KMessageBox::error(*)</b>";

  const QString optionsBackupWhatthis = "Create a copy of the original replaced files with the BAK extension before replacing";

  const QString optionsCaseWhatthis = "The lowers and uppers are different. For example, if you search for <b>Linux</b> and "
                                      "there is <b>linux</b>, then the string will not be found/replaced.";

  const QString optionsVarWhatthis = "Enable use of the variables, as the date & time or the name of the current file. "
                                     "The variable must be in the replace string, with the format [$Name:  $]. When doing the replace, "
                                     "the value of the variable "
                                     "is written instead of the variable. If disabled, the replace string will be copied without "
                                     "substitution. Please, read help for "
                                     "more details on the variables names and formats.";

  const QString optionsRecursiveWhatthis = "Search/Replace files in the subfolders of the main folder selected "
                                           "when creating a new project at startup.";
}  
#endif