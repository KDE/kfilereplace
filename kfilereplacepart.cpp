//
//
// C++ Implementation: kfilereplacepart
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

//#undef APP

// QT
#include <qdir.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qimage.h>

// KDE
#include <kaboutapplication.h>
#include <kapplication.h>
#include <kaction.h>
#include <kbugreport.h>
#include <kconfig.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kparts/genericfactory.h>
#include <kstandarddirs.h>
#include <kiconloader.h>
#include <kglobal.h>
#include <kiconeffect.h>
#include <klistview.h>
#include <kuser.h>
#include <kcombobox.h>

// local
#include "kfilereplacelib.h"
#include "kfilereplacepart.h"
#include "kfilereplaceview.h"
#include "koptionsdlg.h"
#include "knewprojectdlg.h"
#include "whatthis.h"

using namespace whatthisNameSpace;

//PUBLIC CONSTRUCTORS
// Factory code for KDE 3
typedef KParts::GenericFactory<KFileReplacePart> FileReplaceFactory;
K_EXPORT_COMPONENT_FACTORY( libkfilereplacepart, FileReplaceFactory )

KFileReplacePart::KFileReplacePart(QWidget* parentWidget, const char* , QObject* parent, const char* name, const QStringList & )
  : KParts::ReadOnlyPart(parent,name)
{
  setInstance(FileReplaceFactory::instance());

  m_parentWidget = parentWidget;
  QString configName = locateLocal("config", "kfilereplacerc");

  m_lib = new KFileReplaceLib();
  m_config = new KConfig(configName);
  m_aboutDlg = 0;
  m_stop = false;
  m_optionMask = QDir::Files;
  m_w = widget();

  initView();
  initGUI();
  whatsThis();
}

KFileReplacePart::~KFileReplacePart()
{
  m_stop = true;
  m_view = 0L; //it's already deleted, so set it to NULL

  saveOptions();

  delete m_aboutDlg;
  m_aboutDlg = 0;
  delete m_config;
  m_config = 0;
  delete m_lib;
  m_lib = 0;
  delete m_w;
  m_w = 0;
}

//PUBLIC SLOTS
void KFileReplacePart::slotFileNew()
{
  m_view->resultView()->clear();
  m_config->reparseConfiguration();
  launchNewProjectDialog(KURL());
  emit setStatusBarText(i18n("Ready."));
}

void KFileReplacePart::slotFileSearch()
{
   if(!checkBeforeOperation())
     return;

   emit setStatusBarText(i18n("Searching files..."));
   m_view->resultView()->setColumnText(4,i18n("Total number occurrences"));

   // show wait cursor
   QApplication::setOverrideCursor( Qt::waitCursor );

   freezeActions();

   setOptionMask();

   m_view->resultView()->setSorting(-1);

   if(m_info.recursive())
     recursiveFileSearch(m_info.directory(),m_info.filter());
   else
     fileSearch(m_info.directory(),m_info.filter());

   //disabling and enabling sorting... don't ask me why, but it works!
   m_view->resultView()->setSorting(0);
   m_view->resultView()->sort();
   m_view->resultView()->setSorting(-1);

   // restore false status for stop button
   m_stop = false;

   QApplication::restoreOverrideCursor();

   emit setStatusBarText(i18n("Search completed."));
   m_info.setSearchMode(true);
   resetActions();
   m_searchingOperation = true;
}

void KFileReplacePart::slotFileReplace()
{
  if(!checkBeforeOperation())
    return;

  if(m_info.simulation())
  {
    emit setStatusBarText(i18n("Replacing files (simulation)..."));
    m_view->resultView()->setColumnText(4,i18n("Replaced strings (simulation)"));
  }
  else
  {
    emit setStatusBarText(i18n("Replacing files..."));
    m_view->resultView()->setColumnText(4,i18n("Replaced strings"));
  }
  // show wait cursor
  QApplication::setOverrideCursor( Qt::waitCursor );

  freezeActions();

  setOptionMask();

  m_view->resultView()->setSorting(-1);

  if(m_info.recursive())
    recursiveFileReplace(m_info.directory());
  else
    fileReplace();

  m_view->resultView()->setSorting(0);
  m_view->resultView()->sort();
  m_view->resultView()->setSorting(-1);

  // restore false status for stop button
  m_stop = false;

  QApplication::restoreOverrideCursor();

  resetActions();
  m_searchingOperation = false;
}

void KFileReplacePart::slotFileSimulate()
{
  m_info.setSimulation(true);
  slotFileReplace();
  m_info.setSimulation(false);
}

void KFileReplacePart::slotFileStop()
{
  emit setStatusBarText(i18n("Stop."));
  m_stop = true;
  QApplication::restoreOverrideCursor();
  resetActions();
}

void KFileReplacePart::slotFileSave()
{
  KListView* lvResult = m_view->resultView();

  // Check there are results
  if(lvResult->firstChild() == 0)
    {
      KMessageBox::error(m_w, i18n("There are no results to save: the result list is empty."));
      return ;
    }
  // Select the file where results will be saved
  QString docName = KFileDialog::getSaveFileName(QString::null, "*.xml|XML " + i18n("Files") + " (*.xml)", m_w, i18n("Save Report"));
  if (docName.isEmpty())
    return ;

  // delete a spourious extension
  docName.truncate(docName.length()-4);

  QFileInfo fileInfo(docName);
  if(fileInfo.exists())
    {
      KMessageBox::error(m_w, i18n("A directory or a file with this name already exists."));
      return ;
    }

  QDir dirName;

  if(!dirName.mkdir(docName, true))
    {
      KMessageBox::error(m_w, i18n("Cannot create %1 directory.").arg(docName));
      return ;
    }

  dirName.cd(docName);

  QString docPath = docName+"/"+dirName.dirName();

  m_report.createDocument(docPath, m_view->stringView(), m_view->resultView(), m_searchingOperation);

  resetActions();
}

void KFileReplacePart::slotStringsAdd()
{
  m_view->slotStringsAdd();

  m_config->setGroup("General Options");
  m_info.setSearchMode(m_config->readBoolEntry(rcSearchMode,SearchMode));

  resetActions();
}

