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

#undef APP

//qt includes
#include <qdir.h>
#include <qdatastream.h>
#include <qdatetime.h>
#include <qlistview.h>

//kde includes
#include <kaboutkfilereplace.h>
#include <kapplication.h>
#include <kaction.h>
#include <kbugreport.h>
#include <kconfig.h>
#include <kdebug.h>
#include <kfiledialog.h>
#include <kinstance.h>
#include <kio/netaccess.h>
#include <kmessagebox.h>
#include <kparts/genericfactory.h>
#include <kstandarddirs.h>
#include <dcopclient.h>
#include <dcopref.h>

//own includes
#include "apistruct.h"
#include "kfilereplacelib.h"
#include "kernel.h"
#include "resource.h"
#include "kfilereplacepart.h"
#include "kfilereplacedoc.h"
#include "kfilereplaceview.h"
#include "koptionsdlg.h"

#include "whatthis.h"

using namespace whatthisNameSpace;

// Global Thread data
bool g_bThreadRunning = false;
bool g_bThreadMustStop = false;
int g_nFilesRep = 0;
int g_nStringsRep = 0;
int g_nOperation;
RepDirArg g_argu;
QString g_szErrMsg;

// Factory code for KDE 3
typedef KParts::GenericFactory<KFileReplacePart> KFileReplaceFactory;
K_EXPORT_COMPONENT_FACTORY( libkfilereplacepart, KFileReplaceFactory )

KFileReplacePart::KFileReplacePart(QWidget* parentWidget, const char* , QObject* parent, const char* name, const QStringList & )
  : KParts::ReadOnlyPart(parent,name)
{
  setInstance(KFileReplaceFactory::instance());

  // Initialize variables
  g_bThreadRunning = false;
  g_bThreadMustStop = false;
  g_nFilesRep = 0;
  g_szErrMsg = "";
  m_parentWidget = parentWidget;
  QString configName = locateLocal("config", "kfilereplacerc");
  //kdDebug(23000) << "config file: " << configName << endl;
  m_config = new KConfig(configName);
  m_dlgAbout = 0L;

  initView();
  initGUI();
  setWhatsThis();
  readOptions();
  updateCommands(); // Gray or ungray commands
  initDocument();
}

KFileReplacePart::~KFileReplacePart()
{
  saveOptions();
  slotFileStop();
  delete m_doc;
  if(m_dlgAbout != 0)
    delete m_dlgAbout;
  delete m_config;
}

KAboutData* KFileReplacePart::createAboutData()
{
  KAboutData* aboutData = new KAboutData( "kfilereplacepart", I18N_NOOP("KFileReplacePart"),
                                          "1.0", I18N_NOOP( "Batch search and replace tool" ),
                                          KAboutData::License_GPL_V2,
                                          "(C) 1999-2002 Francois Dupoux\n(C) 2003 Andras Mantia");
                                          aboutData->addAuthor("Francois Dupou",I18N_NOOP("Original author of the KFileReplace tool"), "dupoux@dupoux.com");
                                          aboutData->addAuthor("Andras Mantia",I18N_NOOP("Current maintainer, KPart creator"), "amantia@kde.org");

  return aboutData;
}

bool KFileReplacePart::openURL(const KURL &url)
{
  if (url.protocol() != "file")
  {
    KMessageBox::sorry(0, i18n("Sorry, currently the KFileReplace part works only for local files."), i18n("Non Local File"));
    emit canceled("");
    return false;
  }
  if (m_doc->newDocument(url.path(), "*", true))
  {
      m_settings.bRecursive = m_doc->includeSubfolders();
      m_settings.bCaseSensitive = m_doc->caseSensitive();
      m_settings.bWildcards = m_doc->enableWildcards();
      m_settings.bVariables = m_doc->enableVariables();
      m_view->addString(0L, m_doc->searchFor(), m_doc->replaceWith());
      if (!m_doc->searchLater())
      {
          if (m_doc->replaceWith().isEmpty())
            slotFileSearch();
          else
            slotFileReplace();
      }
      return true;
  } else return false;
}

