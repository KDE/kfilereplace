/***************************************************************************
                          knewprojectdlg.h  -  description
                             -------------------
    begin                : Tue Dec 28 1999
    copyright            : (C) 1999 by François Dupoux
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

#ifndef KNEWPROJECTDLG_H
#define KNEWPROJECTDLG_H

//KDE 
class KConfig;

//local
#include "knewprojectdlgs.h"
#include "configurationclasses.h"

class KNewProjectDlg : public KNewProjectDlgS
{
  Q_OBJECT
  
  public:
    ConfigurationInformation m_info;
    
  private:
    KConfig *m_config;
    QString m_searchNowFlag;
      
  public:
    KNewProjectDlg(QWidget *parent, KConfig *config, const char *name=0);
    ~KNewProjectDlg();
    
  public:
    void loadOptions();
    void loadFileSizeFilter();
    void loadDateAccessFilter();
    void loadOwnerFilter();
    void loadLocationsList();
    void loadFiltersList();
    void loadBackupExtensionFilter();
    QString currentDir() const; 
    QString currentFilter() const;
    QString quickSearchString() const;
    QString quickReplaceString() const;
    void saveOptions();
    void saveFileSizeFilter();
    void saveDateAccessFilter();
    void saveOwnerFilter();
    void saveLocationsList();
    void saveFiltersList();
    void saveBackupExtensionFilter();
    void setDatas(const QString& directoryString, const QString& filterString);
    void whatsThis();
     
  protected slots:
    void slotDir();
    void slotOK();
    void slotReject();
    void slotSearchNow();
    void slotSearchLater();
    void slotSearchLineEdit(const QString& t);
    void slotEnableSpinboxSizeMin(bool b);
    void slotEnableSpinboxSizeMax(bool b);
    void slotEnableCbValidDate(bool b);
    void slotEnableChbUser(bool b);
    void slotEnableChbGroup(bool b); 
    void slotEnableChbBackup(bool b);
    void slotHelp();
    
  private:
    bool contains(QListView* lv,const QString& s, int column);
};

#endif  // KNewProjectDlg