void KFileReplacePart::slotQuickStringsAdd()
{
  //this slot handles a pair of strings that come from project dialog,
  //if the control character 'N' is found at the position 0 of the two strings,
  //then we start the search now.
  QString qs = m_info.quickSearchString();
  QStringList stringsPair;

  stringsPair.append(qs.left(1));
  stringsPair.append(qs.right(qs.length()-1));
  //in this case there is no string to search for so return
  if(stringsPair[1].isEmpty())
    return;

  //in this one instead, we must serach for astring
  qs = m_info.quickReplaceString();
  stringsPair.append(qs.left(1));
  stringsPair.append(qs.right(qs.length()-1));

  m_view->slotQuickStringsAdd(stringsPair[1],stringsPair[3]);

  m_config->setGroup("General Options");
  m_info.setSearchMode(m_config->readBoolEntry(rcSearchMode,SearchMode));

  //resetActions();
  //if search-only mode == true and search-now mode == true then
  //search string
  if(stringsPair[0] == "N")
    if(m_info.searchMode())
      slotFileSearch();
    else
      slotFileReplace();
}

void KFileReplacePart::slotStringsDeleteItem()
{
  m_view->slotStringsDeleteItem();
  resetActions();
}

void KFileReplacePart::slotStringsEmpty()
{
  m_view->slotStringsEmpty();
  resetActions();
}

void KFileReplacePart::slotStringsEdit()
{
  m_view->slotStringsEdit(0);
  m_config->setGroup("General Options");
  m_info.setSearchMode(m_config->readBoolEntry(rcSearchMode,SearchMode));

  resetActions();
}

void KFileReplacePart::slotStringsSave()
{
  // Check there are strings in the list
  if (m_view->stringView()->firstChild() == 0)
    {
      KMessageBox::error(m_w, i18n("There are no strings to save in the list."));
      return ;
    }

   QString header("<?xml version=\"1.0\" ?>\n<kfr>"),
           footer("\n</kfr>"),
           body;
   QListViewItem*  lvi = m_view->stringView()->firstChild();

   while( lvi )
     {
       body += QString("\n\t<replacement>"
                       "\n\t\t<oldstring><![CDATA[%1]]></oldstring>"
                       "\n\t\t<newstring><![CDATA[%2]]></newstring>"
                       "\n\t</replacement>").arg(lvi->text(0)).arg(lvi->text(1));
       lvi = lvi->nextSibling();
     }

   // Selects the file where strings will be saved
   QString menu = "*.kfr|" + i18n("KFileReplace Strings") + " (*.kfr)\n*|" + i18n("All Files") + " (*)";
   QString fileName = KFileDialog::getSaveFileName(QString::null, menu, m_w, i18n("Save Strings to File"));
   if (fileName.isEmpty())
     return;

   // Forces the extension to be "kfr" == KFileReplace extension
   fileName = m_lib->addExtension(fileName, "kfr");

   QFile file( fileName );
   if(!file.open( IO_WriteOnly ))
     {
       KMessageBox::error(m_w, i18n("File %1 cannot be saved.").arg(fileName));
       return ;
     }
   QTextStream oTStream( &file );
   oTStream << header
            << body
            << footer;
   file.close();
}

void KFileReplacePart::slotStringsLoad()
{
  // Selects the file to load from
  QString menu = "*.kfr|" + i18n("KFileReplace strings") + " (*.kfr)\n*|"+i18n("All Files") + " (*)";
  QString fileName = KFileDialog::getOpenFileName(QString::null, menu, m_w, i18n("Load Strings From File"));

  if(!fileName.isEmpty())
    loadRulesFile(fileName);

  resetActions();
}

void KFileReplacePart::slotStringsInvertCur()
{
  stringsInvert(false);
}

void KFileReplacePart::slotStringsInvertAll()
{
  stringsInvert(true);
}

void KFileReplacePart::slotOpenRecentStringFile(const KURL& urlFile)
{
  QString fileName;

  // Downloads file if need (if url is "http://...")
  if (!(KIO::NetAccess::download(urlFile, fileName, 0)))
    return;

  // Checks it's not a directory
  QFileInfo fileInfo;
  fileInfo.setFile(fileName);
  if(fileInfo.isDir())
    {
      KMessageBox::error(m_w, i18n("Cannot open folders."));
      return;
    }

  loadRulesFile(fileName);
  resetActions();
}

void KFileReplacePart::slotOptionsRecursive()
{
  m_info.setRecursive(!(m_info.recursive()));
  m_config->setGroup("Options");
  m_config->writeEntry(rcRecursive, m_info.recursive());
  m_config->sync();
  resetActions();
}

void KFileReplacePart::slotOptionsBackup()
{
  m_info.setBackup(!(m_info.backup()));
  m_config->setGroup("Options");

  QString bkOptions;
  if(m_info.backup())
    bkOptions = "true,"+m_info.backupExtension();
  else
    bkOptions = "false,"+m_info.backupExtension();

  m_config->writeEntry(rcBackupExtension, bkOptions);
  m_config->sync();

  resetActions();
}

void KFileReplacePart::slotOptionsCaseSensitive()
{
  m_info.setCaseSensitive(!(m_info.caseSensitive()));
  m_config->setGroup("Options");
  m_config->writeEntry(rcCaseSensitive, m_info.caseSensitive());
  m_config->sync();
  resetActions();
}

void KFileReplacePart::slotOptionsVariables()
{
  m_info.setVariables(!(m_info.variables()));
  m_config->setGroup("Options");
  m_config->writeEntry(rcVariables, m_info.variables());
  m_config->sync();
  resetActions();
}

void KFileReplacePart::slotOptionsRegularExpressions()
{
  m_info.setRegularExpressions(!(m_info.regularExpressions()));
  m_config->setGroup("Options");
  m_config->writeEntry(rcRegularExpressions, m_info.regularExpressions());
  m_config->sync();
  resetActions();
}

void KFileReplacePart::slotOptionsPreferences()
{
  KOptionsDlg dlg(m_w, 0);

  if(!dlg.exec())
    return;
  m_config->reparseConfiguration();
  readOptions();
  resetActions();
}

void KFileReplacePart::reportBug()
{
  KBugReport dlg(m_w, true, createAboutData());
  dlg.exec();
}

void KFileReplacePart::appHelpActivated()
{
  kapp->invokeHelp(QString::null, "kfilereplace");
}

void KFileReplacePart::showAboutApplication()
{
  m_aboutDlg = new KAboutApplication(createAboutData(), 0, false);
  if(m_aboutDlg == 0)
    return;

  if(!m_aboutDlg->isVisible())
    m_aboutDlg->show();
  else
    m_aboutDlg->raise();
}