void KFileReplacePart::initGUI()
{
   setXMLFile("kfilereplacepartui.rc");

   // File
   (void)new KAction(i18n("New Search Project..."), "newproject", 0, this, SLOT(slotFileNew()), actionCollection(), "new_project");

   (void)new KAction(i18n("&Search"), "find", 0, this, SLOT(slotFileSearch()), actionCollection(), "search");

   (void)new KAction(i18n("&Simulate"), "filesimulate", 0, this, SLOT(slotFileSimulate()), actionCollection(), "file_simulate");

   (void)new KAction(i18n("&Replace"), "filereplace", 0, this, SLOT(slotFileReplace()), actionCollection(), "replace");

   (void)new KAction(i18n("Sto&p"), "filestop", 0, this, SLOT(slotFileStop()), actionCollection(), "stop");

   (void)new KAction(i18n("Save &Results As..."), "filesave", 0, this, SLOT(slotFileSave()), actionCollection(), "save_results");

   // Strings
   (void)new KAction(i18n("&Add String..."), "edit_add", 0, this, SLOT(slotStringsAdd()), actionCollection(), "strings_add");

   (void)new KAction(i18n("&Delete String"), "edit_remove", 0, this, SLOT(slotStringsDel()), actionCollection(), "strings_del");

   (void)new KAction(i18n("&Empty Strings List"), "strempty", 0, this, SLOT(slotStringsEmpty()), actionCollection(), "strings_empty");

   (void)new KAction(i18n("Edi&t Selected String..."), "lineedit", 0, this, SLOT(slotStringsEdit()), actionCollection(), "strings_edit");

   (void)new KAction(i18n("&Save Strings List to File..."), "filesave", 0, this, SLOT(slotStringsSave()), actionCollection(), "strings_save");

   (void)new KAction(i18n("&Load Strings List From File..."), "unsortedList", 0, this, SLOT(slotStringsLoad()), actionCollection(), "strings_load");

   (void)new KRecentFilesAction(i18n("&Load Recent Strings Files"), "fileopen", 0, this, SLOT(slotOpenRecentStringFile(const KURL&)), actionCollection(),"strings_load_recent");

   (void)new KAction(i18n("&Invert Current String (search <--> replace)"), "invert", 0, this, SLOT(slotStringsInvertCur()), actionCollection(), "strings_invert");

   (void)new KAction(i18n("&Invert All Strings (search <--> replace)"), "invert", 0, this, SLOT(slotStringsInvertAll()), actionCollection(), "strings_invert_all");

   // Options
   (void)new KToggleAction(i18n("&Include Sub-Folders"), "recursive", 0, this, SLOT(slotOptionsRecursive()), actionCollection(), "options_recursive");

   (void)new KToggleAction(i18n("Create &Backup"), "backup", 0, this, SLOT(slotOptionsBackup()), actionCollection(), "options_backup");

   (void)new KToggleAction(i18n("Case &Sensitive"), "casesensitive", 0, this, SLOT(slotOptionsCaseSensitive()), actionCollection(), "options_case");

   KToggleAction* action=new KToggleAction(i18n("Enable &Wildcards"), "optwildcards", 0, this, SLOT(slotOptionsWildcards()), actionCollection(), "options_wildcards");
#if KDE_IS_VERSION(3,2,90)
   action->setCheckedState(i18n("Disable &Wildcards"));
#endif
   action=new KToggleAction(i18n("Enable &Variables in Replace String: [$name:format$]"), "optvar", 0, this, SLOT(slotOptionsVariables()), actionCollection(), "options_var");
#if KDE_IS_VERSION(3,2,90)
   action->setCheckedState(i18n("Disable &Variables in Replace String: [$name:format$]"));
#endif
   (void) new KAction(i18n("Configure &KFileReplace..."), "configure", 0, this, SLOT(slotOptionsPreferences()), actionCollection(), "configure_kfilereplace");

   // Results
   (void)new KAction(i18n("&Properties"), "resfileinfo", 0, m_view, SLOT(slotResultProperties()), actionCollection(), "results_infos");

   (void)new KAction(i18n("&Open"), "resfileopen", 0, m_view, SLOT(slotResultOpen()), actionCollection(), "results_openfile");

   //if(QString(kapp->startupId()).contains("quanta")==0)
   (void)new KAction(i18n("&Open in Quanta"), "resfileedit", 0, m_view, SLOT(slotResultEdit()), actionCollection(), "results_editfile");


   (void)new KAction(i18n("Open Parent &Folder"), "resdiropen", 0, m_view, SLOT(slotResultDirOpen()), actionCollection(), "results_opendir");

   (void)new KAction(i18n("&Delete"), "resfiledel", 0, m_view, SLOT(slotResultDelete()), actionCollection(), "results_delete");

   (void)new KAction(i18n("E&xpand Tree"), 0, m_view, SLOT(slotResultTreeExpand()), actionCollection(), "results_treeexpand");

   (void)new KAction(i18n("&Reduce Tree"), 0, m_view, SLOT(slotResultTreeReduce()), actionCollection(), "results_treereduce");

   // Help menu
//   setHelpMenuEnabled(false);
   (void)new KAction(i18n("&About KFileReplace"), "kfilereplace", 0, this, SLOT(showAboutApplication()), actionCollection(), "about_kfilereplace");

   (void)new KAction(i18n("KFileReplace &Handbook"), "help", 0, this, SLOT(appHelpActivated()), actionCollection(), "help_kfilereplace");

   (void)new KAction(i18n("&Report Bug..."), 0, 0, this, SLOT(reportBug()), actionCollection(), "report_bug");
}

void KFileReplacePart::initDocument()
{
  bool nRes;

  m_doc = new KFileReplaceDoc(m_parentWidget, this);
  m_doc->addView(m_view);
  nRes = m_doc->newDocument(QDir::homeDirPath(), "*", false);
}

void KFileReplacePart::initView()
{
  ////////////////////////////////////////////////////////////////////
  // create the main widget here that is managed by KMainWindow's view-region and
  // connect the widget to your document to display document contents.

  m_view = new KFileReplaceView(m_parentWidget, "view");

  setWidget(m_view);

  m_view->setAcceptDrops(false);

}

KFileReplaceDoc* KFileReplacePart::document() const
{
  return m_doc;
}

KConfig* KFileReplacePart::config()
{
  return m_config;
}

