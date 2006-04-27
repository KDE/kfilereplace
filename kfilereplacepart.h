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

#define KFR_VERSION "0.8.1"

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
    RCOptions* m_option;
    bool m_stop,
         m_searchingOperation;
    int m_optionMask;

  public://Constructors
    KFileReplacePart(QWidget *parentWidget,
                     const char *widgetName,
                     QObject *parent,
                     const char *name,
                     const QStringList &args);
    ~KFileReplacePart();

  //SLOTS
  private slots:
    void slotSetNewParameters();
    void slotSearchingOperation();
    void slotReplacingOperation();
    void slotSimulatingOperation();
    void slotStop();
    void slotCreateReport();
    void slotStringsAdd();
    void slotQuickStringsAdd();
    void slotStringsDeleteItem();
    void slotStringsEmpty();
    void slotStringsEdit();
    void slotStringsSave();
    void slotStringsLoad();
    void slotStringsInvertCur();
    void slotStringsInvertAll();
    void slotOpenRecentStringFile(const KUrl& urlFile);
    void slotOptionRecursive();
    void slotOptionBackup();
    void slotOptionCaseSensitive();
    void slotOptionVariables();
    void slotOptionRegularExpressions();
    void slotOptionPreferences();
    void showAboutApplication(void);
    void appHelpActivated();
    void reportBug();
    void resetActions();
    void slotSearchMode(bool b){ m_option->m_searchingOnlyMode = b; }

  //METHODS
  public:
    static KAboutData* createAboutData();
    KConfig *config(){ return m_config; }

  protected:
    virtual bool openFile() { return false; }
    virtual bool openURL (const KUrl &url);

  private:
    /**
     * Initialization methods
     */
    void initGUI();
    void initView();

    /**
     * This method freezes the GUI of kfilereplace during searching
     */
    void freezeActions();

    /**
     * These methods load the resources from kfilereplacerc
     */
    void loadOptionsFromRC();
    void loadOptions();
    void loadFileSizeOptions();
    void loadDateAccessOptions();
    void loadOwnerOptions();
    void loadLocationsList();
    void loadFiltersList();
    void loadBackupExtensionOptions();

    /**
     * These methods save the resources to kfilereplacerc
     */
    void saveOptionsToRC();
    void saveOptions();
    void saveFileSizeOptions();
    void saveDateAccessOptions();
    void saveOwnerOptions();
    void saveLocationsList();
    void saveFiltersList();
    void saveBackupExtensionOptions();

    /**
     * Replacing methods
     */
    void fileReplace();
    void recursiveFileReplace(const QString& dirName, int& filesNumber);
    void replaceAndBackup(const QString& currentDir, const QString& oldFileName);
    void replaceAndOverwrite(const QString& currentDir, const QString& oldFileName);
    void replacingLoop(QString& line, K3ListViewItem** item, bool& atLeastOneStringFound, int& occur, bool regularExpression, bool& askConfirmReplace);

    /**
     * Searching methods
     */
    void fileSearch(const QString& dirName, const QString& filters);
    void recursiveFileSearch(const QString& dirName, const QString& filters, uint& filesNumber);
    void search(const QString& currentDir, const QString& fileName);

    /**
     * Others methods
     */
    void loadViewContent();
    void loadRulesFile(const QString& fileName);
    bool launchNewProjectDialog(const KUrl& startURL);
    void setOptionMask();
    bool checkBeforeOperation();
    bool dontAskAgain();
    void whatsThis();
};

#endif// KFileReplacePart