//PUBLIC METHODS
KAboutData* KFileReplacePart::createAboutData()
{
  KAboutData * aboutData = new KAboutData( "kfilereplacepart",
                                              I18N_NOOP("KFileReplacePart"),
                                              KFR_VERSION,
                                              I18N_NOOP( "Batch search and replace tool."),
                                              KAboutData::License_GPL_V2,
                                              "(C) 1999-2002 Francois Dupoux\n(C) 2003-2004 Andras Mantia\n(C) 2004 Emiliano Gulmini", I18N_NOOP("Part of the KDEWebDev module."), "http://www.kdewebdev.org");
  aboutData->addAuthor("Francois Dupou",
                          I18N_NOOP("Original author of the KFileReplace tool"),
                          "dupoux@dupoux.com");
  aboutData->addAuthor("Emiliano Gulmini", I18N_NOOP("Current maintainer, code cleaner and rewriter"),"emi_barbarossa@yahoo.it");
  aboutData->addAuthor("Andras Mantia",
                          I18N_NOOP("Co-maintainer, KPart creator"),
                          "amantia@kde.org");
  aboutData->addCredit("Heiko Goller", I18N_NOOP("Original german translator"), "heiko.goller@tuebingen.mpg.de");
  return aboutData;
}

KConfig* KFileReplacePart::config()
{
  return m_config;
}

//PROTECTED METHODS
bool KFileReplacePart::openURL(const KURL &url)
{
  if (url.protocol() != "file")
  {
    KMessageBox::sorry(m_w, i18n("Sorry, currently the KFileReplace part works only for local files."), i18n("Non Local File"));
    emit canceled("");
    return false;
  }
  launchNewProjectDialog(url);
  return true;
}

void KFileReplacePart::initGUI()
{
  setXMLFile("kfilereplacepartui.rc");

  actionCollection()->setHighlightingEnabled(true);
  // File
  (void)new KAction(i18n("New Search Project..."), "projectopen", 0, this, SLOT(slotFileNew()), actionCollection(), "new_project");
  (void)new KAction(i18n("&Search"), "filesearch", 0, this, SLOT(slotFileSearch()), actionCollection(), "search");
  (void)new KAction(i18n("S&imulate"), "filesimulate", 0, this, SLOT(slotFileSimulate()), actionCollection(), "file_simulate");
  (void)new KAction(i18n("&Replace"), "filereplace", 0, this, SLOT(slotFileReplace()), actionCollection(), "replace");
  (void)new KAction(i18n("Sto&p"), "stop", 0, this, SLOT(slotFileStop()), actionCollection(), "stop");
  (void)new KAction(i18n("Cre&ate Report File..."), "filesave", 0, this, SLOT(slotFileSave()), actionCollection(), "save_results");

  // Strings
  (void)new KAction(i18n("&Add String..."), "editadd", 0, this, SLOT(slotStringsAdd()), actionCollection(), "strings_add");

  (void)new KAction(i18n("&Delete String"), "editremove", 0, this, SLOT(slotStringsDeleteItem()), actionCollection(), "strings_del");
  (void)new KAction(i18n("&Empty Strings List"), "eraser", 0, this, SLOT(slotStringsEmpty()), actionCollection(), "strings_empty");
  (void)new KAction(i18n("Edit Selected String..."), "editclear", 0, this, SLOT(slotStringsEdit()), actionCollection(), "strings_edit");
  (void)new KAction(i18n("&Save Strings List to File..."), "filesave", 0, this, SLOT(slotStringsSave()), actionCollection(), "strings_save");
  (void)new KAction(i18n("&Load Strings List From File..."), "unsortedList", 0, this, SLOT(slotStringsLoad()), actionCollection(), "strings_load");
  (void)new KRecentFilesAction(i18n("&Load Recent Strings Files..."), "fileopen", 0, this, SLOT(slotOpenRecentStringFile(const KURL&)), actionCollection(),"strings_load_recent");
  (void)new KAction(i18n("&Invert Current String (search <--> replace)"), "invert", 0, this, SLOT(slotStringsInvertCur()), actionCollection(), "strings_invert");
  (void)new KAction(i18n("&Invert All Strings (search <--> replace)"), "invert", 0, this, SLOT(slotStringsInvertAll()), actionCollection(), "strings_invert_all");

  // Options
  (void)new KToggleAction(i18n("&Include Sub-Folders"), "recursive_option", 0, this, SLOT(slotOptionsRecursive()), actionCollection(), "options_recursive");
  (void)new KToggleAction(i18n("Create &Backup"), "backup_option", 0, this, SLOT(slotOptionsBackup()), actionCollection(), "options_backup");
  (void)new KToggleAction(i18n("Case &Sensitive"), "casesensitive_option", 0, this, SLOT(slotOptionsCaseSensitive()), actionCollection(), "options_case");
  (void)new KToggleAction(i18n("Enable &Variables in Replace String: [$command:option$]"), "command_option", 0, this, SLOT(slotOptionsVariables()), actionCollection(), "options_var");
  (void)new KToggleAction(i18n("Enable &Regular expressions"), "regularexpression_option", 0, this, SLOT(slotOptionsRegularExpressions()), actionCollection(), "options_regularexpressions");
  (void)new KAction(i18n("Configure &KFileReplace..."), "configure", 0, this, SLOT(slotOptionsPreferences()), actionCollection(), "configure_kfilereplace");

  // Results
  (void)new KAction(i18n("&Properties"), "informations", 0, m_view, SLOT(slotResultProperties()), actionCollection(), "results_infos");
  (void)new KAction(i18n("&Open"), "filenew", 0, m_view, SLOT(slotResultOpen()), actionCollection(), "results_openfile");
  (void)new KAction(i18n("&Open in Quanta"), "edit", 0, m_view, SLOT(slotResultEdit()), actionCollection(), "results_editfile");
  (void)new KAction(i18n("Open Parent &Folder"), "fileopen", 0, m_view, SLOT(slotResultDirOpen()), actionCollection(), "results_opendir");
  (void)new KAction(i18n("&Delete"), "eraser", 0, m_view, SLOT(slotResultDelete()), actionCollection(), "results_delete");
  (void)new KAction(i18n("E&xpand Tree"), 0, m_view, SLOT(slotResultTreeExpand()), actionCollection(), "results_treeexpand");
  (void)new KAction(i18n("&Reduce Tree"), 0, m_view, SLOT(slotResultTreeReduce()), actionCollection(), "results_treereduce");
  (void)new KAction(i18n("&About KFileReplace"), "kfilereplace", 0, this, SLOT(showAboutApplication()), actionCollection(), "help_about_kfilereplace");
  (void)new KAction(i18n("KFileReplace &Handbook"), "help", 0, this, SLOT(appHelpActivated()), actionCollection(), "help_kfilereplace");
  (void)new KAction(i18n("&Report Bug..."), 0, 0, this, SLOT(reportBug()), actionCollection(), "report_bug");
}