void KFileReplacePart::updateCommands() // Gray or ungray commands
{
  // File
  actionCollection()->action("new_project")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("search")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("file_simulate")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("replace")->setEnabled(!g_bThreadRunning && m_view->stringView()->childCount() > 0);
  actionCollection()->action("save_results")->setEnabled(!g_bThreadRunning && m_view->resultView()->childCount() > 0);
  actionCollection()->action("stop")->setEnabled(g_bThreadRunning);

  // Strings
  actionCollection()->action("strings_add")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("strings_del")->setEnabled(!g_bThreadRunning && m_view->stringView()->childCount() > 0);
  actionCollection()->action("strings_empty")->setEnabled(!g_bThreadRunning && m_view->stringView()->childCount() > 0);
  actionCollection()->action("strings_edit")->setEnabled(!g_bThreadRunning && m_view->stringView()->childCount() > 0);
  actionCollection()->action("strings_save")->setEnabled(!g_bThreadRunning && m_view->stringView()->childCount() > 0);
  actionCollection()->action("strings_load")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("strings_invert")->setEnabled(!g_bThreadRunning && m_view->stringView()->childCount() > 0);
  actionCollection()->action("strings_invert_all")->setEnabled(!g_bThreadRunning && m_view->stringView()->childCount() > 0);

  // Options
  actionCollection()->action("options_recursive")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("options_backup")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("options_case")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("options_wildcards")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("options_var")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("configure_kfilereplace")->setEnabled(!g_bThreadRunning);

  // Results
  actionCollection()->action("results_infos")->setEnabled(!g_bThreadRunning && m_view->resultView()->childCount() > 0);
  actionCollection()->action("results_openfile")->setEnabled(!g_bThreadRunning && m_view->resultView()->childCount() > 0);
  actionCollection()->action("results_editfile")->setEnabled(!g_bThreadRunning && m_view->resultView()->childCount() > 0);
  actionCollection()->action("results_opendir")->setEnabled(!g_bThreadRunning && m_view->resultView()->childCount() > 0);
  actionCollection()->action("results_delete")->setEnabled(!g_bThreadRunning && m_view->resultView()->childCount() > 0);
  actionCollection()->action("results_treeexpand")->setEnabled(!g_bThreadRunning && m_view->resultView()->childCount() > 0);
  actionCollection()->action("results_treereduce")->setEnabled(!g_bThreadRunning && m_view->resultView()->childCount() > 0);

  // Update menus and toolbar
  ((KToggleAction* ) actionCollection()->action("options_recursive"))->setChecked(m_settings.bRecursive);
  ((KToggleAction* ) actionCollection()->action("options_backup"))->setChecked(m_settings.bBackup);
  ((KToggleAction* ) actionCollection()->action("options_case"))->setChecked(m_settings.bCaseSensitive);
  ((KToggleAction* ) actionCollection()->action("options_wildcards"))->setChecked(m_settings.bWildcards);
  ((KToggleAction* ) actionCollection()->action("options_var"))->setChecked(m_settings.bVariables);
}

int KFileReplacePart::checkBeforeOperation(int nTypeOfOperation)
{
  QString strMess;
  QListViewItem* lviCurItem;
  QListViewItem* lviFirst;
  QString strString;
  /*char cFirst, cLast;
  char szString[256];*/
  bool bWildcardsArePresent=false;
  QWidget* w = widget();

  // ================= Check Thread is not running ========================
  if (g_bThreadRunning)
    {
      KMessageBox::error(w, i18n("The replacing operation is already running. You must finish it before."));
      return -1;
    }

  // Check there are strings to replace (not need in search operation)
  if (nTypeOfOperation == OPERATION_REPLACE && m_view->stringView()->childCount() == 0)
    {
      KMessageBox::error(w, i18n("There are no strings to search and replace."));
      return -1;
    }

  // Check the main directory can be accessed
  QDir dir(m_doc->m_strProjectDirectory);

  if (!dir.exists())
    {
      strMess = i18n("<qt>The main folder of the project <b>%1</b> does not exist.</qt>").arg(m_doc->m_strProjectDirectory);
      KMessageBox::error(w, strMess);
      return -1;
    }

  if (::access(m_doc->m_strProjectDirectory.local8Bit(), R_OK | X_OK) == -1)
    {
      strMess = i18n("<qt>Access denied in the main folder of the project:<br><b>%1</b></qt>").arg(m_doc->m_strProjectDirectory);
      KMessageBox::error(w, strMess);
      return -1;
    }

  // ================= Check wildcards properties ========================
  if (m_settings.bWildcards)
    {
      // -- Check first and last char of Searched string is not a wildcard

      //lviCurItem = lviFirst = m_view->stringView()->firstChild();
    /*  if (lviCurItem)
        {
          do
            {
              // Get first char of the string
              strString = lviCurItem->text(0).left(1);
              strncpy(szString, strString.ascii(), 255);
              cFirst = *szString;

              // Get last char of the string
              strString = lviCurItem->text(0).right(1);
              strncpy(szString, strString.ascii(), 255);
              cLast = *szString;*/

              /*if ((cFirst == m_settings.cWildcardsWords) || (cLast == m_settings.cWildcardsWords) || (cFirst == m_settings.cWildcardsLetters) || (cLast == m_settings.cWildcardsLetters))
                {        KMessageBox::error(this, i18n("First and last characters of the searched string cannot be a wildcard."));
                return -1;
                }*/

          /*    lviCurItem = lviCurItem->nextSibling();
            } while(lviCurItem && lviCurItem != lviFirst);
        }*/

      // -- Check Maximum expression length is valid
      if ((m_settings.nMaxExpressionLength < 2) || (m_settings.nMaxExpressionLength > 10000))
        {
           KMessageBox::error(w, i18n("The maximum expression wildcard length is not valid (should be between 2 and 10000)"));
           return -1;
         }

      // check expression wildcard and character wildcards are not the same
      if (m_settings.cWildcardsWords == m_settings.cWildcardsLetters)
        {
           KMessageBox::error(w, i18n("<qt>You cannot use the same character for <b>expression wildcard</b> and for <b>character wildcard</b>.</qt>"));
           return -1;
        }

      // -- If wildcards are enabled, check there are wildcards in the strings, else, desactivate them to optimize
      bWildcardsArePresent = false;

      lviCurItem = lviFirst = m_view->stringView()->firstChild();
      if (lviCurItem)
        {
         do
           {
              QString szString = lviCurItem->text(0);

              if ( szString.contains(m_settings.cWildcardsLetters) || szString.contains(m_settings.cWildcardsWords))
                {
                  // Wildcards are present in at less a string
                  bWildcardsArePresent = true;
                }

              lviCurItem = lviCurItem->nextSibling();
            } while(lviCurItem && lviCurItem != lviFirst);
        }
    }

  /** Prepare argument structure to pass to the replaceDirectory function */

  g_argu.szDir = m_doc->m_strProjectDirectory;
  g_argu.szFilter = m_doc->m_strProjectFilter;
  g_argu.qlvResult = m_view->resultView();
  g_argu.qlvStrings = m_view->stringView();

  g_argu.bMinSize = m_doc->m_bMinSize;
  g_argu.bMaxSize = m_doc->m_bMaxSize;
  g_argu.nMinSize = m_doc->m_nMinSize;
  g_argu.nMaxSize = m_doc->m_nMaxSize;
  g_argu.nTypeOfAccess = m_doc->m_nTypeOfAccess;
  g_argu.bMinDate = m_doc->m_bMinDate;
  g_argu.bMaxDate = m_doc->m_bMaxDate;
  g_argu.qdMinDate = m_doc->m_qdMinDate;
  g_argu.qdMaxDate = m_doc->m_qdMaxDate;

  g_argu.bOwnerUserBool = m_doc->m_bOwnerUserBool;
  g_argu.bOwnerGroupBool = m_doc->m_bOwnerGroupBool;
  g_argu.bOwnerUserMustBe = m_doc->m_bOwnerUserMustBe;
  g_argu.bOwnerGroupMustBe = m_doc->m_bOwnerGroupMustBe;
  g_argu.strOwnerUserType = m_doc->m_strOwnerUserType;
  g_argu.strOwnerGroupType = m_doc->m_strOwnerGroupType;
  g_argu.strOwnerUserValue = m_doc->m_strOwnerUserValue;
  g_argu.strOwnerGroupValue = m_doc->m_strOwnerGroupValue;

  g_argu.bConfirmFiles = m_settings.bConfirmFiles;
  g_argu.bConfirmStrings = m_settings.bConfirmStrings;
  g_argu.bConfirmDirs = m_settings.bConfirmDirs;
  g_argu.bCaseSensitive = m_settings.bCaseSensitive;
  g_argu.bRecursive = m_settings.bRecursive;
  g_argu.bBackup = m_settings.bBackup;
  g_argu.bVariables = m_settings.bVariables;
  g_argu.bFollowSymLinks = m_settings.bFollowSymLinks;
  g_argu.bAllStringsMustBeFound = m_settings.bAllStringsMustBeFound;
  g_argu.bWildcards = m_settings.bWildcards && bWildcardsArePresent;
  g_argu.cWildcardsLetters = m_settings.cWildcardsLetters;
  g_argu.cWildcardsWords = m_settings.cWildcardsWords;
  g_argu.bWildcardsInReplaceStrings = m_settings.bWildcardsInReplaceStrings;
  g_argu.nMaxExpressionLength = m_settings.nMaxExpressionLength;
  g_argu.bHaltOnFirstOccur = m_settings.bHaltOnFirstOccur;
  g_argu.bIgnoreWhitespaces = m_settings.bIgnoreWhitespaces;
  g_argu.bIgnoreHidden = m_settings.bIgnoreHidden;

  g_argu.mainwnd = (QWidget* ) this;
  g_argu.view = m_view;

  // Clear the list view
  m_view->resultView()->clear();

  return 0; // Operation successfully prepared
}

