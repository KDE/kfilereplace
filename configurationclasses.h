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
const QString rcDirectoriesList="Directories list";
const QString rcFiltersList="Filters list";
const QString rcRecentFiles="Recent files";
const QString rcAllStringsMustBeFound="All strings must be found";
const QString rcCaseSensitive="Case sensitive";
const QString rcConfirmStrings="Confirm strings";
const QString rcConfirmFiles="Confirm files";
const QString rcConfirmDirs="Confirm directories";
const QString rcFollowSymLinks="Follow symbolic links";
const QString rcHaltOnFirstOccur="Halt on first occurrence";
const QString rcIgnoreHidden="Ignore hidden files";
const QString rcIgnoreWhitespaces="Ignore special characters";
const QString rcRecursive="Search/replace in sub folders";
const QString rcVariables="Enable variables";
const QString rcRegularExpressions="Enable regular expressions";
const QString rcMinFileSize="Minimum file size";
const QString rcMaxFileSize="Maximum file size";
const QString rcValidAccessDate="Access mode";
const QString rcMinDate="Minimum access date";
const QString rcMaxDate="Maximum access date";
const QString rcOwnerUser="Owner user filters";
const QString rcOwnerGroup="Owner group filters";
const QString rcSearchMode="Search only mode";
const QString rcBackupExtension="Backup file extension";
const QString rcIgnoreFiles="Ignore files if there is no match";

// Default configuration options
const bool RecursiveOption = true;
const bool CaseSensitiveOption = false;
const bool FollowSymbolicLinksOption = false;
const bool RegularExpressionsOption = false;
const bool VariablesOption = false;
const bool ConfirmStringsOption = false;
const bool ConfirmFilesOption = false;
const bool ConfirmDirectoriesOption = false;
const bool StopWhenFirstOccurenceOption = false;
const bool IgnoreWhiteSpacesOption = false;
const bool IgnoreHiddenOption = false;
const int FileSizeOption = -1;
const QString AccessDateOption="unknown";
const QString ValidAccessDateOption="unknown";
const QString OwnerOption="false,Name,Equals To,???";
const bool SearchMode=true;
const QString BackupExtensionOption="false,~";
const bool IgnoreFilesOption = true;

// This class store configuration informations
class ConfigurationInformation
{
  private:
    QString m_directory,
            m_filter;
           
    int m_minSize,
        m_maxSize;
       
    QString m_dateAccess,
            m_minDate,
            m_maxDate;
        
    bool m_caseSensitive,
         m_recursive,
         m_followSymLinks,
         m_allStringsMustBeFound,
         m_backup,
         m_regularExpressions;
       
    bool m_variables,
         m_confirmStrings,
         m_confirmFiles,         
         m_confirmDirs,
         m_haltOnFirstOccur,
         m_ignoreWhitespaces,
         m_ignoreHidden,
         m_simulation,
         m_searchMode;
 
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
    
  public:
    ConfigurationInformation& operator=(const ConfigurationInformation& ci);
    
  public:
    void setDirectory(const QString ndir) { m_directory = ndir; }
    QString directory() const { return m_directory; }
    void setFilter(const QString nfilter) { m_filter = nfilter; }
    QString filter() const { return m_filter; }
    
    void setMinSize(int min) { m_minSize = min; }
    int minSize() const { return m_minSize; }
    void setMaxSize(int max) { m_maxSize = max; }
    int maxSize() const { return m_maxSize; }
    
    void setDateAccess(const QString& da) { m_dateAccess = da; }
    QString dateAccess() const { return m_dateAccess; }
    void setMinDate(const QString& min) { m_minDate = min; }
    QString minDate() const { return m_minDate; }
    void setMaxDate(const QString& max) { m_maxDate = max; }
    QString maxDate() const { return m_maxDate; }
    