void KFileReplacePart::initView()
{
  m_view = new KFileReplaceView(m_parentWidget, "view");

  setWidget(m_view);

  m_view->setAcceptDrops(false);

  m_view->setConfig(m_config);
}

void KFileReplacePart::loadRulesFile(const QString& fileName)
{
  //loads kfr file
  QDomDocument doc("mydocument");
  QFile file(fileName);
  if(!file.open(IO_ReadOnly))
    {
      KMessageBox::error(m_w, i18n("<qt>Cannot open the file <b>%1</b> and load the string list.</qt>").arg(fileName));
      return ;
    }
  if(!doc.setContent(&file))
    {
      file.close();
      KMessageBox::information(m_w, i18n("<qt>File <b>%1</b> seems not to be written in new kfr format. Remember that old kfr format will be soon abandoned! You can convert your old rules files by simply saving them with kfilereplace.</qt>").arg(fileName),i18n("Warning"));
      m_lib->convertOldToNewKFRFormat(fileName,m_view->stringView());
      return;
    }

  file.close();

  //clears view
  m_view->stringView()->clear();

  QDomElement docElem = doc.documentElement();
  QDomNode n = docElem.firstChild();
  KeyValueMap docMap;
  while(!n.isNull())
    {
      QDomElement e = n.toElement(); // tries to convert the node to an element.
      if(!e.isNull())
        {
          QString oldString = e.firstChild().toElement().text(),
                  newString = e.lastChild().toElement().text();
                  docMap[oldString] = newString;
        }
      n = n.nextSibling();
    }


     // Adds file to "load strings form file" menu
  if(!m_recentStringFileList.contains(fileName))
    {
      m_recentStringFileList.append(fileName);
      ((KRecentFilesAction* ) actionCollection()->action("strings_load_recent"))->setItems(m_recentStringFileList);
    }
  m_view->loadMap(docMap);
  m_config->setGroup("General Options");
  m_info.setSearchMode(m_config->readBoolEntry(rcSearchMode,SearchMode));

  resetActions();
}

void KFileReplacePart::readOptions()
{
  m_config->setGroup("General Options");

  // Recent files
  m_recentStringFileList = m_config->readListEntry(rcRecentFiles);
  ((KRecentFilesAction* ) actionCollection()->action("strings_load_recent"))->setItems(m_recentStringFileList);

  m_info.setSearchMode(m_config->readBoolEntry(rcSearchMode,SearchMode));

  // options seetings (recursivity, backup, case sensitive)
  m_config->setGroup("Options");

  m_info.setRecursive(m_config->readBoolEntry(rcRecursive, RecursiveOption));
  QStringList bkList = QStringList::split(",",
                                          m_config->readEntry(rcBackupExtension, BackupExtensionOption),
                                          true);
  if(bkList[0] == "true")
    m_info.setBackup(true);
  else
    m_info.setBackup(false);

  m_info.setBackupExtension(bkList[1]);

  m_info.setCaseSensitive(m_config->readBoolEntry(rcCaseSensitive, CaseSensitiveOption));
  m_info.setVariables(m_config->readBoolEntry(rcVariables, VariablesOption));
  m_info.setRegularExpressions(m_config->readBoolEntry(rcRegularExpressions, RegularExpressionsOption));
  m_info.setFollowSymLinks(m_config->readBoolEntry(rcFollowSymLinks, FollowSymbolicLinksOption));

  m_info.setConfirmFiles(m_config->readBoolEntry(rcConfirmFiles, ConfirmFilesOption));
  m_info.setConfirmStrings(m_config->readBoolEntry(rcConfirmStrings, ConfirmStringsOption));
  m_info.setConfirmDirs(m_config->readBoolEntry(rcConfirmDirs, ConfirmDirectoriesOption));
  m_info.setHaltOnFirstOccur(m_config->readBoolEntry(rcHaltOnFirstOccur, StopWhenFirstOccurenceOption));
  m_info.setIgnoreWhitespaces(m_config->readBoolEntry(rcIgnoreWhitespaces, IgnoreWhiteSpacesOption));
  m_info.setIgnoreHidden(m_config->readBoolEntry(rcIgnoreHidden, IgnoreHiddenOption));
  m_info.setIgnoreFiles(m_config->readBoolEntry(rcIgnoreFiles, IgnoreFilesOption));
  m_info.setMinSize(m_config->readNumEntry(rcMinFileSize, FileSizeOption));
  m_info.setMaxSize(m_config->readNumEntry(rcMaxFileSize, FileSizeOption));

  m_info.setDateAccess(m_config->readEntry(rcValidAccessDate, ValidAccessDateOption));
  m_info.setMinDate(m_config->readEntry(rcMinDate, AccessDateOption));
  m_info.setMaxDate(m_config->readEntry(rcMaxDate, AccessDateOption));

  QStringList ownerList = QStringList::split(",",m_config->readEntry(rcOwnerUser, OwnerOption),true);
  if(ownerList[0] == "true")
    m_info.setOwnerUserIsChecked(true);
  else
    m_info.setOwnerUserIsChecked(false);

  m_info.setOwnerUserType(ownerList[1]);
  m_info.setOwnerUserBool(ownerList[2]);
  if(ownerList[3] == "???")
    m_info.setOwnerUserValue("");
  else
    m_info.setOwnerUserValue(ownerList[3]);

  ownerList = QStringList::split(",",m_config->readEntry(rcOwnerGroup, OwnerOption),true);

  if(ownerList[0] == "true")
    m_info.setOwnerGroupIsChecked(true);
  else
    m_info.setOwnerGroupIsChecked(false);

  m_info.setOwnerGroupType(ownerList[1]);
  m_info.setOwnerGroupBool(ownerList[2]);
  if(ownerList[3] == "???")
    m_info.setOwnerGroupValue("");
  else
    m_info.setOwnerGroupValue(ownerList[3]);

}