void KFileReplacePart::readOptions()
{
  m_config->setGroup("General Options");

  // Recent files
  m_recentStringFileList = m_config->readListEntry("Recent files");
  ((KRecentFilesAction* ) actionCollection()->action("strings_load_recent"))->setItems(m_recentStringFileList);

  // options seetings (recursivity, backup, case sensitive)
  m_config->setGroup("Options");

  m_settings.bRecursive = m_config->readBoolEntry("Recursivity", RecursiveOption);
  m_settings.bBackup = m_config->readBoolEntry("Backup", BackupOption);
  m_settings.bCaseSensitive = m_config->readBoolEntry("Case sensitive", CaseSensitiveOption);
  m_settings.bWildcards = m_config->readBoolEntry("Wildcards", WildcardsOption);
  m_settings.bVariables = m_config->readBoolEntry("Variables", VariablesOption);
  m_settings.bFollowSymLinks = m_config->readBoolEntry("FollowSymLinks", FollowSymbolicLinksOption);
  m_settings.bAllStringsMustBeFound = m_config->readBoolEntry("AllStringsMustBeFound", AllStringsMustBeFoundOption);
  m_settings.bConfirmFiles = m_config->readBoolEntry("ConfirmFiles", ConfirmFilesOption);
  m_settings.bConfirmStrings = m_config->readBoolEntry("ConfirmStrings", ConfirmStringsOption);
  m_settings.bConfirmDirs = m_config->readBoolEntry("ConfirmDirs", ConfirmDirectoriesOption);
  m_settings.bHaltOnFirstOccur = m_config->readBoolEntry("HaltOnFirstOccur", StopWhenFirstOccurenceOption);
  m_settings.bIgnoreWhitespaces = m_config->readBoolEntry("IgnoreWhitespaces", IgnoreWhiteSpacesOption);
  m_settings.bIgnoreHidden = m_config->readBoolEntry("IgnoreHidden", IgnoreHiddenOption);

  // Wildcards properties
  m_config->setGroup("Wildcards");
  m_settings.bWildcardsInReplaceStrings = m_config->readBoolEntry("WildcardsInReplaceStrings", WildcardsInReplaceStringOption);
  m_settings.cWildcardsLetters = m_config->readNumEntry("WildcardSymbolForLetters", (int) WildcardsLetterOption); // '?'
  m_settings.cWildcardsWords = m_config->readNumEntry("WildcardSymbolForWords", (int) WildcardsWordOption); // '*'
  m_settings.nMaxExpressionLength = m_config->readNumEntry("MaximumExpressionLength", WildcardsExpressionLengthOption);
}