    void setCaseSensitive(bool cs) { m_caseSensitive = cs; }
    bool caseSensitive() const { return m_caseSensitive; }
    void setRecursive(bool r) { m_recursive = r; }
    bool recursive() const { return m_recursive; }
    void setFollowSymLinks(bool sl) { m_followSymLinks = sl; }
    bool followSymLinks() const { return m_followSymLinks; }
    void setRegularExpressions(bool rx) { m_regularExpressions = rx; }
    bool regularExpressions() const { return m_regularExpressions; }
    void setBackup(bool backup) { m_backup = backup; }
    bool backup() const { return m_backup; }
    void setVariables(bool v) { m_variables = v; }
    bool variables() const { return m_variables; }
    void setConfirmFiles(bool c) { m_confirmFiles = c; } 
    bool confirmFiles() const { return m_confirmFiles; }
    void setConfirmStrings(bool c) { m_confirmStrings = c; } 
    bool confirmStrings() const { return m_confirmStrings; }
    void setConfirmDirs(bool c) { m_confirmDirs = c; } 
    bool confirmDirs() const { return m_confirmDirs; }
    void setHaltOnFirstOccur(bool fo) { m_haltOnFirstOccur = fo; }  
    bool haltOnFirstOccur() const { return m_haltOnFirstOccur; }
    void setIgnoreWhitespaces(bool iw) { m_ignoreWhitespaces = iw; }
    bool ignoreWhitespaces() const { return m_ignoreWhitespaces; }
    void setIgnoreHidden(bool ih) { m_ignoreHidden = ih; }
    bool ignoreHidden() const { return m_ignoreHidden; }
    void setSimulation(bool s) { m_simulation = s; }
    bool simulation() const { return m_simulation; }
    void setSearchMode(bool s) { m_searchMode = s; }
    bool searchMode() const { return m_searchMode; }
    
    void setOwnerUserIsChecked(bool chk) { m_ownerUserIsChecked = chk; }
    bool ownerUserIsChecked() const { return m_ownerUserIsChecked; }
    void setOwnerGroupIsChecked(bool chk) { m_ownerGroupIsChecked = chk; }
    bool ownerGroupIsChecked() const { return m_ownerGroupIsChecked; }   
    void setOwnerUserBool(const QString& user) { m_ownerUserBool = user; }
    QString ownerUserBool() const { return m_ownerUserBool; }
    void setOwnerGroupBool(const QString& group) { m_ownerGroupBool = group; }
    QString ownerGroupBool() const { return m_ownerGroupBool; }
       
    void setOwnerUserType(const QString& user) { m_ownerUserType = user; }
    QString ownerUserType() const { return m_ownerUserType; }
    void setOwnerGroupType(const QString& group) { m_ownerGroupType = group; }
    QString ownerGroupType() const { return m_ownerGroupType; }
    
    void setOwnerUserValue(const QString& user) { m_ownerUserValue = user; }
    QString ownerUserValue() const { return m_ownerUserValue; } 
    void setOwnerGroupValue(const QString& group) { m_ownerGroupValue = group; }
    QString ownerGroupValue() const { return m_ownerGroupValue; }
    
    void setBackupExtension(const QString& bkext) { m_backupExtension = bkext; }
    QString backupExtension() const { return m_backupExtension; }
    
    void setIgnoreFiles(bool ifs) { m_ignoreFiles = ifs; }
    bool ignoreFiles() const { return m_ignoreFiles; }
    
    void setMapStringsView(const KeyValueMap& map) { m_mapStringsView = map; }
    KeyValueMap mapStringsView() const { return m_mapStringsView; }
    
    void setQuickSearchString(const QString& quickSearch) { m_quickSearchString = quickSearch; }
    QString quickSearchString() const { return m_quickSearchString; }
    void setQuickReplaceString(const QString& quickReplace) { m_quickReplaceString = quickReplace; }
    QString quickReplaceString() const { return m_quickReplaceString; } 
       
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
    ResultViewEntry(QString nkey, QString ndata, bool regexp, bool caseSensitive);
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