void KFileReplacePart::saveOptions()
{
  m_config->setGroup("General Options");

  m_config->writeEntry(rcRecentFiles, m_recentStringFileList);
  m_config->writeEntry(rcSearchMode,m_info.searchMode());

  m_config->setGroup("Options");

  m_config->writeEntry(rcRecursive, m_info.recursive());

  QString bkOptions;
  if(m_info.backup())
    bkOptions = "true,"+m_info.backupExtension();
  else
    bkOptions = "false,"+m_info.backupExtension();

  m_config->writeEntry(rcBackupExtension, bkOptions);

  m_config->writeEntry(rcCaseSensitive, m_info.caseSensitive());
  m_config->writeEntry(rcVariables, m_info.variables());
  m_config->writeEntry(rcRegularExpressions, m_info.regularExpressions());

  m_config->writeEntry(rcFollowSymLinks, m_info.followSymLinks());

  m_config->writeEntry(rcConfirmFiles, m_info.confirmFiles());
  m_config->writeEntry(rcConfirmStrings, m_info.confirmStrings());
  m_config->writeEntry(rcConfirmDirs, m_info.confirmDirs());
  m_config->writeEntry(rcHaltOnFirstOccur, m_info.haltOnFirstOccur());
  m_config->writeEntry(rcIgnoreWhitespaces, m_info.ignoreWhitespaces());
  m_config->writeEntry(rcIgnoreHidden, m_info.ignoreHidden());
  m_config->writeEntry(rcIgnoreFiles, m_info.ignoreFiles());

  m_config->sync();
}

void KFileReplacePart::resetActions()
{
  bool hasItems = false,
       searchOnly = m_info.searchMode();

  if(m_view->stringView()->firstChild() != 0)
    hasItems = true;

  // File
  actionCollection()->action("new_project")->setEnabled(true);
  actionCollection()->action("search")->setEnabled(hasItems && searchOnly);

  actionCollection()->action("file_simulate")->setEnabled(hasItems && !searchOnly);
  actionCollection()->action("replace")->setEnabled(hasItems && !searchOnly);
  actionCollection()->action("save_results")->setEnabled(hasItems);
  actionCollection()->action("stop")->setEnabled(false);

  // Strings
  actionCollection()->action("strings_add")->setEnabled(true);
  actionCollection()->action("strings_del")->setEnabled(hasItems );
  actionCollection()->action("strings_empty")->setEnabled(hasItems);
  actionCollection()->action("strings_edit")->setEnabled(hasItems);
  actionCollection()->action("strings_save")->setEnabled(hasItems);
  actionCollection()->action("strings_load")->setEnabled(true);
  actionCollection()->action("strings_invert")->setEnabled(hasItems);
  actionCollection()->action("strings_invert_all")->setEnabled(hasItems);

  // Options
  actionCollection()->action("options_recursive")->setEnabled(true);
  actionCollection()->action("options_backup")->setEnabled(true);
  actionCollection()->action("options_case")->setEnabled(true);
  actionCollection()->action("options_var")->setEnabled(true);
  actionCollection()->action("options_regularexpressions")->setEnabled(true);
  actionCollection()->action("configure_kfilereplace")->setEnabled(true);

  if(m_view->resultView()->firstChild() != 0)
    hasItems = true;
  else
    hasItems = false;

  // Results
  actionCollection()->action("results_infos")->setEnabled(hasItems);
  actionCollection()->action("results_openfile")->setEnabled(hasItems);
  actionCollection()->action("results_editfile")->setEnabled(hasItems);
  actionCollection()->action("results_opendir")->setEnabled(hasItems);
  actionCollection()->action("results_delete")->setEnabled(hasItems);
  actionCollection()->action("results_treeexpand")->setEnabled(hasItems);
  actionCollection()->action("results_treereduce")->setEnabled(hasItems);

  // Updates menus and toolbar
  ((KToggleAction* ) actionCollection()->action("options_recursive"))->setChecked(m_info.recursive());
  ((KToggleAction* ) actionCollection()->action("options_backup"))->setChecked(m_info.backup());
  ((KToggleAction* ) actionCollection()->action("options_case"))->setChecked(m_info.caseSensitive());
  ((KToggleAction* ) actionCollection()->action("options_var"))->setChecked(m_info.variables());
  ((KToggleAction* ) actionCollection()->action("options_regularexpressions"))->setChecked(m_info.regularExpressions());
}

void KFileReplacePart::freezeActions()
{
  //Disables action during search/replace operation
  actionCollection()->action("new_project")->setEnabled(false);
  actionCollection()->action("stop")->setEnabled(true);

  actionCollection()->action("file_simulate")->setEnabled(false);
  actionCollection()->action("replace")->setEnabled(false);
  actionCollection()->action("search")->setEnabled(false);
  actionCollection()->action("strings_add")->setEnabled(false);
  actionCollection()->action("strings_del")->setEnabled(false);
  actionCollection()->action("strings_empty")->setEnabled(false);
  actionCollection()->action("strings_edit")->setEnabled(false);
  actionCollection()->action("strings_save")->setEnabled(false);
  actionCollection()->action("strings_load")->setEnabled(false);
  actionCollection()->action("strings_invert")->setEnabled(false);
  actionCollection()->action("strings_invert_all")->setEnabled(false);
  actionCollection()->action("options_recursive")->setEnabled(false);
  actionCollection()->action("options_backup")->setEnabled(false);
  actionCollection()->action("options_case")->setEnabled(false);
  actionCollection()->action("options_var")->setEnabled(false);
  actionCollection()->action("options_regularexpressions")->setEnabled(false);
  actionCollection()->action("configure_kfilereplace")->setEnabled(false);
  actionCollection()->action("results_infos")->setEnabled(false);
  actionCollection()->action("results_openfile")->setEnabled(false);
  actionCollection()->action("results_editfile")->setEnabled(false);
  actionCollection()->action("results_opendir")->setEnabled(false);
  actionCollection()->action("results_delete")->setEnabled(false);
  actionCollection()->action("results_treeexpand")->setEnabled(false);
  actionCollection()->action("results_treereduce")->setEnabled(false);
  ((KToggleAction* ) actionCollection()->action("options_recursive"))->setChecked(false);
  ((KToggleAction* ) actionCollection()->action("options_backup"))->setChecked(false);
  ((KToggleAction* ) actionCollection()->action("options_case"))->setChecked(false);
  ((KToggleAction* ) actionCollection()->action("options_var"))->setChecked(false);
  ((KToggleAction* ) actionCollection()->action("options_regularexpressions"))->setChecked(false);
}

