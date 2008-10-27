/***************************************************************************
                          configurationclasses.h  -  description
                             -------------------
    begin                : Sat Apr 17 2004
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
#ifndef CONFIGURATIONCLASSES_H
#define CONFIGURATIONCLASSES_H

// QT
#include <qstring.h>
#include <qstringlist.h>
#include <qdatetime.h>
#include <qmap.h>
#include <qregexp.h>

typedef QMap<QString,QString> KeyValueMap;
// entry strings in the kfilereplacerc file
const QString rcDirectoriesList = "Directories list";
const QString rcFiltersList = "Filters list";
const QString rcRecentFiles = "Recent files";
const QString rcAllStringsMustBeFound = "All strings must be found";
const char rcEncoding[] = "Encoding";
const QString rcCaseSensitive = "Case sensitive";
const QString rcConfirmStrings = "Confirm strings";
const QString rcConfirmFiles = "Confirm files";
const QString rcConfirmDirs = "Confirm directories";
const QString rcFollowSymLinks = "Follow symbolic links";
const QString rcHaltOnFirstOccur = "Halt on first occurrence";
const QString rcIgnoreHidden = "Ignore hidden files";
const QString rcRecursive = "Search/replace in sub folders";
const QString rcVariables = "Enable variables";
const QString rcRegularExpressions = "Enable regular expressions";
const QString rcMinFileSize = "Minimum file size";
const QString rcMaxFileSize = "Maximum file size";
const QString rcValidAccessDate = "Access mode";
const QString rcMinDate = "Minimum access date";
const QString rcMaxDate = "Maximum access date";
const QString rcOwnerUser = "Owner user filters";
const QString rcOwnerGroup = "Owner group filters";
const QString rcSearchMode = "Search only mode";
const QString rcBackupExtension = "Backup file extension";
const QString rcIgnoreFiles = "Ignore files if there is no match";
const QString rcNotifyOnErrors = "NotifyOnErrors";
const QString rcAskConfirmReplace = "Ask confirmation on replace";
const QString rcDontAskAgain = "Don't ask again";
// Default configuration options
const QString EncodingOption = "utf8";
const bool RecursiveOption = true;
const bool CaseSensitiveOption = false;
const bool FollowSymbolicLinksOption = false;
const bool RegularExpressionsOption = false;
const bool VariablesOption = false;
const bool StopWhenFirstOccurenceOption = false;
const bool IgnoreHiddenOption = false;
const int FileSizeOption = -1;
const QString AccessDateOption="unknown";
const QString ValidAccessDateOption="unknown";
const QString OwnerOption="false,Name,Equals To";
const bool SearchModeOption=true;
const QString BackupExtensionOption="false,~";
const bool IgnoreFilesOption = true;
const bool NotifyOnErrorsOption = false;
const bool AskConfirmReplaceOption = false;

// This class stores configuration information
class RCOptions
{
  public:
    bool m_callResetActions;

    bool m_askConfirmReplace,
         m_dontAskAgain;

    QString m_directories,
            m_filters,
	    m_currentDirectory;

    int m_minSize,
        m_maxSize;

    QString m_dateAccess,
            m_minDate,
            m_maxDate;
            
    QString m_encoding;

    bool m_caseSensitive,
         m_recursive,
         m_followSymLinks,
         m_allStringsMustBeFound,
         m_backup,
         m_regularExpressions;

    bool m_variables,
         m_haltOnFirstOccur,
         m_ignoreHidden,
         m_simulation,
         m_searchingOnlyMode;

    bool m_ownerUserIsChecked,
         m_ownerGroupIsChecked;

    QString m_ownerUserType,
            m_ownerGroupType,
            m_ownerUserValue,
            m_ownerGroupValue,
            m_ownerUserBool,
            m_ownerGroupBool;

    QString m_backupExtension;

    bool m_ignoreFiles;

    KeyValueMap m_mapStringsView;

    QString m_quickSearchString,
            m_quickReplaceString;

    QStringList m_recentStringFileList;

    bool m_notifyOnErrors;

  public:
    RCOptions();
    RCOptions& operator=(const RCOptions& ci);
};

class ResultViewEntry
{
  private:
    QString m_key;
    QString m_data;
    QRegExp m_rxKey;
    bool m_regexp;
    bool m_caseSensitive;
    int m_pos;
    int m_matchedStringsOccurrence;

  public:
    ResultViewEntry(const QString &nkey, const QString &ndata, bool regexp, bool caseSensitive);
    int lineNumber(const QString& line) const ;
    int columnNumber(const QString& line) const ;
    void incOccurrences();
    int occurrences() const ;
    bool regexp()const ;
    int pos(const QString& line) ;
    void incPos();
    QString capturedText(const QString& line)  ;
    QString message(const QString& capturedText, int x, int y) const;
    int keyLength() const;
    int dataLength() const;
    void updateLine(QString& line);
};
#endif
