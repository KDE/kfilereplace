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

  private:
    QString m_searchNowFlag;
    RCOptions* m_option;

  public:
    KNewProjectDlg(RCOptions* info, QWidget *parent=0, const char *name=0);
    ~KNewProjectDlg();

  public:
    //void updateOptions(const RCOptions& info);
    void saveRCOptions();

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
    void slotHelp(){ kapp->invokeHelp(QString::null, "kfilereplace"); }

  private:
    void initGUI();

    void loadOptions();
    void loadFileSizeOptions();
    void loadDateAccessOptions();
    void loadOwnerOptions();
    void loadLocationsList();
    void loadFiltersList();
    void loadBackupExtensionOptions();

    void saveOptions();
    void saveFileSizeOptions();
    void saveDateAccessOptions();
    void saveOwnerOptions();
    void saveLocationsList();
    void saveFiltersList();
    void saveBackupExtensionOptions();

    bool contains(QListView* lv,const QString& s, int column);
    void setDatas(const QString& directoryString, const QString& filterString);
    void whatsThis();
};

#endif  // KNewProjectDlg