bool KFileReplacePart::checkBeforeOperation()
{
  readOptions();
  loadViewContent();

  // Checks if there are strings to replace (not need in search operation)
  if (m_view->stringView()->childCount() == 0)
    {
      KMessageBox::error(m_w, i18n("There are no strings to search and replace."));
      return false;
    }

  // Checks if the main directory can be accessed
  QString directory = m_info.directory();
  QDir dir;

  dir.setPath(directory);
  directory = dir.absPath();

  if(!dir.exists())
    {
      KMessageBox::error(m_w, i18n("<qt>The main folder of the project <b>%1</b> does not exist.</qt>").arg(directory));
      return false;
    }

  QFileInfo dirInfo(directory);
  if(!(dirInfo.isReadable() && dirInfo.isExecutable())
     or
     not(dirInfo.isWritable()))
    {
      KMessageBox::error(m_w, i18n("<qt>Access denied in the main folder of the project:<br><b>%1</b></qt>").arg(directory));
      return false;
    }
  /*if(::access(directory.local8Bit(), R_OK | X_OK) == -1)
    {
      KMessageBox::error(w, i18n("<qt>Access denied in the main folder of the project:<br><b>" + directory + "</b></qt>"));
      return false;
    } */

  // Clears the list view
  m_view->resultView()->clear();

  return true;
}

void KFileReplacePart::whatsThis()
{
  actionCollection()->action("options_backup")->setWhatsThis(i18n(optionsBackupWhatthis));
  actionCollection()->action("options_case")->setWhatsThis(i18n(optionsCaseWhatthis));
  actionCollection()->action("options_var")->setWhatsThis(i18n(optionsVarWhatthis));
  actionCollection()->action("options_recursive")->setWhatsThis(i18n(optionsRecursiveWhatthis));
  actionCollection()->action("options_regularexpressions")->setWhatsThis(i18n(optionsRegularExpressionsWhatthis));
}

//PRIVATE METHODS
void KFileReplacePart::launchNewProjectDialog(const KURL & startURL)
{
  KNewProjectDlg dlg(0, m_config);

  if (!startURL.isEmpty())
    dlg.m_cbLocation->setCurrentText(startURL.path());
  if(!dlg.exec())
    return;

  m_info.setDirectory(dlg.currentDir());
  m_info.setFilter(dlg.currentFilter());
  m_info.setQuickSearchString(dlg.quickSearchString());
  m_info.setQuickReplaceString(dlg.quickReplaceString());

  readOptions();

  slotQuickStringsAdd();

  resetActions();
}

void KFileReplacePart::setOptionMask()
{
  m_optionMask |= QDir::Dirs;

  if(!m_info.ignoreHidden())
    m_optionMask |= QDir::Hidden;

  if(!m_info.followSymLinks())
    m_optionMask |= QDir::NoSymLinks;
}

void KFileReplacePart::fileReplace()
{
  QString dirName = m_info.directory();
  QDir d(dirName);
  d.setMatchAllDirs(true);
  d.setFilter(m_optionMask);

  QStringList filesList = d.entryList(m_info.filter());
  QStringList::iterator filesIt;

  for (filesIt = filesList.begin(); filesIt != filesList.end() ; ++filesIt)
    {
      QString fileName = (*filesIt);

      // m_stop == true means that we pushed the stop button
      if(m_stop)
        return;

      // Avoids files that not match requirements
      if(!m_lib->isAnAccessibleFile(d.canonicalPath(), fileName, m_info))
        continue;

      if(m_info.backup())
        replaceAndBackup(d.canonicalPath(),fileName, m_info.regularExpressions(), m_info.simulation());
      else
        replaceAndOverwrite(d.canonicalPath(), fileName, m_info.regularExpressions(), m_info.simulation());
    }
}

void KFileReplacePart::recursiveFileReplace(const QString& dirName)
{
  //if m_stop == true then interrupts recursion
  if(m_stop)
    return;
  else
    {
      QDir d(dirName);

      d.setMatchAllDirs(true);
      d.setFilter(m_optionMask);

      QStringList filesList = d.entryList(m_info.filter());
      QStringList::iterator filesIt;

      for (filesIt = filesList.begin(); filesIt != filesList.end(); ++filesIt)
        {
          QString fileName = (*filesIt);

         // Avoids files that not match requirements
          if(!m_lib->isAnAccessibleFile(d.canonicalPath(),fileName,m_info))
            continue;

          QString filePath = d.canonicalPath()+"/"+fileName;

          QFileInfo qi(filePath);
          //if filePath is a directory then recursion
          if(qi.isDir())
            recursiveFileReplace(filePath);
          else
            {
              if(m_info.backup())
                replaceAndBackup(d.canonicalPath(), fileName, m_info.regularExpressions(), m_info.simulation());
              else
                replaceAndOverwrite(d.canonicalPath(), fileName, m_info.regularExpressions(), m_info.simulation());
            }
          //if m_stop == true then end for-loop
          if(m_stop)
            break;
        }
    }
}

void KFileReplacePart::replaceAndBackup(const QString& currentDir, const QString& oldFileName, bool regex, bool simulation)
{
  //Creates a path string
  QString oldPathString = currentDir+"/"+oldFileName;

  QFile oldFile(oldPathString);
  if(!oldFile.open(IO_ReadOnly))
    {
      KMessageBox::error(m_w,i18n("<qt>Cannot open file <b>%1</b> for reading.</qt>").arg(oldFileName));
      return ;
    }

  QString backupExtension = m_info.backupExtension();
  QFile newFile;

  //If we are not performing a simulation creates a backup file
  if(!simulation)
    {
      newFile.setName(oldPathString + backupExtension);
      if(!newFile.open(IO_WriteOnly))
        {
          KMessageBox::error(m_w,i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(oldFileName+backupExtension));
          return ;
        }
    }

  QTextStream oldStream(&oldFile),
              newStream(&newFile);//if simulation == true this stream is empty

  QFileInfo oldFileInfo(oldPathString);
  QString fileSizeBeforeReplacing =  m_lib->formatFileSize(oldFileInfo.size());

  kapp->processEvents();

  bool atLeastOneStringFound = false;
  KListViewItem *item = 0L;
  QString line = oldStream.read();
  int occurrence = 0;

  replacingLoop(line, &item, atLeastOneStringFound, occurrence, regex);

  if(!simulation)
    newStream << line;

   newFile.close();
   oldFile.close();

   if(!m_info.ignoreFiles())
     atLeastOneStringFound = true;

   if(atLeastOneStringFound && item/* && atLeastOneStringConfirmed*/)
     {
       m_lib->setIconForFileEntry(item,currentDir+"/"+oldFileName);
       item->setText(0,oldFileName);
       item->setText(1,currentDir);
       item->setText(2,fileSizeBeforeReplacing);
       QFileInfo nf(oldPathString+backupExtension);
       if(!simulation)
         {
           item->setText(3,m_lib->formatFileSize(nf.size()));
         }
       else
         {
           item->setText(3,"-");
         }

       item->setText(4,QString::number(occurrence,10));
       item->setText(5,QString("%1[%2]").arg(oldFileInfo.owner()).arg(oldFileInfo.ownerId()));
       item->setText(6,QString("%1[%2]").arg(oldFileInfo.group()).arg(oldFileInfo.groupId()));
     }
}

