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
class QStringList;
class QFile;
//class QTextStream;

// KDE
#include <kparts/part.h>
class KAboutApplication;
class KAboutData;
class KConfig;

// local
#include "configurationclasses.h"
#include "report.h"
#include "commandengine.h"
class KFileReplaceView;

#define KFR_VERSION "0.8.0"

class KFileReplacePart: public KParts::ReadOnlyPart
{
  Q_OBJECT

  private: //MEMBERS
    KFileReplaceView* m_view;
    KFileReplaceLib* m_lib;
    QWidget* m_parentWidget,
           * m_w;
    KConfig* m_config;
    QStringList m_recentStringFileList;
    KAboutApplication* m_aboutDlg;
    KeyValueMap m_replacementMap;
    ConfigurationInformation m_info;
    Report m_report;
    CommandEngine m_command;
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
  public slots:
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

    void resetActions();
    void freezeActions();

    void readOptions();
    void saveOptions();

    void loadRulesFile(const QString& fileName);

    void fileReplace();
    void recursiveFileReplace(const QString& dirName);
    void replaceAndBackup(const QString& currentDir, const QString& oldFileName, bool regex, bool simulation);
    void replaceAndOverwrite(const QString& currentDir, const QString& oldFileName, bool regex, bool simulation);
    void replacingLoop(QString& line, KListViewItem** item, bool& atLeastOneStringFound, int& occur, bool regex);


    void fileSearch(const QString& dirName, const QString& filters);
    void recursiveFileSearch(const QString& dirName, const QString& filters);
    void search(const QString& currentDir, const QString& fileName, bool regex);

    void loadViewContent();

    /**
    Launches new project dialog
    */
    void launchNewProjectDialog(const KURL& startURL);

    void setOptionMask();

    void stringsInvert(bool invertAll);

    bool checkBeforeOperation();

    void whatsThis();
};

#endif// KFileReplacePart