void KFileReplacePart::saveOptions()
{
  m_config->setGroup("General Options");
  // Recent file list
  m_config->writeEntry("Recent files", m_recentStringFileList);

  // options seetings (recursivity, backup, case sensitive)
  m_config->setGroup("Options");

  m_config->writeEntry("Recursivity", m_settings.bRecursive);
  m_config->writeEntry("Backup", m_settings.bBackup);
  m_config->writeEntry("Case sensitive", m_settings.bCaseSensitive);
  m_config->writeEntry("Wildcards", m_settings.bWildcards);
  m_config->writeEntry("Variables", m_settings.bVariables);
  m_config->writeEntry("FollowSymLinks", m_settings.bFollowSymLinks);
  m_config->writeEntry("AllStringsMustBeFound", m_settings.bAllStringsMustBeFound);
  m_config->writeEntry("ConfirmFiles", m_settings.bConfirmFiles);
  m_config->writeEntry("ConfirmStrings", m_settings.bConfirmStrings);
  m_config->writeEntry("ConfirmDirs", m_settings.bConfirmDirs);
  m_config->writeEntry("HaltOnFirstOccur", m_settings.bHaltOnFirstOccur);
  m_config->writeEntry("IgnoreWhitespaces", m_settings.bIgnoreWhitespaces);
  m_config->writeEntry("IgnoreHidden", m_settings.bIgnoreHidden);

  // Wildcards properties
  m_config->setGroup("Wildcards");
  m_config->writeEntry("WildcardsInReplaceStrings", m_settings.bWildcardsInReplaceStrings);
  m_config->writeEntry("WildcardSymbolForLetters", (int) m_settings.cWildcardsLetters);
  m_config->writeEntry("WildcardSymbolForWords", (int) m_settings.cWildcardsWords);
  m_config->writeEntry("MaximumExpressionLength", m_settings.nMaxExpressionLength);

  m_config->sync();
}

void KFileReplacePart::slotFileNew()
{
  emit setStatusBarText(i18n("Creating new document..."));

  if (m_doc->newDocument())
  {
      // Empty lists views
    // m_view->stringView()->clear();
      m_view->resultView()->clear();

      emit setStatusBarText(i18n("Ready."));
      updateCommands();
      m_settings.bRecursive = m_doc->includeSubfolders();
      m_settings.bCaseSensitive = m_doc->caseSensitive();
      m_settings.bWildcards = m_doc->enableWildcards();
      m_settings.bVariables = m_doc->enableVariables();
      m_view->addString(0L, m_doc->searchFor(), m_doc->replaceWith());
      if (!m_doc->searchLater())
      {
          if (m_doc->replaceWith().isEmpty())
            slotFileSearch();
          else
            slotFileReplace();
      }
  }

}

void KFileReplacePart::slotFileSearch()
{
   QString strMess;

   if (checkBeforeOperation(OPERATION_SEARCH) == -1)
      return;

   emit setStatusBarText(i18n("Searching files..."));

   // Run the Replacing operation Thread
   g_bThreadRunning = true;
   g_bThreadMustStop = false;
   g_nFilesRep = 0;
   g_nStringsRep = 0;
   g_nOperation = OPERATION_SEARCH;

   updateCommands();

   // Default error message
   g_szErrMsg = "";

   // show wait cursor
   QApplication::setOverrideCursor( Qt::waitCursor );

   //nRes = pthread_create(&g_threadReplace, NULL, searchThread, (void *) &g_argu);
   //startTimer(100);
   Kernel::instance()->searchThread(&g_argu);

   // restore cursor
   QApplication::restoreOverrideCursor();

   // Show results after operation
   if (g_nFilesRep == -1) // Error
      strMess = i18n("Error while searching/replacing");
   else // Success
   {
      if (!g_argu.bHaltOnFirstOccur) {
         strMess = i18n("%n string found", "%n strings found", g_nStringsRep);
         strMess += i18n(" (in %n file)", " (in %n files)", g_nFilesRep);
      } else
         strMess = i18n("%n file found", "%n files found", g_nFilesRep);
   }

   emit setStatusBarText(strMess); // Show message in status bar
   updateCommands(); // Gray or ungray commands

}

void KFileReplacePart::slotFileReplace()
{
   QString strMess;

   if (checkBeforeOperation(OPERATION_REPLACE) == -1)
      return;

   emit setStatusBarText(i18n("Replacing files..."));

   // Run the Replacing operation Thread
   g_bThreadRunning = true;
   g_bThreadMustStop = false;
   g_nFilesRep = 0;
   g_nStringsRep = 0;
   g_nOperation = OPERATION_REPLACE;

   updateCommands();

   // Default error message
   g_szErrMsg = "";

   // cancel impossible options
   g_argu.bHaltOnFirstOccur = false;

   g_argu.bSimulation = false;

   // show wait cursor
   QApplication::setOverrideCursor( Qt::waitCursor );

   Kernel::instance()->replaceThread(&g_argu);

   // restore cursor
   QApplication::restoreOverrideCursor();

   // Show results after operation
   if (g_nFilesRep == -1) // Error
     strMess = i18n("Error while searching/replacing");
   else // Success
    {
      if (!g_argu.bHaltOnFirstOccur)
        {
          strMess = i18n("%n string successfully replaced", "%n strings successfully replaced", g_nStringsRep);
          strMess += i18n(" (in %n file)", " (in %n files)", g_nFilesRep);
        }
      else
        strMess = i18n("%n file successfully replaced", "%n files successfully replaced", g_nFilesRep);
    }

   emit setStatusBarText(strMess); // Show message in status bar
   updateCommands(); // Gray or ungray commands

}

void KFileReplacePart::slotFileSimulate()
{
   QString strMess;

   if (checkBeforeOperation(OPERATION_REPLACE) == -1)
      return;

   emit setStatusBarText(i18n("Replacing files... (simulation)"));

   // Run the Replacing operation Thread
   g_bThreadRunning = true;
   g_bThreadMustStop = false;
   g_nFilesRep = 0;
   g_nStringsRep = 0;
   g_nOperation = OPERATION_REPLACE;

   updateCommands();

   // Default error message
   g_szErrMsg = "";

   // cancel impossible options
   g_argu.bHaltOnFirstOccur = false;

   g_argu.bSimulation = true;

   // show wait cursor
   QApplication::setOverrideCursor( Qt::waitCursor );

   Kernel::instance()->replaceThread(&g_argu);

   // restore cursor
   QApplication::restoreOverrideCursor();

   // Show results after operation
   if (g_nFilesRep == -1) // Error
     strMess = i18n("Error while searching/replacing");
   else // Success
     {
       if (!g_argu.bHaltOnFirstOccur)
         {
           strMess = i18n("%n string successfully replaced", "%n strings successfully replaced", g_nStringsRep);
           strMess += i18n(" (in %n file)", " (in %n files)", g_nFilesRep);
         }
       else
         strMess = i18n("%n file successfully replaced", "%n files successfully replaced", g_nFilesRep);
     }

   emit setStatusBarText(strMess); // Show message in status bar
   updateCommands(); // Gray or ungray commands
}

