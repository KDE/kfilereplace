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

//kde includes
#include <kparts/part.h>

//app includes
#include "apistruct.h"

class QStringList;
class QDateTime;
class KAboutData;
class KConfig;

class KAboutKFileReplace;
class KFileReplaceDoc;
class KFileReplaceView;
class KResultView;

// Global Thread data
extern bool g_bThreadRunning;
extern bool g_bThreadMustStop;
extern int g_nFilesRep;
extern RepDirArg g_argu;
extern QString g_szErrMsg;
extern int g_nStringsRep;
extern int g_nOperation;

class KFileReplacePart: public KParts::ReadOnlyPart
{
  Q_OBJECT

public:
  KFileReplacePart(QWidget *parentWidget, const char *widgetName,
                   QObject *parent, const char *name, const QStringList &args);
  virtual ~KFileReplacePart();

  static KAboutData* createAboutData();
  KFileReplaceDoc* document() const;
  KConfig *config();
  KResultView *resultView();

public slots:
  void slotFileNew();
  void slotFileSearch();
  void slotFileReplace();
  void slotFileSimulate();
  void slotFileStop();
  void slotFileSave();
  void slotStringsAdd();
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
  void slotOptionsWildcards();
  void slotOptionsVariables();
  void slotOptionsPreferences();
  void showAboutApplication(void);
  void appHelpActivated();
  void reportBug();


protected:
  virtual bool openFile() {return false;}
  virtual bool openURL (const KURL &url);

  void initDocument();
  void initView();
  void initGUI();
  void loadStringFile(const QString& strFilename);
  void readOptions();
  void saveOptions();
  void updateCommands();
  int  checkBeforeOperation(int nTypeOfOperation);
  void setWhatsThis();
private:

  KFileReplaceView* m_view;
  KFileReplaceDoc* m_doc;
  QWidget* m_parentWidget;
  /** the configuration object of the application */
  KConfig* m_config;
  Settings m_settings;
  QStringList m_recentStringFileList; // a list of recently accessed files
  KAboutKFileReplace* m_dlgAbout;

private:
  void convertOldToNewKFRFormat(const QString& strFilename,KFileReplaceView* view);
};

#endif
