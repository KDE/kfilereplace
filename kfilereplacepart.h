//
//
// C++ Interface: kfilereplacepart
//
// Description:
//
//
// Author: Andras Mantia <amantia@kde.org>, (C) 2003
// Maintainer: Emiliano Gulmini <emi_barbarossa@yahoo.it>, (C) 2004
//
// Copyright: GPL v2. See COPYING file that comes with this distribution
//
//

#ifndef KFILEREPLACEPART_H
#define KFILEREPLACEPART_H

// QT

// KDE
#include <kparts/part.h>
class KAboutApplication;
class KAboutData;
class KConfig;

// local
#include "configurationclasses.h"
class KFileReplaceView;

#define KFR_VERSION "0.8.0"

class KFileReplacePart: public KParts::ReadOnlyPart
{
  Q_OBJECT

  private: //MEMBERS
    KFileReplaceView* m_view;
    QWidget* m_parentWidget,
           * m_w;
    KConfig* m_config;
    KAboutApplication* m_aboutDlg;
    KeyValueMap m_replacementMap;
    RCOptions m_option;
    bool m_stop,
         m_searchingOperation;
    int m_optionMask;

  //CONTRUCTORS
  public:
    KFileReplacePart(QWidget *parentWidget,
                     const char *widgetName,
                     QObject *parent,
                     const char *name,
                     const QStringList &args);
    ~KFileReplacePart();

  //SLOTS
  protected slots:
    void slotFileNew();
    void slotFileSearch();
    void slotFileReplace();
    void slotFileSimulate();
    void slotFileStop();
    void slotFileSave();
    void slotStringsAdd();
    void slotQuickStringsAdd();
    void slotStringsDeleteItem();
    void slotStringsEmpty();
    void slotStringsEdit();
    void slotStringsSave();
    void slotStringsLoad();
    void slotStringsInvertCur();
    void slotStringsInvertAll();
    void slotOpenRecentStringFile(const KURL& urlFile);
    void slotOptionsRecursive();
    void slotOptionsBackup();
    void slotOptionsCaseSensitive();
    void slotOptionsVariables();
    void slotOptionsRegularExpressions();
    void slotOptionsPreferences();
    void showAboutApplication(void);
    void appHelpActivated();
    void reportBug();
    void resetActions();

  private slots:
    /**
     *this method is used to retrieve searchMode option from m_view
     */
    void searchMode(bool b){ m_option.setSearchMode(b); }

  //METHODS
  public:
    static KAboutData* createAboutData();
    KConfig *config();

  protected:
    virtual bool openFile() { return false; }
    virtual bool openURL (const KURL &url);

  private:
    void initView();
    void initGUI();

    void freezeActions();

    void loadOptionsFromRC();
    void saveOptionsToRC();

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

    void fileReplace();
    void recursiveFileReplace(const QString& dirName);
    void replaceAndBackup(const QString& currentDir, const QString& oldFileName);
    void replaceAndOverwrite(const QString& currentDir, const QString& oldFileName);
    void replacingLoop(QString& line, KListViewItem** item, bool& atLeastOneStringFound, int& occur, bool regex);


    void fileSearch(const QString& dirName, const QString& filters);
    void recursiveFileSearch(const QString& dirName, const QString& filters);
    void search(const QString& currentDir, const QString& fileName);

    void loadViewContent();

    void loadRulesFile(const QString& fileName);

    /**
    Launches new project dialog
    */
    bool launchNewProjectDialog(const KURL& startURL);

    void setOptionMask();

    void stringsInvert(bool invertAll);

    bool checkBeforeOperation();

    void whatsThis();
};

#endif// KFileReplacePart