void KFileReplacePart::slotFileStop()
{
   if (!g_bThreadRunning) // If no thread running
      return ;

   // Stop the current thread
   g_bThreadMustStop = true;
   updateCommands();

   // restore cursor
   QApplication::restoreOverrideCursor();
}

void KFileReplacePart::slotFileSave()
{
  QString fileName;

  QWidget* w = widget();

  QListView* lvResult = m_view->resultView();

  // Check there are results
  if (lvResult->childCount() == 0)
    {
      KMessageBox::error(w, i18n("There are no results to save: the result list is empty."));
      return ;
    }

  // Select the file where results will be saved
  fileName = KFileDialog::getSaveFileName(QString::null, i18n("*.xhtml|XHTML Files (*.xhtml)\n*|All Files (*)"), w, i18n("Save Results"));
  if (fileName.isEmpty())
    return ;

  // Force the extension to be "html"
  fileName = KFileReplaceLib::instance()->addFilenameExtension(fileName, "xhtml");

  // Save results to file
 // a) Open the file
  QFile fResults(fileName);
  if ( !fResults.open( IO_WriteOnly ) )
    {
      KMessageBox::error(w, i18n("<qt>Cannot open the file <b>%1</b> for writing the save results.</qt>").arg(fileName));
      return ;
    }

    // b) Write header of the XHTML file
   QDateTime datetime = QDateTime::currentDateTime(Qt::LocalTime);
   QString dateString = datetime.toString(Qt::LocalDate);
   QString XHTML = "<?xml version=\"1.0\" ?>\n"
                   "<!DOCTYPE html PUBLIC \"-//W3C//DTD XHTML 1.0 Strict//EN\" \"http://www.w3.org/TR/xhtml1/DTD/xhtml1-strict.dtd\">\n"
                   "<html xmlns=\"http://www.w3.org/1999/xhtml\">\n"
                   "\t<head>\n"
                   "\t\t<title>";
           XHTML += i18n("KFileReplace Results File")+ "</title>\n"
                    "\t\t\t<style type=\"text/css\">\n"
                    "\t\t\t\t.a {background-color : lightblue;}\n"
                    "\t\t\t\t.b {background-color : paleturquoise;}\n"
                    "\t\t\t\tbody { border: solid teal;}\n"
                    "\t\t\t\t.date { text-align:right; color:darkcyan;}\n"
                    "\t\t\t</style>\n"
                    "\t</head>\n"
                    "\t<body>\n"
                    "\t\t<div style=\"background-color:ivory;padding :10px;\">\n"
                    "\t\t\t<table width=\"100%\">\n"
                    "\t\t\t\t<tr>\n"
                    "\t\t\t\t\t<td><h1>";
           XHTML += i18n("KFileReplace Results File")+"</h1></td>\n"
                    "\t\t\t\t\t<td><div class=\"date\">";
           XHTML += i18n("Creation date : ")+dateString+"</div></td>\n"
                    "\t\t\t\t</tr>\n"
                    "\t\t\t</table>\n"
                    "\t\t<div>\n"
                    "\t\t\t<dl>\n";
   QTextStream oTStream( &fResults );
   oTStream << XHTML;

  // c) Write the file list
  QListViewItem* lviCurItem;
  QListViewItem* lviFirst;
  QString strPath;

  lviCurItem = lviFirst = lvResult->firstChild();

  if (lviCurItem == 0)
    return ;

  unsigned int replacedFileNumber = 0;

  QString classValue="a";
  do
    {
      strPath = KFileReplaceLib::instance()->formatFullPath(lviCurItem->text(1), lviCurItem->text(0));
      QString divclassString ="\t\t\t\t<div class=\""+classValue+"\">\n"
                              "\t\t\t\t\t<dt><a href=\"file:"+strPath+"\">file:"+strPath+"</a>\n"
                              "\t\t\t\t\t</dt>\n";
              divclassString += "\t\t\t\t\t<dd>"
                             + i18n("Old size:")
                             +" "
                             + lviCurItem->text(2)
                             +" "
                             +i18n("--> New size:")
                             + " "
                             + lviCurItem->text(3)
                             +" "
                             +i18n("--> Replaced strings:")
                             +" "
                             + lviCurItem->text(4)
                             +"</dd>\n"
                             "\t\t\t\t</div>\n";
      oTStream << divclassString;

      if(classValue == "a")
        classValue = "b";
      else
        classValue = "a";

      replacedFileNumber += lviCurItem->text(4).toInt();

      lviCurItem = lviCurItem->nextSibling();
    } while(lviCurItem && lviCurItem != lviFirst);


  // d) Write the end of the file

   oTStream<<"\t\t\t\t</dl>\n"
             "\t\t\t</div>\n"
             "\t\t\t<div style=\"text-align:right;color:darkcyan\">"
           <<i18n("Number of replaced strings: ")
           <<replacedFileNumber
           <<"</div>\n"
             "\t\t</div>\n"
             "\t</body>\n"
             "</html>\n";
   fResults.close();
   updateCommands();
}

void KFileReplacePart::slotStringsAdd()
{
  m_view->slotStringsAdd();
  updateCommands();
  return;
}

void KFileReplacePart::slotStringsDel()
{
  delete m_view->stringView()->currentItem(); // Remove item from ListView
  updateCommands();
}

