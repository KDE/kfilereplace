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

// QT
#include <qdir.h>
#include <qdatastream.h>
#include <qregexp.h>
#include <qimage.h>
#include <qlcdnumber.h>

// KDE
#include <dcopclient.h>
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
#include <kled.h>

// local
#include "kfilereplacelib.h"
#include "kfilereplacepart.h"
#include "kfilereplaceview.h"
#include "koptionsdlg.h"
#include "knewprojectdlg.h"
#include "report.h"
#include "commandengine.h"
#include "whatthis.h"

using namespace whatthisNameSpace;

//PUBLIC CONSTRUCTORS
// Factory code for KDE 3
typedef KParts::GenericFactory<KFileReplacePart> FileReplaceFactory;

K_EXPORT_COMPONENT_FACTORY( libkfilereplacepart, FileReplaceFactory )

KFileReplacePart::KFileReplacePart(QWidget* parentWidget, const char* , QObject* parent, const char* name, const QStringList & ) : KParts::ReadOnlyPart(parent,name)
{
  setInstance(FileReplaceFactory::instance());

  m_parentWidget = parentWidget;
  QString configName = locateLocal("config", "kfilereplacerc");

  m_config = new KConfig(configName);
  m_aboutDlg = 0;
  m_stop = false;
  m_optionMask = QDir::Files;
  m_w = widget();
  m_option = 0;

  loadOptionsFromRC();
  initView();
  initGUI();

  whatsThis();
}

KFileReplacePart::~KFileReplacePart()
{
  m_view = 0; //it's already deleted, so set it to NULL

  saveOptionsToRC();

  delete m_aboutDlg;
  m_aboutDlg = 0;
  delete m_config;
  m_config = 0;
  delete m_w;
  m_w = 0;
  delete m_option;
}

//PUBLIC SLOTS
void KFileReplacePart::slotSetNewParameters()
{
  launchNewProjectDialog(KURL());
  m_view->changeView(m_option->m_searchingOnlyMode);
  emit setStatusBarText(i18n("Ready."));
}

void KFileReplacePart::slotSearchingOperation()
{
  if(!checkBeforeOperation())
    return;

  KListView* rv = m_view->getResultsView();

  rv->clear();

  rv->setSorting(-1);

  // show wait cursor
  QApplication::setOverrideCursor( Qt::waitCursor );

  freezeActions();

  setOptionMask();

  QString currentDirectory = QStringList::split(",",m_option->m_directories)[0],
          currentFilter = QStringList::split(",",m_option->m_filters)[0];

  showSemaphore("red");

  uint filesNumber = 0;

  if(m_option->m_recursive)
    recursiveFileSearch(currentDirectory, currentFilter, filesNumber);
  else
    fileSearch(currentDirectory, currentFilter);

  showSemaphore("yellow");

  kapp->processEvents();

  //disabling and enabling sorting... don't ask me why, but it works!
  rv->setSorting(0);
  rv->sort();
  rv->setSorting(-1);

  // restore false status for stop button
  m_stop = false;

  QApplication::restoreOverrideCursor();

  emit setStatusBarText(i18n("Search completed."));

  m_option->m_searchingOnlyMode = true;

  resetActions();

  m_searchingOperation = true;

  showSemaphore("green");
}

void KFileReplacePart::slotReplacingOperation()
{
  if(!checkBeforeOperation())
    return;

  KListView* rv = m_view->getResultsView();

  if(m_option->m_simulation)
    {
      emit setStatusBarText(i18n("Replacing files (simulation)..."));
      rv->setColumnText(4,i18n("Replaced strings (simulation)"));
    }
  else
    {
      emit setStatusBarText(i18n("Replacing files..."));
      rv->setColumnText(4,i18n("Replaced strings"));
    }
  // show wait cursor
  QApplication::setOverrideCursor( Qt::waitCursor );

  freezeActions();

  setOptionMask();

  rv->setSorting(-1);

  showSemaphore("green");

  QString currentDirectory = QStringList::split(",",m_option->m_directories)[0];

  showSemaphore("red");

  if(m_option->m_recursive)
    {
      int filesNumber = 0;
      recursiveFileReplace(currentDirectory, filesNumber);
    }
  else
    {
      fileReplace();
    }

  rv->setSorting(0);
  rv->sort();
  rv->setSorting(-1);

  // restore false status for stop button
  m_stop = false;

  QApplication::restoreOverrideCursor();

  m_option->m_searchingOnlyMode = false;

  resetActions();

  m_searchingOperation = false;

  showSemaphore("green");
}

void KFileReplacePart::slotSimulatingOperation()
{
  m_option->m_simulation = true;
  slotReplacingOperation();
  m_option->m_simulation = false;
}

void KFileReplacePart::slotStop()
{
  emit setStatusBarText(i18n("Stopping..."));
  m_stop = true;
  QApplication::restoreOverrideCursor();
  resetActions();
}

void KFileReplacePart::slotCreateReport()
{
  // Check there are results
  KListView* rv = m_view->getResultsView(),
           * sv = m_view->getStringsView();

  if(rv->firstChild() == 0)
    {
      KMessageBox::error(m_w, i18n("There are no results to save: the result list is empty."));
      return ;
    }
  // Select the file where results will be saved
  QString documentName = KFileDialog::getSaveFileName(QString::null, "*.xml|XML " + i18n("Files") + " (*.xml)", m_w, i18n("Save Report"));
  if (documentName.isEmpty())
    return ;

  // delete a spourious extension
  documentName.truncate(documentName.length()-4);

  QFileInfo fileInfo(documentName);
  if(fileInfo.exists())
    {
      KMessageBox::error(m_w, i18n("<qt>A folder or a file named <b>%1</b> already exists.</qt>").arg(documentName));
      return ;
    }

  QDir directoryName;

  if(!directoryName.mkdir(documentName, true))
    {
      KMessageBox::error(m_w, i18n("<qt>Cannot create the <b>%1</b> folder.</qt>").arg(documentName));
      return ;
    }

  directoryName.cd(documentName);

  QString documentPath = documentName+"/"+directoryName.dirName();

  Report report(m_option, rv, sv);
         report.createDocument(documentPath);

  resetActions();
}

void KFileReplacePart::slotStringsAdd()
{
  m_view->slotStringsAdd();
  resetActions();
}

void KFileReplacePart::slotQuickStringsAdd()
{
  //this slot handles a pair of strings that come from project dialog,
  //if the control character 'N' is found at the position 0 of the two strings,
  //then we start the search now.
  QString qs = m_option->m_quickSearchString;
  QStringList map;

  map.append(qs.left(1));
  map.append(qs.right(qs.length()-1));
  //in this case there is no string to search for, so return
  if(map[1].isEmpty())
    return;

  //in this one instead, we must search for a string
  qs = m_option->m_quickReplaceString;
  map.append(qs.left(1));
  map.append(qs.right(qs.length()-1));

  m_view->updateOptions(m_option);

  m_view->slotQuickStringsAdd(map[1],map[3]);

  //if search-only mode == true and search-now mode == true then
  //search string
  if(map[0] == "N")
    if(m_option->m_searchingOnlyMode)
      slotSearchingOperation();
    else
      slotReplacingOperation();
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
  m_view->slotStringsEdit();
  resetActions();
}

