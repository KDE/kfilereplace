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
//qt includes

//kde includes
#include <kparts/part.h>
#include <kuser.h>

#include "configurationclasses.h"

#define KFR_VERSION "0.8"

class QStringList;
class KAboutData;
class KConfig;

class KAboutApplication;
class KFileReplaceView;

class KFileReplacePart: public KParts::ReadOnlyPart
{
  Q_OBJECT

  private:
    KFileReplaceView* m_view;

    QWidget* m_parentWidget;
    /** the configuration object of the application */
    KConfig* m_config;
    /** a list of recently accessed files */
    QStringList m_recentStringFileList;

    KAboutApplication* m_dlgAbout;

    QMap<QString,QString> m_replacementMap;
    ConfigurationInformation m_info;
    bool m_stop;
    int m_optionMask;

  public:
    KFileReplacePart(QWidget *parentWidget,
                     const char *widgetName,
                     QObject *parent,
                     const char *name,
                     const QStringList &args);
    virtual ~KFileReplacePart();

  public slots:
    void slotFileNew();
    void slotFileSearch();
    void slotFileReplace();
    void slotFileSimulate();
    void slotFileStop();
    void slotFileSave();
    void slotStringsAdd();
    void slotStringsAddFromProjectDlg(const QMap<QString,QString>& replacementMap);
    void slotStringsDel();
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
    void slotOptionsWildcards();
    void slotOptionsPreferences();
    void slotAboutApplication();
    void appHelpActivated();
    void reportBug();

  public:
    static KAboutData* createAboutData();
    KConfig *config();

  protected:
    virtual bool openFile() { return false; }
    virtual bool openURL (const KURL &url);

    void initView();
    void initGUI();
    void loadStringFile(const QString& strFilename);
    void readOptions();
    void saveOptions();
    void resetActions();
    void freezeActions();
    bool checkBeforeOperation();
    bool verifyFileRequirements(const QString& filePath,const QString& fileName);
    void setWhatsThis();

  private:
    void convertOldToNewKFRFormat(const QString& strFilename);

    void normalFileReplace();
    void recursiveFileReplace(const QString& dirName);
    void replaceAndBackupExpression(const QString& currentDir, const QString& oldFileName);
    void replaceAndOverwriteExpression(const QString& currentDir, const QString& oldFileName,bool simulation=false);
    void replaceAndBackupLiteral(const QString& currentDir, const QString& oldFileName);
    void replaceAndOverwriteLiteral(const QString& currentDir, const QString& oldFileName,bool simulation=false);

    void normalFileSearch(const QString& dirName, const QString& filters);
    void recursiveFileSearch(const QString& dirName, const QString& filters);
    void searchExpression(const QString& currentDir, const QString& oldFileName);
    void searchLiteral(const QString& currentDir, const QString& oldFileName);
    void loadInformationFromView();
    void launchNewProjectDialog();
    void setOptionMask();

    QString variableValue(const QString& variable);

};

#endif