void KFileReplacePart::slotStringsEmpty()
{
  m_view->stringView()->clear();
  updateCommands();
}

void KFileReplacePart::slotStringsEdit()
{
  m_view->slotStringsEdit(0L);
  updateCommands();
}

void KFileReplacePart::slotStringsSave()
{
  QWidget* w = widget();

  // Check there are strings in the list
  if (m_view->stringView()->childCount() == 0)
    {
      KMessageBox::error(w, i18n("There are no strings to save in the list."));
      return ;
    }

   QString header("<?xml version=\"1.0\" ?>\n<kfr>"),
           footer("\n</kfr>"),
           body;
   QListViewItem*  lvi = m_view->stringView()->firstChild();

   while( lvi )
     {
       body += "\n\t<replacement>\n\t\t<oldstring><![CDATA[";
       body += lvi->text(0);
       body += "]]></oldstring>\n\t\t";
       body += "<newstring><![CDATA[";
       body += lvi->text(1);
       body += "]]></newstring>\n\t</replacement>";
       lvi = lvi->nextSibling();
     }

   // Select the file where strings will be saved
   QString fileName = KFileDialog::getSaveFileName(QString::null, i18n("*.kfr|KFileReplace Strings (*.kfr)\n*|All Files (*)"), w, i18n("Save Strings to File"));
   if (fileName.isEmpty())
     return;

   // Force the extension to be "kfr" == KFileReplace extension
   fileName = KFileReplaceLib::instance()->addFilenameExtension(fileName, "kfr");

   QFile file( fileName );
   if(!file.open( IO_WriteOnly ))
     {
       KMessageBox::error(w, i18n("File %1 cannot be saved.").arg(fileName));
       return ;
     }
   QTextStream oTStream( &file );
   oTStream << header
            << body
            << footer;
   file.close();
}

void KFileReplacePart::loadStringFile(const QString& fileName)
{
  //load kfr file
  QDomDocument doc( "mydocument" );
  QFile file( fileName );
  if ( !file.open( IO_ReadOnly ) )
    {
      KMessageBox::error(widget(), i18n("<qt>Cannot open the file <b>%1</b> and load the string list.</qt>").arg(fileName));
      return ;
    }
  if ( !doc.setContent( &file ) )
    {
      file.close();
      KMessageBox::information(widget(), i18n("<qt>File <b>%1</b> seems not to be written in new kfr format. Remember that old kfr format will be soon abandoned! You can convert your old rules files by simply saving them with kfilereplace.</qt>").arg(fileName),i18n("Warning"));

      convertOldToNewKFRFormat(fileName,m_view);
      return;

    }
  file.close();

  //clear view
  m_view->stringView()->clear();

  QDomElement docElem = doc.documentElement();

  QDomNode n = docElem.firstChild();
  while( !n.isNull() )
    {
      QDomElement e = n.toElement(); // try to convert the node to an element.
      if( !e.isNull() )
        {
          QListViewItem* lvi = new QListViewItem(m_view->stringView());
          QString oldString = e.firstChild().toElement().text();
          lvi->setText(0,oldString);
          QString newString = e.lastChild().toElement().text();
          lvi->setText(1,newString);
          lvi->setPixmap(0, m_view->iconString());
        }
      n = n.nextSibling();
    }

     // Add file to "load strings form file" menu
  if (!m_recentStringFileList.contains(fileName))
    {
      m_recentStringFileList.append(fileName);
      ((KRecentFilesAction* ) actionCollection()->action("strings_load_recent"))->setItems(m_recentStringFileList);
    }
  updateCommands();
}

void KFileReplacePart::slotStringsLoad()
{
  QString fileName;

  // Select the file to load from
  fileName = KFileDialog::getOpenFileName(QString::null, i18n("*.kfr|KFileReplace Strings (*.kfr)\n*|All Files (*)"), widget(), i18n("Load Strings From File"));

  if(!fileName.isEmpty())
    loadStringFile(fileName);
  updateCommands();
}

void KFileReplacePart::slotStringsInvertCur()
{
  QListViewItem* lviCurItem;
  lviCurItem = m_view->stringView()->currentItem();
  if (lviCurItem == 0)
    return;

  QString searchText, replaceText;
  searchText = lviCurItem->text(0);
  replaceText = lviCurItem->text(1);

  // Cannot invert the string if search string will be empty
  if (replaceText.isEmpty())
    {
      KMessageBox::error(widget(), i18n("<qt>Cannot invert string <b>%1</b>, because the search string would be empty.</qt>").arg(searchText));
      return;
    }

  lviCurItem->setText(0, replaceText);
  lviCurItem->setText(1, searchText);
}

void KFileReplacePart::slotStringsInvertAll()
{
  QListViewItem* lviCurItem,
               * lviFirst;
  int nItemPos;
  QString searchText,
          replaceText,
          strMess;

  nItemPos = 0;
  lviCurItem = lviFirst = m_view->stringView()->firstChild();
  if (lviCurItem == 0)
    return ;

  do
    {
      searchText = lviCurItem->text(0);
      replaceText = lviCurItem->text(1);

      // Cannot invert the string if search string will be empty
      if (replaceText.isEmpty())
        {
          KMessageBox::error(widget(), i18n("<qt>Cannot invert string <b>%1</b>, because the search string would be empty.</qt>").arg(searchText));
          return;
        }

      lviCurItem->setText(0, replaceText);
      lviCurItem->setText(1, searchText);

      lviCurItem = lviCurItem->nextSibling();
    } while(lviCurItem && lviCurItem != lviFirst);
}