void KFileReplacePart::slotStringsSave()
{
  // Check there are strings in the list
  KListView* sv = m_view->getStringsView();

  if (sv->firstChild() == 0)
    {
      KMessageBox::error(m_w, i18n("No strings to save as the list is empty."));
      return ;
    }

   QString header("<?xml version=\"1.0\" ?>\n<kfr>"),
           footer("\n</kfr>"),
           body;
   if(m_option->m_searchingOnlyMode)
     header += "\n\t<mode search=\"true\"/>";
   else
     header += "\n\t<mode search=\"false\"/>";

   QListViewItem*  lvi = sv->firstChild();

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

   fileName = KFileReplaceLib::addExtension(fileName, "kfr");

   QFile file( fileName );
   if(!file.open( IO_WriteOnly ))
     {
       KMessageBox::error(m_w, i18n("File %1 cannot be saved.").arg(fileName));
       return ;
     }
   QTextStream oTStream( &file );
   oTStream.setEncoding(QTextStream::UnicodeUTF8);
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
  resetActions();
}

void KFileReplacePart::slotStringsInvertAll()
{
  stringsInvert(true);
  resetActions();
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

void KFileReplacePart::slotOptionRecursive()
{
  m_option->m_recursive = !m_option->m_recursive;
  resetActions();
}

void KFileReplacePart::slotOptionBackup()
{
  m_option->m_backup = !m_option->m_backup;
  resetActions();
}

void KFileReplacePart::slotOptionCaseSensitive()
{
  m_option->m_caseSensitive = !m_option->m_caseSensitive;
  resetActions();
}

void KFileReplacePart::slotOptionVariables()
{
  m_option->m_variables = !m_option->m_variables;
  resetActions();
}

void KFileReplacePart::slotOptionRegularExpressions()
{
  m_option->m_regularExpressions = !m_option->m_regularExpressions;
  resetActions();
}

void KFileReplacePart::slotOptionPreferences()
{
  KOptionsDlg dlg(m_option, m_w, 0);

  if(!dlg.exec())
    return;
  qWarning("ASK=%d",dontAskAgain());
  qWarning("M_OPTION=%d",m_option->m_askConfirmReplace);
  //updating m_view
  m_view->updateOptions(m_option);

  resetActions();
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

void KFileReplacePart::reportBug()
{
  KBugReport dlg(m_w, true, createAboutData());
  dlg.exec();
}

void KFileReplacePart::appHelpActivated()
{
  kapp->invokeHelp(QString::null, "kfilereplace");
}

void KFileReplacePart::resetActions()
{
  KListView* rv = m_view->getResultsView(),
           * sv = m_view->getStringsView();

  bool hasItems = (sv->firstChild() != 0),
       searchOnlyMode = m_option->m_searchingOnlyMode;

  // File
  actionCollection()->action("new_project")->setEnabled(true);
  actionCollection()->action("search")->setEnabled(hasItems && searchOnlyMode);
  actionCollection()->action("file_simulate")->setEnabled(hasItems && !searchOnlyMode);
  actionCollection()->action("replace")->setEnabled(hasItems && !searchOnlyMode);
  actionCollection()->action("save_results")->setEnabled(hasItems);
  actionCollection()->action("stop")->setEnabled(false);

  // Strings
  actionCollection()->action("strings_add")->setEnabled(true);
  actionCollection()->action("strings_del")->setEnabled(hasItems);
  actionCollection()->action("strings_empty")->setEnabled(hasItems);
  actionCollection()->action("strings_edit")->setEnabled(hasItems);
  actionCollection()->action("strings_save")->setEnabled(hasItems);
  actionCollection()->action("strings_load")->setEnabled(true);
  actionCollection()->action("strings_invert")->setEnabled(hasItems && !searchOnlyMode);
  actionCollection()->action("strings_invert_all")->setEnabled(hasItems && !searchOnlyMode);

  // Options
  actionCollection()->action("options_recursive")->setEnabled(true);
  actionCollection()->action("options_backup")->setEnabled(!searchOnlyMode);
  actionCollection()->action("options_case")->setEnabled(true);
  actionCollection()->action("options_var")->setEnabled(!searchOnlyMode);
  actionCollection()->action("options_regularexpressions")->setEnabled(true);
  actionCollection()->action("configure_kfilereplace")->setEnabled(true);

  hasItems = (rv->firstChild() != 0);

  // Results
  actionCollection()->action("results_infos")->setEnabled(hasItems);
  actionCollection()->action("results_openfile")->setEnabled(hasItems);
  if (actionCollection()->action("results_editfile"))
      actionCollection()->action("results_editfile")->setEnabled(hasItems);
  actionCollection()->action("results_opendir")->setEnabled(hasItems);
  actionCollection()->action("results_delete")->setEnabled(hasItems);
  actionCollection()->action("results_treeexpand")->setEnabled(hasItems);
  actionCollection()->action("results_treereduce")->setEnabled(hasItems);

  // Updates menus and toolbar
  ((KToggleAction* ) actionCollection()->action("options_recursive"))->setChecked(m_option->m_recursive);
  ((KToggleAction* ) actionCollection()->action("options_backup"))->setChecked(m_option->m_backup && !searchOnlyMode);
  ((KToggleAction* ) actionCollection()->action("options_case"))->setChecked(m_option->m_caseSensitive);
  ((KToggleAction* ) actionCollection()->action("options_var"))->setChecked(m_option->m_variables && !searchOnlyMode);
  ((KToggleAction* ) actionCollection()->action("options_regularexpressions"))->setChecked(m_option->m_regularExpressions);
}

//PUBLIC METHODS
KAboutData* KFileReplacePart::createAboutData()
{
  KAboutData * aboutData = new KAboutData("kfilereplacepart",
                                          I18N_NOOP("KFileReplacePart"),
                                          KFR_VERSION,
                                          I18N_NOOP( "Batch search and replace tool."),
                                          KAboutData::License_GPL_V2,
                                          "(C) 1999-2002 Fran�is Dupoux\n(C) 2003-2004 Andras Mantia\n(C) 2004 Emiliano Gulmini", I18N_NOOP("Part of the KDEWebDev module."), "http://www.kdewebdev.org");
  aboutData->addAuthor("Fran�is Dupoux",
                       I18N_NOOP("Original author of the KFileReplace tool"),
                       "dupoux@dupoux.com");
  aboutData->addAuthor("Emiliano Gulmini",
                       I18N_NOOP("Current maintainer, code cleaner and rewriter"),
                       "emi_barbarossa@yahoo.it");
  aboutData->addAuthor("Andras Mantia",
                       I18N_NOOP("Co-maintainer, KPart creator"),
                       "amantia@kde.org");
  aboutData->addCredit("Heiko Goller",
                       I18N_NOOP("Original german translator"),
                       "heiko.goller@tuebingen.mpg.de");
  return aboutData;
}

KConfig* KFileReplacePart::config()
{
  return m_config;
}

//PROTECTED METHODS
bool KFileReplacePart::openURL(const KURL &url)
{
  if(!url.isEmpty() && (url.protocol() != "file"))
    {
      KMessageBox::sorry(m_w, i18n("Sorry, currently the KFileReplace part works only for local files."), i18n("Non Local File"));
      emit canceled("");
      return false;
    }
  if(!url.isEmpty())
    return launchNewProjectDialog(url);
  else
    {
      resetActions();
      return true;
    }
}

//PRIVATE METHODS
void KFileReplacePart::initGUI()
{
  setXMLFile("kfilereplacepartui.rc");

  actionCollection()->setHighlightingEnabled(true);
  
  DCOPClient *client = kapp->dcopClient();
  QCStringList appList = client->registeredApplications();
  bool quantaFound = false;
  for(QCStringList::Iterator it = appList.begin(); it != appList.end(); ++it)
  {
    if((*it).left(6) == "quanta")
    {
      quantaFound = true;
      break;
    }
  }
  // File
  (void)new KAction(i18n("Customize Search/Replace Session"), "projectopen", 0, this, SLOT(slotSetNewParameters()), actionCollection(), "new_project");
  (void)new KAction(i18n("&Search"), "filesearch", 0, this, SLOT(slotSearchingOperation()), actionCollection(), "search");
  (void)new KAction(i18n("S&imulate"), "filesimulate", 0, this, SLOT(slotSimulatingOperation()), actionCollection(), "file_simulate");
  (void)new KAction(i18n("&Replace"),  "filereplace", 0, this, SLOT(slotReplacingOperation()), actionCollection(), "replace");
  (void)new KAction(i18n("Sto&p"), "stop", 0, this, SLOT(slotStop()), actionCollection(), "stop");
  (void)new KAction(i18n("Cre&ate Report File"), "filesave", 0, this, SLOT(slotCreateReport()), actionCollection(), "save_results");

  // Strings
  (void)new KAction(i18n("&Add String"), "edit_add", 0, this, SLOT(slotStringsAdd()), actionCollection(), "strings_add");

  (void)new KAction(i18n("&Delete String"), "edit_remove", 0, this, SLOT(slotStringsDeleteItem()), actionCollection(), "strings_del");
  (void)new KAction(i18n("&Empty Strings List"), "eraser", 0, this, SLOT(slotStringsEmpty()), actionCollection(), "strings_empty");
  (void)new KAction(i18n("Edit Selected String"), "edit", 0, this, SLOT(slotStringsEdit()), actionCollection(), "strings_edit");
  (void)new KAction(i18n("&Save Strings List to File"), "filesave", 0, this, SLOT(slotStringsSave()), actionCollection(), "strings_save");
  (void)new KAction(i18n("&Load Strings List From File"), "unsortedList", 0, this, SLOT(slotStringsLoad()), actionCollection(), "strings_load");
  (void)new KRecentFilesAction(i18n("&Load Recent Strings Files"), "fileopen", 0, this, SLOT(slotOpenRecentStringFile(const KURL&)), actionCollection(),"strings_load_recent");
  (void)new KAction(i18n("&Invert Current String (search <--> replace)"), "invert", 0, this, SLOT(slotStringsInvertCur()), actionCollection(), "strings_invert");
  (void)new KAction(i18n("&Invert All Strings (search <--> replace)"), "invert", 0, this, SLOT(slotStringsInvertAll()), actionCollection(), "strings_invert_all");

  // Options
  (void)new KToggleAction(i18n("&Include Sub-Folders"), "recursive_option", 0, this, SLOT(slotOptionRecursive()), actionCollection(), "options_recursive");
  (void)new KToggleAction(i18n("Create &Backup Files"), "backup_option", 0, this, SLOT(slotOptionBackup()), actionCollection(), "options_backup");
  (void)new KToggleAction(i18n("Case &Sensitive"), "casesensitive_option", 0, this, SLOT(slotOptionCaseSensitive()), actionCollection(), "options_case");
  (void)new KToggleAction(i18n("Enable &Commands in Replace String: [$command:option$]"), "command_option", 0, this, SLOT(slotOptionVariables()), actionCollection(), "options_var");
  (void)new KToggleAction(i18n("Enable &Regular Expressions"), "regularexpression_option", 0, this, SLOT(slotOptionRegularExpressions()), actionCollection(), "options_regularexpressions");
  (void)new KAction(i18n("Configure &KFileReplace"), "configure", 0, this, SLOT(slotOptionPreferences()), actionCollection(), "configure_kfilereplace");

  // Results
  (void)new KAction(i18n("&Properties"), "informations", 0, m_view, SLOT(slotResultProperties()), actionCollection(), "results_infos");
  (void)new KAction(i18n("&Open"), "filenew", 0, m_view, SLOT(slotResultOpen()), actionCollection(), "results_openfile");

  if(quantaFound)
    {
      (void)new KAction(i18n("&Edit in Quanta"), "quanta", 0, m_view, SLOT(slotResultEdit()), actionCollection(), "results_editfile");
    }
  (void)new KAction(i18n("Open Parent &Folder"), "fileopen", 0, m_view, SLOT(slotResultDirOpen()), actionCollection(), "results_opendir");
  (void)new KAction(i18n("&Delete"), "eraser", 0, m_view, SLOT(slotResultDelete()), actionCollection(), "results_delete");
  (void)new KAction(i18n("E&xpand Tree"), 0, m_view, SLOT(slotResultTreeExpand()), actionCollection(), "results_treeexpand");
  (void)new KAction(i18n("&Reduce Tree"), 0, m_view, SLOT(slotResultTreeReduce()), actionCollection(), "results_treereduce");
  (void)new KAction(i18n("&About KFileReplace"), "kfilereplace", 0, this, SLOT(showAboutApplication()), actionCollection(), "help_about_kfilereplace");
  (void)new KAction(i18n("KFileReplace &Handbook"), "help", 0, this, SLOT(appHelpActivated()), actionCollection(), "help_kfilereplace");
  (void)new KAction(i18n("&Report Bug"), 0, 0, this, SLOT(reportBug()), actionCollection(), "report_bug");

}

void KFileReplacePart::initView()
{
  m_view = new KFileReplaceView(m_option, m_parentWidget, "view");

  setWidget(m_view);

  m_view->setAcceptDrops(false);

  showSemaphore("green");
}

void KFileReplacePart::freezeActions()
{
  //Disables actions during search/replace operation
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
  if (actionCollection()->action("results_editfile"))
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

void KFileReplacePart::loadOptionsFromRC()
{
  m_option = new RCOptions();

  loadOptions();
  loadFileSizeOptions();
  loadDateAccessOptions();
  loadOwnerOptions();
  loadLocationsList();
  loadFiltersList();
  loadBackupExtensionOptions();
}

void KFileReplacePart::loadOptions()
{
  m_config->setGroup("General Options");

  m_option->m_recentStringFileList = m_config->readListEntry(rcRecentFiles);

  m_option->m_searchingOnlyMode = m_config->readBoolEntry(rcSearchMode,SearchModeOption);

  m_config->setGroup("Options");

  m_option->m_recursive = m_config->readBoolEntry(rcRecursive, RecursiveOption);

  m_option->m_caseSensitive = m_config->readBoolEntry(rcCaseSensitive, CaseSensitiveOption);
  m_option->m_variables = m_config->readBoolEntry(rcVariables, VariablesOption);
  m_option->m_regularExpressions = m_config->readBoolEntry(rcRegularExpressions, RegularExpressionsOption);
  m_option->m_followSymLinks = m_config->readBoolEntry(rcFollowSymLinks, FollowSymbolicLinksOption);

  m_option->m_haltOnFirstOccur = m_config->readBoolEntry(rcHaltOnFirstOccur, StopWhenFirstOccurenceOption);

  m_option->m_ignoreHidden = m_config->readBoolEntry(rcIgnoreHidden, IgnoreHiddenOption);
  m_option->m_ignoreFiles = m_config->readBoolEntry(rcIgnoreFiles, IgnoreFilesOption);

  m_config->setGroup("Notification Messages");

  m_option->m_notifyOnErrors  = m_config->readBoolEntry(rcNotifyOnErrors, true);

  m_option->m_askConfirmReplace = m_config->readBoolEntry(rcAskConfirmReplace, AskConfirmReplaceOption);

  QString dontAskAgain = m_config->readEntry(rcDontAskAgain, "no");

  if(dontAskAgain == "yes")
    m_option->m_askConfirmReplace = false;
}

void KFileReplacePart::loadFileSizeOptions()
{
  m_config->setGroup("Size options");

  m_option->m_minSize = m_config->readNumEntry(rcMinFileSize, FileSizeOption);
  m_option->m_maxSize = m_config->readNumEntry(rcMaxFileSize, FileSizeOption);
}

void KFileReplacePart::loadDateAccessOptions()
{
  m_config->setGroup("Access options");

  m_option->m_dateAccess = m_config->readEntry(rcValidAccessDate, ValidAccessDateOption);
  m_option->m_minDate = m_config->readEntry(rcMinDate, AccessDateOption);
  m_option->m_maxDate = m_config->readEntry(rcMaxDate, AccessDateOption);
}

void KFileReplacePart::loadOwnerOptions()
{
  m_config->setGroup("Owner options");

  QStringList ownerList = QStringList::split(",",m_config->readEntry(rcOwnerUser, OwnerOption),true);
  if(ownerList[0] == "true")
    m_option->m_ownerUserIsChecked = true;
  else
    m_option->m_ownerUserIsChecked = false;

  m_option->m_ownerUserType = ownerList[1];
  m_option->m_ownerUserBool = ownerList[2];
  m_option->m_ownerUserValue = ownerList[3];

  ownerList = QStringList::split(",",m_config->readEntry(rcOwnerGroup, OwnerOption),true);

  if(ownerList[0] == "true")
    m_option->m_ownerGroupIsChecked = true;
  else
    m_option->m_ownerGroupIsChecked = false;

  m_option->m_ownerGroupType = ownerList[1];
  m_option->m_ownerGroupBool = ownerList[2];
  m_option->m_ownerGroupValue = ownerList[3];
}

void KFileReplacePart::loadLocationsList()
{
  m_config->setGroup("Directories");
  #if KDE_IS_VERSION(3,1,3)
  QString locationsEntryList = m_config->readPathEntry(rcDirectoriesList);
  #else
  QString locationsEntryList = m_config->readEntry(rcDirectoriesList);
  #endif

  if(locationsEntryList.isEmpty())
    locationsEntryList = QDir::current().path();

    m_option->m_directories = locationsEntryList;
}

void KFileReplacePart::loadFiltersList()
{
   m_config->setGroup("Filters");
  #if KDE_IS_VERSION(3,1,3)
  QString filtersEntryList = m_config->readPathEntry(rcFiltersList);
  #else
  QString filtersEntryList = m_config->readEntry(rcFiltersList);
  #endif

  DCOPClient *client = kapp->dcopClient();
  QCStringList appList = client->registeredApplications();
  bool quantaFound = false;

  for(QCStringList::Iterator it = appList.begin(); it != appList.end(); ++it)
    {
      if((*it).left(6) == "quanta")
        {
          quantaFound = true;
          break;
        }
    }
  if(quantaFound)
    {
      if(filtersEntryList.isEmpty())
	filtersEntryList = "*.htm;*.html;*.xml;*.xhtml;*.css;*.js;*.php";
    }
  else
    {
      if(filtersEntryList.isEmpty())
	filtersEntryList = "*.c*;*.h*;*.sh;*.txt*;*.p*;*.java;*.log";
    }

  m_option->m_filters = filtersEntryList;
}

void KFileReplacePart::loadBackupExtensionOptions()
{
  m_config->setGroup("Options");
  QStringList bkList = QStringList::split(",",
                                          m_config->readEntry(rcBackupExtension, BackupExtensionOption),
                                          true);
  if(bkList[0] == "true")
    m_option->m_backup = true;
  else
    m_option->m_backup = false;

  m_option->m_backupExtension = bkList[1];
}

void KFileReplacePart::saveOptionsToRC()
{
  saveOptions();
  saveFileSizeOptions();
  saveDateAccessOptions();
  saveOwnerOptions();
  saveLocationsList();
  saveFiltersList();
  saveBackupExtensionOptions();
}

void KFileReplacePart::saveOptions()
{
  m_config->setGroup("General Options");

  m_config->writeEntry(rcRecentFiles, m_option->m_recentStringFileList);
  m_config->writeEntry(rcSearchMode,m_option->m_searchingOnlyMode);

  m_config->setGroup("Options");

  m_config->writeEntry(rcRecursive, m_option->m_recursive);
  m_config->writeEntry(rcCaseSensitive, m_option->m_caseSensitive);
  m_config->writeEntry(rcVariables, m_option->m_variables);
  m_config->writeEntry(rcRegularExpressions, m_option->m_regularExpressions);

  m_config->writeEntry(rcFollowSymLinks, m_option->m_followSymLinks);
  m_config->writeEntry(rcHaltOnFirstOccur, m_option->m_haltOnFirstOccur);

  m_config->writeEntry(rcIgnoreHidden, m_option->m_ignoreHidden);
  m_config->writeEntry(rcIgnoreFiles, m_option->m_ignoreFiles);

  m_config->setGroup("Notification Messages");
  m_config->writeEntry(rcNotifyOnErrors, m_option->m_notifyOnErrors);
  if(m_config->readEntry(rcDontAskAgain,"no") == "yes")
    m_config->writeEntry(rcAskConfirmReplace, false);
  else
    m_config->writeEntry(rcAskConfirmReplace, m_option->m_askConfirmReplace);

  m_config->sync();
}

void KFileReplacePart::saveFileSizeOptions()
{
  m_config->setGroup("Size options");

  m_config->writeEntry(rcMaxFileSize, m_option->m_maxSize);
  m_config->writeEntry(rcMinFileSize, m_option->m_minSize);

  m_config->sync();
}

void KFileReplacePart::saveDateAccessOptions()
{
  m_config->setGroup("Access options");

  m_config->writeEntry(rcValidAccessDate, m_option->m_dateAccess);
  m_config->writeEntry(rcMinDate, m_option->m_minDate);
  m_config->writeEntry(rcMaxDate, m_option->m_maxDate);

  m_config->sync();
}

void KFileReplacePart::saveOwnerOptions()
{
  m_config->setGroup("Owner options");

  QString list;
  if(m_option->m_ownerUserIsChecked)
    list = "true,";
  else
    list = "false,";

  list += m_option->m_ownerUserType +
          "," +
          m_option->m_ownerUserBool;
  if(!m_option->m_ownerUserValue.isEmpty())
    list += "," + m_option->m_ownerUserValue;

  m_config->writeEntry(rcOwnerUser,list);

  if(m_option->m_ownerGroupIsChecked)
    list = "true,";
  else
    list = "false,";

  list += m_option->m_ownerGroupType +
          "," +
          m_option->m_ownerGroupBool;
  if(!m_option->m_ownerGroupValue.isEmpty())
    list += "," + m_option->m_ownerGroupValue;

  m_config->writeEntry(rcOwnerGroup,list);
  m_config->sync();
}

void KFileReplacePart::saveLocationsList()
{
  m_config->setGroup("Directories");
  m_config->writeEntry(rcDirectoriesList, m_option->m_directories);
  m_config->sync();
}

void KFileReplacePart::saveFiltersList()
{
  m_config->setGroup("Filters");
  m_config->writeEntry(rcFiltersList, m_option->m_filters);
  m_config->sync();
}

void KFileReplacePart::saveBackupExtensionOptions()
{
  m_config->setGroup("Options");
  QString bkOptions;
  if(m_option->m_backup)
    bkOptions = "true," + m_option->m_backupExtension;
  else
    bkOptions = "false," + m_option->m_backupExtension;

  m_config->writeEntry(rcBackupExtension, bkOptions);
  m_config->sync();
}

void KFileReplacePart::fileReplace()
{
  QString directoryName = QStringList::split(",",m_option->m_directories)[0];
  QDir d(directoryName);
  d.setMatchAllDirs(true);
  d.setFilter(m_optionMask);

  QString currentFilter = QStringList::split(",",m_option->m_filters)[0];
  QStringList filesList = d.entryList(currentFilter);
  QStringList::iterator filesIt;
  int filesNumber = 0;

  m_view->displayScannedFiles(filesNumber);

  for (filesIt = filesList.begin(); filesIt != filesList.end() ; ++filesIt)
    {
      QString fileName = (*filesIt);

      // m_stop == true means that we pushed the stop button
      if(m_stop)
        break;

      // Avoids files that not match requirements
      if(!KFileReplaceLib::isAnAccessibleFile(d.canonicalPath(), fileName, m_option))
        continue;

      if(m_option->m_backup)
        replaceAndBackup(d.canonicalPath(), fileName);
      else
        replaceAndOverwrite(d.canonicalPath(), fileName);

      filesNumber++;
      m_view->displayScannedFiles(filesNumber);
    }
}

void KFileReplacePart::recursiveFileReplace(const QString& directoryName, int& filesNumber)
{
  //if m_stop == true then interrupts recursion
  if(m_stop)
    return;
  else
    {
      QDir d(directoryName);

      d.setMatchAllDirs(true);
      d.setFilter(m_optionMask);

      QString currentFilter = QStringList::split(",",m_option->m_filters)[0];
      QStringList filesList = d.entryList(currentFilter);
      QStringList::iterator filesIt;

      for(filesIt = filesList.begin(); filesIt != filesList.end(); ++filesIt)
        {
          //if m_stop == true then end for-loop
          if(m_stop)
            break;

          QString fileName = (*filesIt);

         // Avoids files that not match requirements
          if(!KFileReplaceLib::isAnAccessibleFile(d.canonicalPath(),fileName, m_option))
            continue;

          QString filePath = d.canonicalPath()+"/"+fileName;

          QFileInfo qi(filePath);

          m_view->displayScannedFiles(filesNumber);

          //if filePath is a directory then recursion
          if(qi.isDir())
            recursiveFileReplace(filePath, filesNumber);
          else
            {
              if(m_option->m_backup)
                replaceAndBackup(d.canonicalPath(), fileName);
              else
                replaceAndOverwrite(d.canonicalPath(), fileName);

              filesNumber++;
              m_view->displayScannedFiles(filesNumber);
            }
        }
    }
}

void KFileReplacePart::replaceAndBackup(const QString& currentDir, const QString& oldFileName)
{
   //Creates a path string
  QString oldPathString = currentDir+"/"+oldFileName;

  QFile currentFile(oldPathString);
  if(!currentFile.open(IO_ReadOnly))
    {
      KMessageBox::information(m_w, i18n("<qt>Cannot open file <b>%1</b> for reading.</qt>").arg(oldFileName),QString::null, rcNotifyOnErrors);
      return ;
    }
  QTextStream currentStream(&currentFile);
  currentStream.setEncoding(QTextStream::UnicodeUTF8);
  QString line = currentStream.read(),
          backupLine = line;

  QString backupSize = KFileReplaceLib::formatFileSize(currentFile.size());

  currentFile.close();

  QString backupExtension = m_option->m_backupExtension;

  kapp->processEvents();

  bool atLeastOneStringFound = false;
  KListViewItem *item = 0;
  int occurrence = 0;

  replacingLoop(line, &item, atLeastOneStringFound, occurrence, m_option->m_regularExpressions, m_option->m_askConfirmReplace);

  //If we are not performing a simulation creates a backup file
  if(!m_option->m_simulation)
    {
      if(atLeastOneStringFound)
        {
          QFile backupFile(oldPathString + backupExtension);
          if(!backupFile.open(IO_WriteOnly))
            {
              KMessageBox::information(m_w, i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(backupFile.name()),QString::null, rcNotifyOnErrors);
              return ;
            }
          QTextStream backupStream(&backupFile);
	  backupStream.setEncoding(QTextStream::UnicodeUTF8);
          backupStream << backupLine;
          backupFile.close();
	}
    }

  if(!m_option->m_simulation)
    {
      if(atLeastOneStringFound)
        {
          QFile newFile(oldPathString);
          if(!newFile.open(IO_WriteOnly))
            {
              KMessageBox::information(m_w, i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(oldFileName),QString::null, rcNotifyOnErrors);
              return ;
            }
          QTextStream newStream(&newFile);
	  newStream.setEncoding(QTextStream::UnicodeUTF8);
          newStream << line;
          newFile.close();
        }
     }

   if(!m_option->m_ignoreFiles)
     atLeastOneStringFound = true;

  QFileInfo oldFileInfo(oldPathString);

   if(atLeastOneStringFound && item/* && atLeastOneStringConfirmed*/)
     {
       KFileReplaceLib::setIconForFileEntry(item,currentDir+"/"+oldFileName);
       item->setText(0,oldFileName);
       item->setText(1,currentDir);
       QString newSize = KFileReplaceLib::formatFileSize(oldFileInfo.size());
       if(!m_option->m_simulation)
         {
           item->setText(2, backupSize);
           item->setText(3, newSize);
         }
       else
         {
           item->setText(2, backupSize);
           item->setText(3, "-");
         }

       item->setText(4,QString::number(occurrence,10));
       item->setText(5,QString("%1[%2]").arg(oldFileInfo.owner()).arg(oldFileInfo.ownerId()));
       item->setText(6,QString("%1[%2]").arg(oldFileInfo.group()).arg(oldFileInfo.groupId()));
     }
}

void KFileReplacePart::replaceAndOverwrite(const QString& currentDir, const QString& oldFileName)
{
  QString oldPathString = currentDir+"/"+oldFileName;
  QFile oldFile(oldPathString);
  QFileInfo oldFileInfo(oldPathString);

  if (!oldFile.open(IO_ReadOnly))
    {
      KMessageBox::information(m_w, i18n("<qt>Cannot open file <b>%1</b> for reading.</qt>").arg(oldFile.name()),QString::null, rcNotifyOnErrors);
      return ;
    }

  kapp->processEvents();

  QString fileSizeBeforeReplacing =  KFileReplaceLib::formatFileSize(oldFileInfo.size());
  KListViewItem *item = 0;

  QTextStream oldStream( &oldFile );
  oldStream.setEncoding(QTextStream::UnicodeUTF8);
  QString line = oldStream.read();

  oldFile.close();

  bool atLeastOneStringFound = false;
  int occurrence = 0;

  replacingLoop(line, &item, atLeastOneStringFound, occurrence, m_option->m_regularExpressions, m_option->m_askConfirmReplace);


  if(!m_option->m_simulation)
    {
      if(atLeastOneStringFound)
        {
	  QFile newFile(oldPathString);
          if(!newFile.open(IO_WriteOnly))
            {
              KMessageBox::information(m_w, i18n("<qt>Cannot open file <b>%1</b> for writing.</qt>").arg(newFile.name()),QString::null, rcNotifyOnErrors);
              return ;
            }
          QTextStream newStream( &newFile );
	  newStream.setEncoding(QTextStream::UnicodeUTF8);
          newStream << line;
          newFile.close();
	}
     }

  QFileInfo nf(oldPathString);
  QString fileSizeAfterReplacing = KFileReplaceLib::formatFileSize(nf.size());

  //if ignoreFiles == false then every files must be show
  if(!m_option->m_ignoreFiles)
    atLeastOneStringFound = true;

  if(atLeastOneStringFound  && item/*&& atLeastOneStringConfirmed*/)
    {
      KFileReplaceLib::setIconForFileEntry(item,currentDir+"/"+oldFileName);
      item->setText(0,oldFileName);
      item->setText(1,currentDir);
      item->setText(2,fileSizeBeforeReplacing);
      if(!m_option->m_simulation)
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

void KFileReplacePart::replacingLoop(QString& line, KListViewItem** item, bool& atLeastOneStringFound, int& occur, bool regularExpression, bool& askConfirmReplace)
{
  KeyValueMap tempMap = m_replacementMap;
  KeyValueMap::Iterator it;
  bool caseSensitive = m_option->m_caseSensitive;
  KListView* rv = m_view->getResultsView();

  for(it = tempMap.begin(); it != tempMap.end(); ++it)
    {
      if(m_stop)
        break;

      ResultViewEntry entry(it.key(), it.data(), regularExpression, caseSensitive);
      while(entry.pos(line) != -1)
        {
          if(m_stop)
           break;
	  //qWarning("prima=askConfirmReplace=%d",askConfirmReplace);
	  if(askConfirmReplace)
	  {//qWarning("dopo=askConfirmReplace=%d",askConfirmReplace);
	      int answer = KMessageBox::questionYesNo(0,
		                                      i18n("<qt>Do you want to replace the string <b>%1</b> with the string <b>%2</b>?</qt>").arg(it.key()).arg(it.data()),
                                          i18n("Confirm Replace"),
						      KStdGuiItem::yes(),
						      KStdGuiItem::no(),
						      "Dont ask again");
	      if(answer == KMessageBox::Yes)
	        {
		  atLeastOneStringFound = true;
		  QString msg = entry.message(entry.capturedText(line),
					      entry.lineNumber(line),
					      entry.columnNumber(line));

		  if(!*item)
		    *item =  new KListViewItem(rv);
		  KListViewItem* tempItem = new KListViewItem(*item);
		  tempItem->setMultiLinesEnabled(true);
		  tempItem->setText(0,msg);
		  occur ++;
		  entry.updateLine(line);
		  entry.incPos();
		}
              else
	        {
		  entry.incPos();
		}
              if(dontAskAgain()) askConfirmReplace = false;
	     }
	   else
	     {
	       atLeastOneStringFound = true;
	       QString msg = entry.message(entry.capturedText(line),
					   entry.lineNumber(line),
					   entry.columnNumber(line));

	       if(!*item)
		 *item =  new KListViewItem(rv);
	       KListViewItem* tempItem = new KListViewItem(*item);
	       tempItem->setMultiLinesEnabled(true);
	       tempItem->setText(0,msg);
	       occur ++;
	       entry.updateLine(line);
	       entry.incPos();
	     }

        }
    }
}

void KFileReplacePart::fileSearch(const QString& directoryName, const QString& filters)
{
  QDir d(directoryName);

  d.setMatchAllDirs(true);
  d.setFilter(m_optionMask);

  QStringList filesList = d.entryList(filters);
  QString filePath = d.canonicalPath();
  QStringList::iterator filesIt;
  uint filesNumber = 0;

  m_view->displayScannedFiles(filesNumber);

  for (filesIt = filesList.begin(); filesIt != filesList.end() ; ++filesIt)
    {
      kapp->processEvents();
      // We pushed stop button
      if(m_stop)
        break;

      QString fileName = (*filesIt);

      // Avoids files that not match
      if(!KFileReplaceLib::isAnAccessibleFile(filePath, fileName, m_option))
        continue;

      QFileInfo fileInfo(filePath+"/"+fileName);
      if(fileInfo.isDir())
        continue;

      search(filePath, fileName);
      filesNumber++;
      m_view->displayScannedFiles(filesNumber);
    }
}

void KFileReplacePart::recursiveFileSearch(const QString& directoryName, const QString& filters, uint& filesNumber)
{
  // if m_stop == true then interrupt recursion
  if(m_stop)
    return;
  else
    {
      QDir d(directoryName);

      d.setMatchAllDirs(true);
      d.setFilter(m_optionMask);

      QStringList filesList = d.entryList(filters);
      QString filePath = d.canonicalPath();
      QStringList::iterator filesIt;

      for(filesIt = filesList.begin(); filesIt != filesList.end(); ++filesIt)
        {
          // stop polling
          if(m_stop)
            break;

          QString fileName = (*filesIt);
          // Avoids files that not match
          if(!KFileReplaceLib::isAnAccessibleFile(filePath, fileName, m_option))
            continue;

          // Composes file path string
          QFileInfo fileInfo(filePath+"/"+fileName);

          m_view->displayScannedFiles(filesNumber);

          // Searchs recursively if "filePath" is a directory
          if(fileInfo.isDir())
            recursiveFileSearch(filePath+"/"+fileName, filters, filesNumber);
          else
            {
              kapp->processEvents();
              search(filePath, fileName);
              filesNumber++;
              m_view->displayScannedFiles(filesNumber);
            }
         }
    }
}

void KFileReplacePart::search(const QString& currentDir, const QString& fileName)
{
  QFile file(currentDir+"/"+fileName);

  if(!file.open(IO_ReadOnly))
    {
      KMessageBox::information(m_w, i18n("<qt>Cannot open file <b>%1</b> for reading.</qt>").arg(fileName), QString::null, rcNotifyOnErrors);
      return ;
    }
  // Creates a stream with the file
  QTextStream stream( &file );
  stream.setEncoding(QTextStream::UnicodeUTF8);
  QString line = stream.read();
  file.close();

  QFileInfo fileInfo(currentDir+"/"+fileName);

  KListViewItem *item = 0;
  bool caseSensitive = m_option->m_caseSensitive,
       haltOnFirstOccur = m_option->m_haltOnFirstOccur;

  //Counts occurrences
  int occurrence = 0;

  //This map contains strings to search
  KeyValueMap tempMap = m_replacementMap;
  //If no strings has been found, then no lines will be added to the result view
  bool atLeastOneStringFound = false;

  KeyValueMap::Iterator it = tempMap.begin();

  KListView* rv = m_view->getResultsView();

  while(it != tempMap.end())
    {
      //stop polling
      if(m_stop)
        break;

      QString key = it.key();
      QString strKey;
      QRegExp rxKey;

      if(m_option->m_regularExpressions)
        rxKey = QRegExp("("+key+")", caseSensitive, false);
      else
        strKey = key;
      /* If this option is true then for any string in
      *  the map we search for the first match*/
      if(haltOnFirstOccur)
        {
          int pos;

          if(m_option->m_regularExpressions)
            pos = line.find(rxKey);
          else
            pos = line.find(strKey, 0 ,caseSensitive);

          if(pos != -1)
            {
              atLeastOneStringFound = true;
              int lineNumber = line.mid(0,pos).contains('\n')+1;
              int columnNumber = pos - line.findRev('\n',pos);

              if (!item)
                item = new KListViewItem(rv);

              KListViewItem* tempItem= new KListViewItem(item);
              QString msg,
                      capturedText;

              if(m_option->m_regularExpressions)
                capturedText = rxKey.cap(1);
              else
                capturedText = line.mid(pos,strKey.length());

              msg = i18n(" first captured text \"%1\" at line: %2, column: %3").arg(capturedText).arg(QString::number(lineNumber,10)).arg(QString::number(columnNumber,10));
              tempItem->setMultiLinesEnabled(true);
              tempItem->setText(0,msg);
              occurrence = 1;
            }
        }// ends haltOnFirstOccur if-block
      else
        {
          /* This point of the code is reached when we must search for all
	  *  occurrences of all the strings*/

          int pos = 0;
          if(m_option->m_regularExpressions)
            pos = rxKey.search(line,pos);
          else
            pos = line.find(strKey, pos ,caseSensitive);

          while(pos != -1)
            {
              //stop polling
              if(m_stop)
                break;

              atLeastOneStringFound = true;
              QString msg,
                      capturedText;
              int lineNumber = line.mid(0,pos).contains('\n')+1;
              int columnNumber = pos - line.findRev('\n',pos);

              if(m_option->m_regularExpressions)
                {
                  capturedText = rxKey.cap(1);
                  pos = rxKey.search(line, pos+rxKey.matchedLength());
                }
              else
                {
                  capturedText = line.mid(pos,strKey.length());
                  pos = line.find(strKey,pos+strKey.length());
                }

              msg = i18n(" captured text \"%1\" at line: %2, column: %3").arg(capturedText).arg(QString::number(lineNumber,10)).arg(QString::number(columnNumber,10));

              if(!item)
                 item = new KListViewItem(rv);
              KListViewItem* tempItem = new KListViewItem(item);
              tempItem->setMultiLinesEnabled(true);
              tempItem->setText(0,msg);
              occurrence++;
              //we push stop button
              if(m_stop)
                break;
            }
        }
      //Advances of one position in the strings map
      ++it;

      //we push stop button
      if(m_stop)
        break;
    }

   //if ignoreFiles == false then every files must be show
   if(!m_option->m_ignoreFiles)
     atLeastOneStringFound = true;

   if(atLeastOneStringFound && item)
     {
       KFileReplaceLib::setIconForFileEntry(item,currentDir+"/"+fileName);
       item->setText(0,fileName);
       item->setText(1,currentDir);
       item->setText(2,KFileReplaceLib::formatFileSize(fileInfo.size()));
       item->setText(3,QString::number(occurrence,10));
       item->setText(4,QString("%1[%2]").arg(fileInfo.owner()).arg(fileInfo.ownerId()));
       item->setText(5,QString("%1[%2]").arg(fileInfo.group()).arg(fileInfo.groupId()));
     }
}

void KFileReplacePart::loadViewContent()
{
  /* Maps the content of the strings view to a QMap */
  KeyValueMap tempMap;
  CommandEngine command;

  QListViewItemIterator itlv(m_view->getStringsView());
  while(itlv.current())
    {
      QListViewItem *item = itlv.current();
      if(m_option->m_variables)
        tempMap[item->text(0)] = command.variableValue(item->text(1));
      else
        tempMap[item->text(0)] = item->text(1);
      ++itlv;
    }
  m_replacementMap = tempMap;
}

void KFileReplacePart::loadRulesFile(const QString& fileName)
{
  /* Loads a file with kfr extension.
  *  creates a xml document and browses it*/
  QDomDocument doc("mydocument");
  QFile file(fileName);
  KListView* sv = m_view->getStringsView();

  if(!file.open(IO_ReadOnly))
    {
      KMessageBox::error(m_w, i18n("<qt>Cannot open the file <b>%1</b> and load the string list.</qt>").arg(fileName));
      return ;
    }

  if(!doc.setContent(&file))
    {
      file.close();

      KMessageBox::information(m_w, i18n("<qt>File <b>%1</b> seems not to be written in new kfr format. Remember that old kfr format will be soon abandoned! You can convert your old rules files by simply saving them with kfilereplace.</qt>").arg(fileName),i18n("Warning"));

      KFileReplaceLib::convertOldToNewKFRFormat(fileName, sv);

      return;
    }
  else
    file.close();

  //clears view
  sv->clear();

  QDomElement docElem = doc.documentElement();
  QDomNode n = docElem.firstChild();
  QString searchAttribute = n.toElement().attribute("search").latin1();

  KeyValueMap docMap;

  if(searchAttribute.isNull() || searchAttribute.isEmpty())
    {
      int answer = KMessageBox::warningYesNo(m_w, i18n("<qt>The format of kfr file has been changed. I will try load <b>%1</b>. See KFilereplace manual for details. Please, tell me do you want to load a search-and-replace list of strings?</qt>").arg(fileName),i18n("Warning"));

      if(answer == KMessageBox::Yes)
        searchAttribute = "false";
      else
        searchAttribute = "true";
    }
  //Verifies the search mode
  if(searchAttribute == "true")
    m_option->m_searchingOnlyMode = true;
  else
    m_option->m_searchingOnlyMode = false;

  //Refreshes the view appearances
  m_view->changeView(m_option->m_searchingOnlyMode);
  //Goes to next tag
  n = n.nextSibling();
  //Reads the string list
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
  QStringList fileList = m_option->m_recentStringFileList;
  if(!fileList.contains(fileName))
    {
      fileList.append(fileName);
      ((KRecentFilesAction* ) actionCollection()->action("strings_load_recent"))->setItems(fileList);
      m_option->m_recentStringFileList = fileList;
    }

  m_view->changeView(m_option->m_searchingOnlyMode);

  m_view->loadMap(docMap);

  resetActions();
}

bool KFileReplacePart::launchNewProjectDialog(const KURL & startURL)
{
  /* This dlg reads options from m_option, then execs, finally returns options.*/
  KNewProjectDlg dlg(m_option);

  if(!startURL.isEmpty())
    m_option->m_directories = QString(startURL.path()+","+m_option->m_directories);

  if(!dlg.exec())
    return false;

  dlg.saveRCOptions();

  m_config->sync();

  slotQuickStringsAdd();

  m_view->updateOptions(m_option);

  resetActions();

  return true;
}

void KFileReplacePart::setOptionMask()
{
  m_optionMask |= QDir::Dirs;

  if(!m_option->m_ignoreHidden)
    m_optionMask |= QDir::Hidden;

  if(!m_option->m_followSymLinks)
    m_optionMask |= QDir::NoSymLinks;
}

void KFileReplacePart::stringsInvert(bool invertAll)
{
  QListViewItem* lviCurItem,
               * lviFirst;
  KListView* sv = m_view->getStringsView();

  if(invertAll)
    lviCurItem = lviFirst = sv->firstChild();
  else
    lviCurItem = lviFirst = sv->currentItem();

  if(lviCurItem == 0)
    return ;

  do
    {
      QString searchText = lviCurItem->text(0),
              replaceText = lviCurItem->text(1);

      // Cannot invert the string when search string is empty
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
  m_view->setCurrentStringsViewMap();
}

bool KFileReplacePart::checkBeforeOperation()
{
  loadViewContent();
  KListView* sv = m_view->getStringsView();

  // Checks if there are strings to replace (not need in search operation)
  if (sv->childCount() == 0)
    {
      KMessageBox::error(m_w, i18n("There are no strings to search and replace."));
      return false;
    }

  // Checks if the main directory can be accessed
  QString currentDirectory = QStringList::split(",",m_option->m_directories)[0];
  QDir dir;

  dir.setPath(currentDirectory);
  QString directory = dir.absPath();

  if(!dir.exists())
    {
      KMessageBox::error(m_w, i18n("<qt>The main folder of the project <b>%1</b> does not exist.</qt>").arg(directory));
      return false;
    }

  QFileInfo dirInfo(directory);
  if(!(dirInfo.isReadable() && dirInfo.isExecutable())
       || (!m_option->m_searchingOnlyMode && !m_option->m_simulation && !(dirInfo.isWritable())))
    {
      KMessageBox::error(m_w, i18n("<qt>Access denied in the main folder of the project:<br><b>%1</b></qt>").arg(directory));
      return false;
    }

  // Clears the list view
  m_view->getResultsView()->clear();

  return true;
}

void KFileReplacePart::showSemaphore(QString s)
{
  if(s == "green")
    {
      m_view->m_ledGo->setState(KLed::On);
      m_view->m_ledWait->setState(KLed::Off);
      m_view->m_ledStop->setState(KLed::Off);
    }
  else
  if(s == "yellow")
    {
      m_view->m_ledGo->setState(KLed::Off);
      m_view->m_ledWait->setState(KLed::On);
      m_view->m_ledStop->setState(KLed::Off);
    }
  else
  if(s == "red")
    {
      m_view->m_ledGo->setState(KLed::Off);
      m_view->m_ledWait->setState(KLed::Off);
      m_view->m_ledStop->setState(KLed::On);
    }
}

bool KFileReplacePart::dontAskAgain()
{
  m_config->setGroup("Notification Messages");
  QString dontAskAgain = m_config->readEntry(rcDontAskAgain, "no");
  if(dontAskAgain == "yes")
    return true;
  else
    return false;
}

void KFileReplacePart::whatsThis()
{
  actionCollection()->action("options_backup")->setWhatsThis(optionsBackupWhatthis);
  actionCollection()->action("options_case")->setWhatsThis(optionsCaseWhatthis);
  actionCollection()->action("options_var")->setWhatsThis(optionsVarWhatthis);
  actionCollection()->action("options_recursive")->setWhatsThis(optionsRecursiveWhatthis);
  actionCollection()->action("options_regularexpressions")->setWhatsThis(optionsRegularExpressionsWhatthis);
}

#include "kfilereplacepart.moc"