void KFileReplacePart::replaceAndOverwrite(const QString& currentDir, const QString& oldFileName, bool regex, bool simulation)
{
  QString oldPathString = currentDir+"/"+oldFileName;
  QFile oldFile(oldPathString);
  QFileInfo oldFileInfo(oldPathString);

  if (!oldFile.open(IO_ReadOnly) || !oldFileInfo.isWritable())
    {
      KMessageBox::error(m_w,i18n("<qt>Cannot open file <b>%1</b> for reading and/or writing.</qt>").arg(oldFileName));
      return ;
    }

  kapp->processEvents();

  QString fileSizeBeforeReplacing =  m_lib->formatFileSize(oldFileInfo.size());
  KListViewItem *item = 0L;

  QTextStream oldStream( &oldFile );
  QString newFileBuffer,
          line = oldStream.read();;

  bool atLeastOneStringFound = false;
  int occurrence = 0;

  replacingLoop(line, &item, atLeastOneStringFound, occurrence, regex);

  if(!simulation)
    newFileBuffer += line;

  oldFile.close();
  if(!simulation)
  {
    QFile newFile(oldPathString);
    if(!newFile.open(IO_WriteOnly))
      {
        KMessageBox::error(m_w,i18n("<qt>Cannot overwrite file <b>%1</b>.</qt>").arg(oldFileName));
        return ;
      }
    QTextStream newStream( &newFile );
    newStream << newFileBuffer;
    newFile.close();
  }
  QFileInfo nf(oldPathString);
  QString fileSizeAfterReplacing = m_lib->formatFileSize(nf.size());

  //if ignoreFiles == false then every files must be show
  if(!m_info.ignoreFiles())
    atLeastOneStringFound = true;

  if(atLeastOneStringFound  && item/*&& atLeastOneStringConfirmed*/)
    {
      m_lib->setIconForFileEntry(item,currentDir+"/"+oldFileName);
      item->setText(0,oldFileName);
      item->setText(1,currentDir);
      item->setText(2,fileSizeBeforeReplacing);
      if(!simulation)
        {
          item->setText(3,fileSizeAfterReplacing);
        }
      else
        {
          item->setText(3,"-");
        }
      item->setText(4,QString::number(occurrence,10));
      item->setText(5,QString("%1[%2]").arg(oldFileInfo.owner()).arg(oldFileInfo.ownerId()));
      item->setText(6,QString("%1[%2]").arg(oldFileInfo.group()).arg(oldFileInfo.groupId()));
    }
}

void KFileReplacePart::replacingLoop(QString& line, KListViewItem** item, bool& atLeastOneStringFound, int& occur, bool regex)
{
  KeyValueMap tempMap = m_replacementMap;
  KeyValueMap::Iterator it;
  bool caseSensitive = m_info.caseSensitive();

  for(it = tempMap.begin(); it != tempMap.end(); ++it)
    {
      ResultViewEntry entry(it.key(),it.data(),regex,caseSensitive);
      while(entry.pos(line) != -1)
        {
          atLeastOneStringFound = true;
          QString msg = entry.message(entry.capturedText(line),
                              entry.lineNumber(line),
                              entry.columnNumber(line));
          if (!*item)
            *item =  new KListViewItem(m_view->resultView());
          KListViewItem* tempItem = new KListViewItem(*item);
          tempItem->setMultiLinesEnabled(true);
          tempItem->setText(0,msg);
          occur ++;
          entry.updateLine(line);
          entry.incPos();

        }
    }

}

void KFileReplacePart::fileSearch(const QString& dirName, const QString& filters)
{
  QDir d(dirName);

  d.setMatchAllDirs(true);
  d.setFilter(m_optionMask);

  QStringList filesList = d.entryList(filters);
  QString filePath = d.canonicalPath();
  QStringList::iterator filesIt;

  for (filesIt = filesList.begin(); filesIt != filesList.end() ; ++filesIt)
    {
      QString fileName = (*filesIt);

      // We pushed stop button
      if(m_stop)
        return;
      // Avoids files that not match
      if(!m_lib->isAnAccessibleFile(filePath,fileName,m_info))
        continue;
      QFileInfo fileInfo(filePath+"/"+fileName);
      if(fileInfo.isDir())
        continue;

      search(filePath, fileName, m_info.regularExpressions());
    }
}

void KFileReplacePart::recursiveFileSearch(const QString& dirName, const QString& filters)
{
  // if m_stop == true then interrupt recursion
  if(m_stop)
    return;
  else
    {
      QDir d(dirName);

      d.setMatchAllDirs(true);
      d.setFilter(m_optionMask);

      QStringList filesList = d.entryList(filters);
      QString filePath = d.canonicalPath();
      QStringList::iterator filesIt;

      for(filesIt = filesList.begin(); filesIt != filesList.end(); ++filesIt)
        {
          QString fileName = (*filesIt);
          // Avoids files that not match
          if(!m_lib->isAnAccessibleFile(filePath,fileName,m_info))
            continue;

          // Composes file path string
          QFileInfo fileInfo(filePath+"/"+fileName);

          // Searchs recursively if "filePath" is a directory
          if(fileInfo.isDir())
            recursiveFileSearch(filePath+"/"+fileName,filters);
          else
            search(filePath, fileName, m_info.regularExpressions());

          // avoids unnecessary for-loop if we want to stop
          if(m_stop)
            break;
         }
    }
}