void KFileReplacePart::slotOpenRecentStringFile(const KURL& urlFile)
{
  QString fileName;
  QFileInfo fi;

  if (g_bThreadRunning) // Thread running: it has not finished since the last call of this function
    return ;

  // Download file if need (if url is "http://...")
  if (!(KIO::NetAccess::download(urlFile, fileName))) // Open the Archive
    return;

  // Check it's not a directory
  fi.setFile(fileName);
  if (fi.isDir())
    {
      KMessageBox::error(widget(), i18n("Cannot open folders."));
      return;
    }

  loadStringFile(fileName);
  updateCommands();
}

void KFileReplacePart::slotOptionsRecursive()
{
  m_settings.bRecursive = !(m_settings.bRecursive);
  updateCommands();
}

void KFileReplacePart::slotOptionsBackup()
{
  m_settings.bBackup = !(m_settings.bBackup);
  updateCommands();
}

void KFileReplacePart::slotOptionsCaseSensitive()
{
  m_settings.bCaseSensitive = !(m_settings.bCaseSensitive);
  updateCommands();
}

void KFileReplacePart::slotOptionsVariables()
{
  m_settings.bVariables = !(m_settings.bVariables);
  updateCommands();
}

void KFileReplacePart::slotOptionsWildcards()
{
  m_settings.bWildcards = !(m_settings.bWildcards);
  updateCommands();
}

void KFileReplacePart::slotOptionsPreferences()
{
  KOptionsDlg dlg(widget(), 0, m_settings);

  if (dlg.exec() != QDialog::Rejected) // If Cancel
  {
    m_settings = dlg.settings();
    saveOptions();
    updateCommands();
  }
}

void KFileReplacePart::setWhatsThis()
{
  actionCollection()->action("file_simulate")->setWhatsThis(i18n(fileSimulateWhatthis));
  actionCollection()->action("options_wildcards")->setWhatsThis(i18n(optionsWildcardsWhatthis));
  actionCollection()->action("options_backup")->setWhatsThis(i18n(optionsBackupWhatthis));
  actionCollection()->action("options_case")->setWhatsThis(i18n(optionsCaseWhatthis));
  actionCollection()->action("options_var")->setWhatsThis(i18n(optionsVarWhatthis));
  actionCollection()->action("options_recursive")->setWhatsThis(i18n(optionsRecursiveWhatthis));
}

void KFileReplacePart::reportBug()
{
  KAboutData aboutData("kfilereplace", I18N_NOOP("KFileReplace"), "1.0");
  KBugReport dlg(widget(), true, &aboutData);
  dlg.exec();
}

void KFileReplacePart::appHelpActivated()
{
  kapp->invokeHelp(QString::null, "kfilereplace");
}

void KFileReplacePart::showAboutApplication()
{
  m_dlgAbout = new KAboutKFileReplace(widget(), 0, false);
  if(m_dlgAbout == 0)
    return;

  if(!m_dlgAbout->isVisible())
    m_dlgAbout->show();
  else
    m_dlgAbout->raise();

  // Update menu & toolbar commands
  updateCommands();
}

void KFileReplacePart::convertOldToNewKFRFormat(const QString& fileName, KFileReplaceView* view)
{
 //this method convert old format in new XML-based format
 //if is an old format file try to open it
 typedef struct
 {
   char szPgm[13]; // Must be "KFileReplace" : like MZ for EXE files
   int nStringsNb; // Number of strings in file
   char cReserved[64]; // Reserved for future use
 } KFRHeader;

 KFRHeader head;

 FILE* f = fopen(fileName.ascii(),"rb");
 if(!f)
 {
  KMessageBox::error(widget(), i18n("<qt>Cannot open the file <b>%1</b> and load the string list.</qt>").arg(fileName));
  return ;
 }
 int err = fread(&head, sizeof(KFRHeader), 1, f);
 if(err != 1)
 {
  KMessageBox::error(widget(), i18n("<qt>Cannot open the file <b>%1</b> and load the string list.</qt>").arg(fileName));
  return ;
 }
 QString szPgm(head.szPgm);
 if (szPgm != "KFileReplace")
 {
  KMessageBox::error(widget(), i18n("<qt>Cannot open the file <b>%1</b> and load the string list. This file seems not to be a valid old kfr file.</qt>").arg(fileName));
  return ;
 }

  view->stringView()->clear();

  int nOldTextSize,
      nNewTextSize;
  int nErrors = 0;
  int nStrSize;
  QStringList l;

  int i ;
  for (i=0; i < head.nStringsNb; i++)
    {
      nErrors += (fread(&nOldTextSize, sizeof(int), 1, f)) != 1;
      nErrors += (fread(&nNewTextSize, sizeof(int), 1, f)) != 1;
      if (nErrors > 0)
        {
          KMessageBox::error(widget(), i18n("<qt>Cannot read data.</qt>"));
        }
      else
        {
          nStrSize = ((nOldTextSize > nNewTextSize) ? nOldTextSize : nNewTextSize) + 2;
          char* oldString = (char*) malloc(nStrSize+10),
              * newString = (char*) malloc(nStrSize+10);
          memset(oldString, 0, nStrSize);
          memset(newString,0, nStrSize);
          if (oldString == 0 || newString == 0)
            KMessageBox::error(widget(), i18n("<qt>Out of memory.</qt>"));

          else
            {
              if (fread(oldString, nOldTextSize, 1, f) != 1)
                KMessageBox::error(widget(), i18n("<qt>Cannot read data.</qt>"));

              else
                {
                  if (nNewTextSize > 0) // If there is a Replace text
                    {
                      if (fread(newString, nNewTextSize, 1, f) != 1)
                        KMessageBox::error(widget(), i18n("<qt>Cannot read data.</qt>"));

                      else
                        {
                          //
                          QListViewItem* lvi = new QListViewItem(view->stringView());
                          lvi->setText(0,oldString);
                          lvi->setText(1,newString);
                          lvi->setPixmap(0, view->iconString());

                          if(newString)
                            free(newString);
                          if(oldString)
                            free(oldString);
                        }
                    }
                }
            }
        }
    }
    fclose(f);
    return ;
 }

#include "kfilereplacepart.moc"