void KFileReplacePart::search(const QString& currentDir, const QString& fileName, bool regex)
{
  QFile file(currentDir+"/"+fileName);

  if(!file.open(IO_ReadOnly))
    {
      KMessageBox::error(m_w,i18n("<qt>Cannot open file <b>%1</b> for reading.</qt>").arg(fileName));
      return ;
    }
  // Creates a stream with the file
  QTextStream stream( &file );

  QFileInfo fileInfo(currentDir+"/"+fileName);

  kapp->processEvents();

  KListViewItem *item = 0L;
  bool caseSensitive = m_info.caseSensitive(),
       haltOnFirstOccur = m_info.haltOnFirstOccur();

  //Count occurrences
  int occurrence = 0;

  //This map contains search strings
  KeyValueMap tempMap = m_replacementMap;
  bool atLeastOneStringFound = false;

  KeyValueMap::Iterator it = tempMap.begin();

  QString line = stream.read();

  while(it != tempMap.end())
    {
      QString key = it.key(),
             data = it.data();
      QString strKey;
      QRegExp rxKey;

      if(regex)
        rxKey = QRegExp("("+key+")", caseSensitive, false);
      else
        strKey = key;
      //If this option is true then for any string in
      //the map we search for the first occurrence of that string
      if(haltOnFirstOccur)
        {
          int pos;

          if(regex)
            pos = line.find(rxKey);
          else
            pos = line.find(strKey, 0 ,caseSensitive);

          if(pos != -1)
            {
              atLeastOneStringFound = true;
              int lineNumber = line.mid(0,pos).contains('\n')+1;
              int columnNumber = pos - line.findRev('\n',pos);
              if (!item)
                item = new KListViewItem(m_view->resultView());
              KListViewItem* tempItem= new KListViewItem(item);
              QString msg,
                      capturedText;

              if(regex)
                {
                  capturedText = rxKey.cap(1).replace('\n',"\\n");
                  msg = i18n(" first captured text")+
                        " \""+capturedText+"\" "+
                        i18n("at line:")+
                        QString::number(lineNumber,10)+
                        i18n(", column:")+
                        QString::number(columnNumber,10);
                }
              else
                {
                  capturedText = line.mid(pos,strKey.length()).replace('\n',"\\n");
                  msg = i18n(" first occurence of string")+
                        " \""+capturedText+"\" "+
                        i18n("found at line:")+
                        QString::number(lineNumber,10)+
                        i18n(", column:")+
                        QString::number(columnNumber,10);
                }
              tempItem->setText(0,msg);
              occurrence = 1;
              ++it;
              continue;
            }

        }// ends haltOnFirstOccur if-block
      else
        {
          //This point of the code is reached when we must search for all
          //occurrences of all the strings

          int pos = 0;
          if(regex)
            pos = rxKey.search(line,pos);
          else
            pos = line.find(strKey, pos ,caseSensitive);
          while(pos != -1)
            {
              atLeastOneStringFound = true;
              QString msg,
                      capturedText;
              int lineNumber = line.mid(0,pos).contains('\n')+1;
              int columnNumber = pos - line.findRev('\n',pos);
              if (!item)
                item = new KListViewItem(m_view->resultView());
              KListViewItem* tempItem = new KListViewItem(item);

              if(regex)
                {
                  capturedText = rxKey.cap(1).replace('\n',"\\n");
                  msg = i18n(" captured text")+
                        " \""+capturedText+"\" "+
                        i18n("at line:")+
                        QString::number(lineNumber,10)+
                        i18n(", column:")+
                        QString::number(columnNumber,10);
                  pos = rxKey.search(line, pos+rxKey.matchedLength());
                }
              else
                {
                  capturedText = line.mid(pos,strKey.length()).replace('\n',"\\n");
                  msg = i18n(" string")+
                        " \""+capturedText+"\" "+
                        i18n("found at line:")+
                        QString::number(lineNumber,10)+
                        i18n(", column:")+
                        QString::number(columnNumber,10);
                  pos = line.find(strKey,pos+strKey.length());
                }
              tempItem->setText(0,msg);
              occurrence++;
            }
        }
      //Advances of one position in the map of the strings
      ++it;
    }

   file.close();
   //if ignoreFiles == false then every files must be show
   if(!m_info.ignoreFiles())
     atLeastOneStringFound = true;

   if (atLeastOneStringFound && item)
     {

       m_lib->setIconForFileEntry(item,currentDir+"/"+fileName);
       item->setText(0,fileName);
       item->setText(1,currentDir);
       item->setText(2,m_lib->formatFileSize(fileInfo.size()));
       item->setText(4,QString::number(occurrence,10));
       item->setText(5,QString("%1[%2]").arg(fileInfo.owner()).arg(fileInfo.ownerId()));
       item->setText(6,QString("%1[%2]").arg(fileInfo.group()).arg(fileInfo.groupId()));
     }
}

void KFileReplacePart::loadViewContent()
{
  KeyValueMap tempMap;
  QListViewItemIterator itlv(m_view->stringView());
  while(itlv.current())
    {
      QListViewItem *item = itlv.current();
      if(m_info.variables())
        tempMap[item->text(0)] = m_command.variableValue(item->text(1));
      else
        tempMap[item->text(0)] = item->text(1);
      ++itlv;
    }
  m_replacementMap = tempMap;
}

void KFileReplacePart::stringsInvert(bool invertAll)
{
  QListViewItem* lviCurItem,
               * lviFirst;
  if(invertAll)
    lviCurItem = lviFirst = m_view->stringView()->firstChild();
  else
    lviCurItem = lviFirst = m_view->stringView()->currentItem();

  if(lviCurItem == 0)
    return ;

  do
    {
      QString searchText = lviCurItem->text(0),
              replaceText = lviCurItem->text(1);

      // Cannot invert the string if search string will be empty
      if (replaceText.isEmpty())
        {
          KMessageBox::error(m_w, i18n("<qt>Cannot invert string <b>%1</b>, because the search string would be empty.</qt>").arg(searchText));
          return;
        }

      lviCurItem->setText(0, replaceText);
      lviCurItem->setText(1, searchText);

      lviCurItem = lviCurItem->nextSibling();
      if(!invertAll)
        break;
    } while(lviCurItem && lviCurItem != lviFirst);
  m_view->currentStringsViewMap();
}

#include "kfilereplacepart.moc"
