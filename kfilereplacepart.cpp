//
//
// C++ Implementation: kfilereplacepart
//
// Description:
//
//
// Author: Andras Mantia <amantia@freemail.hu>, (C) 2003
//
// Copyright: GPL v2. See COPYING file that comes with this distribution
//
//

#undef APP

// general unix includes
#include <unistd.h>

//qt includes
#include <qdir.h>
#include <qwidget.h>

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

//own includes
#include "apistruct.h"
#include "filelib.h"
#include "kernel.h"
#include "resource.h"
#include "kfilereplacepart.h"
#include "kfilereplacedoc.h"
#include "kfilereplaceview.h"
#include "klistviewstring.h"
#include "koptionsdlg.h"

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
K_EXPORT_COMPONENT_FACTORY( libkfilereplacepart, KFileReplaceFactory );

KFileReplacePart::KFileReplacePart(QWidget *parentWidget, const char *, QObject *parent, const char *name, const QStringList & )
  : KParts::ReadOnlyPart(parent,name)
{
  setInstance(KFileReplaceFactory::instance());

  // Initialize variables
  g_bThreadRunning = false;
  g_bThreadMustStop = false;
  g_nFilesRep = 0;
  g_szErrMsg = "";
  m_parentWidget = parentWidget;
  m_config = new KConfig(locateLocal("config", "kfilereplacerc"));
  m_dlgAbout = 0L;

  initDocument();
  initView();
  initGUI();
  setWhatsThis();
  readOptions();
  updateCommands(); // Gray or ungray commands

}

KFileReplacePart::~KFileReplacePart()
{
  m_config->sync();
  slotFileStop();
}

KAboutData* KFileReplacePart::createAboutData()
{
  KAboutData* aboutData =
              new KAboutData( "kfilereplacepart", I18N_NOOP("KFileReplacePart"),
              "1.0", I18N_NOOP( "Batch search and replace tool" ),
              KAboutData::License_GPL_V2,
              "(C) 1999-2002 Francois Dupoux\n(C) 2003 Andras Mantia");
   aboutData->addAuthor("Francois Dupou",I18N_NOOP("Original author of the KFileReplace tool"), "dupoux@dupoux.com");
   aboutData->addAuthor("Andras Mantia",I18N_NOOP("Current maintainer, KPart creator"), "amantia@freemail.hu");

    return aboutData;
}

bool KFileReplacePart::openURL(const KURL &url)
{
  if (url.protocol() != "file")
  {
    KMessageBox::sorry(0, i18n("Sorry, currently the KFileReplace part works only for local files!"), i18n("Non local file"));
    emit canceled("");
    return false;
  }
  return m_doc->newDocument(url.path(), "*", true);
}

void KFileReplacePart::initGUI()
{
   setXMLFile("kfilereplacepartui.rc");

   // File
   (void)new KAction(i18n("New Search Project"), "newproject", 0, this, SLOT(slotFileNew()), actionCollection(), "new_project");
   (void)new KAction(i18n("&Search"), "filesearch", 0, this, SLOT(slotFileSearch()), actionCollection(), "search");
   (void)new KAction(i18n("&Simulate"), "filesimulate", 0, this, SLOT(slotFileSimulate()), actionCollection(), "file_simulate");
   (void)new KAction(i18n("&Replace"), "filereplace", 0, this, SLOT(slotFileReplace()), actionCollection(), "replace");
   (void)new KAction(i18n("Sto&p"), "filestop", 0, this, SLOT(slotFileStop()), actionCollection(), "stop");
   (void)new KAction(i18n("Save &results as..."), "filesave", 0, this, SLOT(slotFileSave()), actionCollection(), "save_results");

   // Strings
   (void)new KAction(i18n("&Add string"), "stradd", 0, this, SLOT(slotStringsAdd()), actionCollection(), "strings_add");
   (void)new KAction(i18n("&Delete string"), "strdel", 0, this, SLOT(slotStringsDel()), actionCollection(), "strings_del");
   (void)new KAction(i18n("&Empty strings list"), "strempty", 0, this, SLOT(slotStringsEmpty()), actionCollection(), "strings_empty");
   (void)new KAction(i18n("Edi&t selected string"), "stredit", 0, this, SLOT(slotStringsEdit()), actionCollection(), "strings_edit");
   (void)new KAction(i18n("&Save strings list to file..."), "filesave", 0, this, SLOT(slotStringsSave()), actionCollection(), "strings_save");
   (void)new KAction(i18n("&Load strings list from file..."), "fileopen", 0, this, SLOT(slotStringsLoad()), actionCollection(), "strings_load");
   (void)new KRecentFilesAction(i18n("&Load recent strings files..."), "fileopen", 0, this, SLOT(slotOpenRecentStringFile(const KURL&)), actionCollection(),
                                "strings_load_recent");
   (void)new KAction(i18n("&Invert current string (search <--> replace)"), "strinvert", 0, this, SLOT(slotStringsInvertCur()), actionCollection(), "strings_invert");
   (void)new KAction(i18n("&Invert all strings (search <--> replace)"), "strinvert", 0, this, SLOT(slotStringsInvertAll()), actionCollection(), "strings_invert_all");

   // Options
   (void)new KToggleAction(i18n("&Include sub-folders"), "optrecursive", 0, this, SLOT(slotOptionsRecursive()), actionCollection(), "options_recursive");
   (void)new KToggleAction(i18n("Create &backup"), "optbackup", 0, this, SLOT(slotOptionsBackup()), actionCollection(), "options_backup");
   (void)new KToggleAction(i18n("Case &sensitive"), "optcase", 0, this, SLOT(slotOptionsCaseSensitive()), actionCollection(), "options_case");
   (void)new KToggleAction(i18n("Enable &Wildcards"), "optwildcards", 0, this, SLOT(slotOptionsWildcards()), actionCollection(), "options_wildcards");
   (void)new KToggleAction(i18n("Enable &Variables in replace string: [$name:format$]"), "optvar", 0, this, SLOT(slotOptionsVariables()), actionCollection(), "options_var");
   (void) new KAction(i18n("Configure &KFileReplace"), "configure", 0, this, SLOT(slotOptionsPreferences()), actionCollection(), "configure_kfilereplace");

   // Results
   (void)new KAction(i18n("&Properties"), "resfileinfo", 0, getResultView(), SLOT(slotResultProperties()), actionCollection(), "results_infos");
   (void)new KAction(i18n("&Open"), "resfileopen", 0, getResultView(), SLOT(slotResultOpen()), actionCollection(), "results_openfile");
   (void)new KAction(i18n("&Edit with kate"), "resfileedit", 0, getResultView(), SLOT(slotResultEdit()), actionCollection(), "results_editfile");
   (void)new KAction(i18n("Open parent &folder"), "resdiropen", 0, getResultView(), SLOT(slotResultDirOpen()), actionCollection(), "results_opendir");
   (void)new KAction(i18n("&Delete"), "resfiledel", 0, getResultView(), SLOT(slotResultDelete()), actionCollection(), "results_delete");
   (void)new KAction(i18n("E&xpand tree"), 0, getResultView(), SLOT(slotResultTreeExpand()), actionCollection(), "results_treeexpand");
   (void)new KAction(i18n("&Reduce tree"), 0, getResultView(), SLOT(slotResultTreeReduce()), actionCollection(), "results_treereduce");

   // Help menu
//   setHelpMenuEnabled(false);
   (void)new KAction(i18n("&About KFileReplace"), "kfilereplace", 0, this, SLOT(showAboutApplication()), actionCollection(), "about_kfilereplace");
   (void)new KAction(i18n("KFileReplace &Handbook"), "help", 0, this, SLOT(appHelpActivated()), actionCollection(), "help_kfilereplace");
   (void)new KAction(i18n("&Report Bug..."), 0, 0, this, SLOT(reportBug()), actionCollection(), "report_bug");
}



void KFileReplacePart::initDocument()
{
  int nRes;

  m_doc = new KFileReplaceDoc(m_parentWidget, this);
  nRes = m_doc->newDocument(QDir::homeDirPath(), "*", false);
}


// ===========================================================================================================================
void KFileReplacePart::initView()
{
  ////////////////////////////////////////////////////////////////////
  // create the main widget here that is managed by KMainWindow's view-region and
  // connect the widget to your document to display document contents.

  m_view = new KFileReplaceView(m_parentWidget, "view");

  m_doc->addView(m_view);
  setWidget(m_view);

  m_view -> init();
  m_view -> setAcceptDrops(false);

  //QString caption=kapp->caption();
  //setCaption(caption+": "+m_doc->getTitle());
}

// ===========================================================================================================================
KFileReplaceDoc* KFileReplacePart::getDocument() const
{
  return m_doc;
}

// ===========================================================================================================================
KResultView *KFileReplacePart::getResultView()
{
  return m_view->getResultView();
}


void KFileReplacePart::updateCommands() // Gray or ungray commands
{
  // File
  actionCollection()->action("new_project")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("search")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("file_simulate")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("replace")->setEnabled(!g_bThreadRunning && m_view -> getStringView() -> childCount() > 0);
  actionCollection()->action("save_results")->setEnabled(!g_bThreadRunning && m_view -> getResultView() -> childCount() > 0);
  actionCollection()->action("stop")->setEnabled(g_bThreadRunning);

  // Strings
  actionCollection()->action("strings_add")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("strings_del")->setEnabled(!g_bThreadRunning && m_view -> getStringView() -> childCount() > 0);
  actionCollection()->action("strings_empty")->setEnabled(!g_bThreadRunning && m_view -> getStringView() -> childCount() > 0);
  actionCollection()->action("strings_edit")->setEnabled(!g_bThreadRunning && m_view -> getStringView() -> childCount() > 0);
  actionCollection()->action("strings_save")->setEnabled(!g_bThreadRunning && m_view -> getStringView() -> childCount() > 0);
  actionCollection()->action("strings_load")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("strings_invert")->setEnabled(!g_bThreadRunning && m_view -> getStringView() -> childCount() > 0);
  actionCollection()->action("strings_invert_all")->setEnabled(!g_bThreadRunning && m_view -> getStringView() -> childCount() > 0);

  // Options
  actionCollection()->action("options_recursive")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("options_backup")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("options_case")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("options_wildcards")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("options_var")->setEnabled(!g_bThreadRunning);
  actionCollection()->action("configure_kfilereplace")->setEnabled(!g_bThreadRunning);

  // Results
  actionCollection()->action("results_infos")->setEnabled(!g_bThreadRunning && m_view -> getResultView() -> childCount() > 0);
  actionCollection()->action("results_openfile")->setEnabled(!g_bThreadRunning && m_view -> getResultView() -> childCount() > 0);
  actionCollection()->action("results_editfile")->setEnabled(!g_bThreadRunning && m_view -> getResultView() -> childCount() > 0);
  actionCollection()->action("results_opendir")->setEnabled(!g_bThreadRunning && m_view -> getResultView() -> childCount() > 0);
  actionCollection()->action("results_delete")->setEnabled(!g_bThreadRunning && m_view -> getResultView() -> childCount() > 0);
  actionCollection()->action("results_treeexpand")->setEnabled(!g_bThreadRunning && m_view -> getResultView() -> childCount() > 0);
  actionCollection()->action("results_treereduce")->setEnabled(!g_bThreadRunning && m_view -> getResultView() -> childCount() > 0);

  // Update menus and toolbar
  ((KToggleAction *) actionCollection()->action("options_recursive"))->setChecked(m_settings.bRecursive);
  ((KToggleAction *) actionCollection()->action("options_backup"))->setChecked(m_settings.bBackup);
  ((KToggleAction *) actionCollection()->action("options_case"))->setChecked(m_settings.bCaseSensitive);
  ((KToggleAction *) actionCollection()->action("options_wildcards"))->setChecked(m_settings.bWildcards);
  ((KToggleAction *) actionCollection()->action("options_var"))->setChecked(m_settings.bVariables);
}


// ===========================================================================================================================
int KFileReplacePart::checkBeforeOperation(int nTypeOfOperation)
{
  QString strMess;
  QListViewItem *lviCurItem;
  QListViewItem *lviFirst;
  QString strString;
  char cFirst, cLast;
  char szString[256];
  bool bWildcardsArePresent=false;
  QWidget *w = widget();

  // ================= Check Thread is not running ========================
  if (g_bThreadRunning)
    {
      KMessageBox::error(w, i18n("The replacing operation is already running. You must finish it before."));
      return -1;
    }

  // Check there are strings to replace (not need in search operation)
  if (nTypeOfOperation == OPERATION_REPLACE && m_view -> getStringView() -> childCount() == 0)
    {
      KMessageBox::error(w, i18n("There are no strings to search and replace."));
      return -1;
    }

  // Check the main directory can be accessed
  QDir dir(m_doc->m_strProjectDirectory);

  if (dir.exists() == false)
    {
      strMess = i18n("<qt>The main directory of the project <b>%1</b> doesn't exists! Can't continue.</qt>").arg(m_doc->m_strProjectDirectory);
      KMessageBox::error(w, strMess);
      return -1;
    }

  if (::access(m_doc->m_strProjectDirectory.local8Bit(), R_OK | X_OK) == -1)
    {
      strMess = i18n("<qt>Access denied in the main directory of the project:<br><b>%1</b></qt>").arg(m_doc->m_strProjectDirectory);
      KMessageBox::error(w, strMess);
      return -1;
    }

  // ================= Check wildcards properties ========================
  if (m_settings.bWildcards)
    {
      // -- Check first and last char of Searched string is not a wildcard

      lviCurItem = lviFirst = m_view -> getStringView() -> firstChild();
      if (lviCurItem)
            {
          do
            {
              // Get first char of the string
              strString = lviCurItem -> text(0).left(1);
              strncpy(szString, strString.ascii(), 255);
              cFirst = *szString;

              // Get last char of the string
              strString = lviCurItem -> text(0).right(1);
              strncpy(szString, strString.ascii(), 255);
              cLast = *szString;

              /*if ((cFirst == m_settings.cWildcardsWords) || (cLast == m_settings.cWildcardsWords) || (cFirst == m_settings.cWildcardsLetters) || (cLast == m_settings.cWildcardsLetters))
                {        KMessageBox::error(this, i18n("First and last characters of the searched string cannot be a wildcard."));
                return -1;
                }*/

              lviCurItem = lviCurItem -> nextSibling();
            } while(lviCurItem && lviCurItem != lviFirst);
        }

      // -- Check Maximum expression length is valid
      if ((m_settings.nMaxExpressionLength < 2) || (m_settings.nMaxExpressionLength > 10000))
            {
          KMessageBox::error(w, i18n("The maximum expression wildcard length is not valid (beetween 2 and 10000)"));
          return -1;
            }

      // check expression wildcard and character wildcards are not the same
      if (m_settings.cWildcardsWords == m_settings.cWildcardsLetters)
        {
          KMessageBox::error(w, i18n("<qt>You can't use the same character for <b>expression wildcard</b> and for <b>character wildcard</b></qt>"));
          return -1;
        }

      // -- If wildcards are enabled, check there are wildcards in the strings, else, desactivate them to optimize
      bWildcardsArePresent = false;

      lviCurItem = lviFirst = m_view -> getStringView() -> firstChild();
      if (lviCurItem)
            {
          do
            {
              QString szString = lviCurItem -> text(0);

              if ( szString.contains(m_settings.cWildcardsLetters) || szString.contains(m_settings.cWildcardsWords))
                {
                  // Wildcards are present in at less a string
                  bWildcardsArePresent = true;
                }

              lviCurItem = lviCurItem -> nextSibling();
            } while(lviCurItem && lviCurItem != lviFirst);

          /*if (bWildcardsArePresent == false) // Wildcards options is enabled, but there are no wildcards
            {        bWildcards = false;
            fprintf(stderr, "checkBeforeOperation(): Wildcards are not used then disabled\n");
            }*/
        }
    }

  // ================== Prepare argument structure to pass to the ReplaceDirectory function ========================

  g_argu.szDir = m_doc -> m_strProjectDirectory;
  g_argu.szFilter = m_doc -> m_strProjectFilter;
  g_argu.qlvResult = m_view -> getResultView();
  g_argu.qlvStrings = m_view -> getStringView();

  g_argu.bMinSize = m_doc -> m_bMinSize;
  g_argu.bMaxSize = m_doc -> m_bMaxSize;
  g_argu.nMinSize = m_doc -> m_nMinSize;
  g_argu.nMaxSize = m_doc -> m_nMaxSize;
  g_argu.nTypeOfAccess = m_doc -> m_nTypeOfAccess;
  g_argu.bMinDate = m_doc -> m_bMinDate;
  g_argu.bMaxDate = m_doc -> m_bMaxDate;
  g_argu.qdMinDate = m_doc -> m_qdMinDate;
  g_argu.qdMaxDate = m_doc -> m_qdMaxDate;

  g_argu.bOwnerUserBool = m_doc -> m_bOwnerUserBool;
  g_argu.bOwnerGroupBool = m_doc -> m_bOwnerGroupBool;
  g_argu.bOwnerUserMustBe = m_doc -> m_bOwnerUserMustBe;
  g_argu.bOwnerGroupMustBe = m_doc -> m_bOwnerGroupMustBe;
  g_argu.strOwnerUserType = m_doc -> m_strOwnerUserType;
  g_argu.strOwnerGroupType = m_doc -> m_strOwnerGroupType;
  g_argu.strOwnerUserValue = m_doc -> m_strOwnerUserValue;
  g_argu.strOwnerGroupValue = m_doc -> m_strOwnerGroupValue;

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

  g_argu.mainwnd = (QWidget *) this;
  g_argu.view = m_view;

  // Clear the list view
  m_view -> getResultView() -> clear();

  return 0; // Operation successfully prepared
}

// ===========================================================================================================================
void KFileReplacePart::readOptions()
{
  m_config->setGroup("General Options");

  // bar status settings
  /*
    bool bViewToolbar = m_config->readBoolEntry("Show Toolbar", true);
    m_view_menu->setItemChecked(ID_VIEW_TOOLBAR, bViewToolbar);
    if(!bViewToolbar)
    enableToolBar(KToolBar::Hide);

    bool bViewStatusbar = m_config->readBoolEntry("Show Statusbar", true);
    m_view_menu->setItemChecked(ID_VIEW_STATUSBAR, bViewStatusbar);
    if(!bViewStatusbar)
    enableStatusBar(KStatusBar::Hide);

    KToolBar::BarPosition tool_bar_pos;
    tool_bar_pos=(KToolBar::BarPosition)m_config->readNumEntry("ToolBarPos", KToolBar::Top);
    toolBar()->setBarPos(tool_bar_pos);
  */

  // Recent files
  m_recentStringFileList = m_config->readListEntry("Recent files");
  ((KRecentFilesAction *) actionCollection()->action("strings_load_recent"))->setItems(m_recentStringFileList);

  // options seetings (recursivity, backup, case sensitive)
  m_config->setGroup("Options");

  m_settings.bRecursive = m_config->readBoolEntry("Recursivity", OPTIONS_DEFAULT_RECURSIVE);
  m_settings.bBackup = m_config->readBoolEntry("Backup", OPTIONS_DEFAULT_BACKUP);
  m_settings.bCaseSensitive = m_config->readBoolEntry("Case sensitive", OPTIONS_DEFAULT_CASESENSITIVE);
  m_settings.bWildcards = m_config->readBoolEntry("Wildcards", OPTIONS_DEFAULT_WILDCARDS);
  m_settings.bVariables = m_config->readBoolEntry("Variables", OPTIONS_DEFAULT_VARIABLES);
  m_settings.bFollowSymLinks = m_config->readBoolEntry("FollowSymLinks", OPTIONS_DEFAULT_FOLLOWSYMLINKS);
  m_settings.bAllStringsMustBeFound = m_config->readBoolEntry("AllStringsMustBeFound", OPTIONS_DEFAULT_ALLSTRINGSMUSTBEFOUND);
  m_settings.bConfirmFiles = m_config->readBoolEntry("ConfirmFiles", OPTIONS_DEFAULT_CONFIRMFILES);
  m_settings.bConfirmStrings = m_config->readBoolEntry("ConfirmStrings", OPTIONS_DEFAULT_CONFIRMSTRINGS);
  m_settings.bConfirmDirs = m_config->readBoolEntry("ConfirmDirs", OPTIONS_DEFAULT_CONFIRMDIRS);
  m_settings.bHaltOnFirstOccur = m_config->readBoolEntry("HaltOnFirstOccur", OPTIONS_DEFAULT_HALTONFIRSTOCCUR);
  m_settings.bIgnoreWhitespaces = m_config->readBoolEntry("IgnoreWhitespaces", OPTIONS_DEFAULT_IGNOREWHITESPACES);
  m_settings.bIgnoreHidden = m_config->readBoolEntry("IgnoreHidden", OPTIONS_DEFAULT_IGNOREHIDDEN);

  // Wildcards properties
  m_config->setGroup("Wildcards");
  m_settings.bWildcardsInReplaceStrings = m_config->readBoolEntry("WildcardsInReplaceStrings", OPTIONS_DEFAULT_WILDCARDSINREPLACESTR);
  m_settings.cWildcardsLetters = m_config->readNumEntry("WildcardSymbolForLetters", (int) OPTIONS_DEFAULT_SYMBOLFORWILDCARDSLETTERS); // '?'
  m_settings.cWildcardsWords = m_config->readNumEntry("WildcardSymbolForWords", (int) OPTIONS_DEFAULT_SYMBOLFORWILDCARDSWORDS); // '*'
  m_settings.nMaxExpressionLength = m_config->readNumEntry("MaximumExpressionLength", OPTIONS_DEFAULT_MAXIMUMWILDCARDEXPLENGTH);
}


// ===========================================================================================================================
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


//===========================================================================================================================
void KFileReplacePart::slotFileNew()
{
  int nRes;
  emit setStatusBarText(i18n("Creating new document..."));

  nRes = m_doc->newDocument();
  if (nRes == false)
    return;

  // Empty lists views
  m_view -> getStringView() -> clear();
  m_view -> getResultView() -> clear();

  emit setStatusBarText(i18n("Ready."));
  updateCommands();
}

// ===========================================================================================================================
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

   //nRes = pthread_create(&g_threadReplace, NULL, SearchThread, (void *) &g_argu);
   //startTimer(100);
   SearchThread( (void *) &g_argu );

   // restore cursor
   QApplication::restoreOverrideCursor();

   // Show results after operation
   if (g_nFilesRep == -1) // Error
      strMess.sprintf(i18n("Error while searching/replacing"));
   else // Success
   {
      if (!g_argu.bHaltOnFirstOccur)
         strMess = i18n("%1 strings successfully found in %2 files").arg( g_nStringsRep).arg(g_nFilesRep);
      else
         strMess = i18n("%1 files successfully found").arg(g_nFilesRep);
   }

   emit setStatusBarText(strMess); // Show message in status bar
   updateCommands(); // Gray or ungray commands

   /*if (nRes != 0) // Can't create Thread
     {        slotStatusMsg(i18n("Can't create thread to run the replace operation."));
     return ;
     }
     else // if success
     {        startTimer(100);
     }*/

}

// ===========================================================================================================================
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

   //nRes = pthread_create(&g_threadReplace, NULL, ReplaceThread, (void *) &g_argu);
   //startTimer(100);
   ReplaceThread((void *) &g_argu);

   // restore cursor
   QApplication::restoreOverrideCursor();

   // Show results after operation
   if (g_nFilesRep == -1) // Error
      strMess = i18n("Error while searching/replacing");
   else // Success
   {
      if (!g_argu.bHaltOnFirstOccur)
         strMess = i18n("One string successfully replaced in one file", "%1 strings successfully replaced in %2 files").arg(g_nStringsRep).arg(g_nFilesRep);
      else
         strMess = i18n("One file successfully replaced", "%1 files successfully replaced").arg(g_nFilesRep);
   }

   emit setStatusBarText(strMess); // Show message in status bar
   updateCommands(); // Gray or ungray commands


   /*if (nRes != 0) // Can't create Thread
     {        slotStatusMsg(i18n("Can't create thread to run the replace operation."));
     return ;
     }
     else // if success
     {        startTimer(100);
     } */
}

// ===========================================================================================================================
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

   //nRes = pthread_create(&g_threadReplace, NULL, ReplaceThread, (void *) &g_argu);
   //startTimer(100);
   ReplaceThread((void *) &g_argu);

   // restore cursor
   QApplication::restoreOverrideCursor();

   // Show results after operation
   if (g_nFilesRep == -1) // Error
      strMess = i18n("Error while searching/replacing");
   else // Success
   {
      if (!g_argu.bHaltOnFirstOccur)
         strMess = i18n("One string successfully replaced in one file","%1 strings successfully replaced in %2 files").arg(g_nStringsRep).arg(g_nFilesRep);
      else
         strMess = i18n("One file successfully replaced", "%1 files successfully replaced").arg(g_nFilesRep);
   }

   emit setStatusBarText(strMess); // Show message in status bar
   updateCommands(); // Gray or ungray commands

   /*if (nRes != 0) // Can't create Thread
     {        slotStatusMsg(i18n("Can't create thread to run the replace operation."));
     return ;
     }
     else // if success
     {        startTimer(100);
     } */
}

// ===========================================================================================================================
void KFileReplacePart::slotFileStop()
{
   if (g_bThreadRunning == false) // If no thread running
      return ;

   // Stop the current thread
   g_bThreadMustStop = true;
   updateCommands();

   // restore cursor
   QApplication::restoreOverrideCursor();
}

// ===========================================================================================================================
void KFileReplacePart::slotFileSave()
{
  QString strFilename;
  QListView *lvResult;
  QWidget *w = widget();

  lvResult = m_view -> getResultView();

  // Check there are results
  if (lvResult -> childCount() == 0)
    {
      KMessageBox::error(w, i18n("There are no results to save: the result list is empty."));
      return ;
    }

  // Select the file where results will be saved
  strFilename = KFileDialog::getSaveFileName(QString::null, i18n("*.html|HTML files (*.html)\n*|All Files (*)"), w, i18n("Save results ..."));
  if (strFilename.isEmpty())
    return ;

  // Force the extension to be "html"
  strFilename = addFilenameExtension(strFilename, "html");

  // Save results to file

  //TODO: Use QFile
  // a) Open the file
  FILE *fResults;
  fResults = fopen(strFilename.ascii(),"w");

  if (fResults == NULL) // Can't open file
        {
          KMessageBox::error(w, i18n("<qt>Can't open the file <b>%1</b> for writing the save results.</qt>").arg(strFilename));
          return ;
        }

  // b) Write header of the HTML file
  fprintf(fResults, "<HTML><HEAD>\n");
  fprintf(fResults, "<!DOCTYPE %s>\n", i18n("KFileReplace Results File").ascii());
  fprintf(fResults, "<TITLE>%sKFileReplace Results File</TITLE></HEAD>\n", i18n("KFileReplace Results File").ascii());
  fprintf(fResults, "<BODY><H1>%s</H1>\n", i18n("KFileReplace Results File").ascii());
  fprintf(fResults, "<DL><p>\n");

  // c) Write the file list
  QListViewItem *lviCurItem;
  QListViewItem *lviFirst;
  QString strPath;

  lviCurItem = lviFirst = lvResult -> firstChild();
  if (lviCurItem == NULL)
    {
      return ;
    }

  do
    {
      strPath = formatFullPath(lviCurItem -> text(1), lviCurItem -> text(0));
      fprintf(fResults,"<DT><A HREF=\"file:%s\">file:%s</A><br>\n",        strPath.ascii(), strPath.ascii());
      fprintf(fResults,i18n("Size: %s --> %s **** %.3ld strings replaced.<br><br>\n"), lviCurItem -> text(2).ascii(),
              lviCurItem -> text(3).ascii(), lviCurItem -> text(2).toULong());

      lviCurItem = lviCurItem -> nextSibling();
    } while(lviCurItem && lviCurItem != lviFirst);


  // d) Write the end of the file
  fprintf(fResults,"</DL><P></BODY></HTML>\n");
  fclose(fResults);
  updateCommands();
}


// ===========================================================================================================================
void KFileReplacePart::slotStringsAdd()
{
  m_view -> slotStringsAdd();
  updateCommands();
  return;
}

// ===========================================================================================================================
void KFileReplacePart::slotStringsDel()
{
  delete m_view -> getStringView() -> currentItem(); // Remove item from ListView
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotStringsEmpty()
{
  m_view -> getStringView() -> clear();
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotStringsEdit()
{
  m_view -> slotStringsEdit(NULL);
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotStringsSave()
{
  QString strFilename;
  QListViewItem *lviCurItem;
  QListViewItem *lviFirst;
  int nItemPos;
  QWidget *w = widget();

  // Check there are strings in the list
  if (m_view -> getStringView() -> childCount() == 0)
    {
      KMessageBox::error(w, i18n("There are no strings to save in the list."));
      return ;
    }

  // Select the file where strings will be saved
  strFilename = KFileDialog::getSaveFileName(QString::null, i18n("*.kfr|KFileReplace strings (*.kfr)\n*|All Files (*)"), w, i18n("Save strings to file ..."));
  if (strFilename.isEmpty())
    return;

  // Force the extension to be "kfr" == KFileReplace extension
  strFilename = addFilenameExtension(strFilename, "kfr");

//TODO: Use QFile
  // a) Open the file
  FILE *fStrings;
  fStrings = fopen(strFilename.ascii(),"w");

  if (fStrings == NULL) // Can't open file
    {
      KMessageBox::error(w, i18n("<qt>Can't open the file <b>%1</b> for writing to save the string list.</qt>").arg(strFilename));
      return ;
    }

  // b) Write header of the KFR file
  KFRHeader head;
  memset(&head, 0, sizeof(KFRHeader));

  sprintf(head.szPgm, "KFileReplace");
  head.nStringsNb = m_view -> getStringView() -> childCount();

  if (fwrite(&head, sizeof(KFRHeader), 1, fStrings) != 1)
    {
      KMessageBox::error(w, i18n("<qt>Can't write data in file <b>%1</b>.</qt>").arg(strFilename));
      fclose(fStrings);
      return ;
    }

  // c) Write the file strings
  int nOldTextSize, nNewTextSize;
  int nErrors = 0;

  nItemPos = 0;
  lviCurItem = lviFirst = m_view -> getStringView() -> firstChild();
  if (lviCurItem == NULL)
    return ;

  do
    {
      nOldTextSize = strlen(lviCurItem -> text(0));
      nNewTextSize = strlen(lviCurItem -> text(1));
      nErrors += (fwrite(&nOldTextSize, sizeof(int), 1, fStrings)) != 1;
      nErrors += (fwrite(&nNewTextSize, sizeof(int), 1, fStrings)) != 1;
      nErrors += (fwrite(lviCurItem -> text(0), nOldTextSize, 1, fStrings)) != 1;
      if (nNewTextSize > 0)
        nErrors += (fwrite(lviCurItem -> text(1), nNewTextSize, 1, fStrings)) != 1;
      if (nErrors > 0)
        {
          KMessageBox::error(w, i18n("<qt>Can't write data in file <b>%1</b>.</qt>").arg(strFilename));
          fclose(fStrings);
          return ;
        }
      lviCurItem = lviCurItem -> nextSibling();
    } while(lviCurItem && lviCurItem != lviFirst);


  // d) Close the file
  fclose(fStrings);
}

// ===========================================================================================================================
void KFileReplacePart::loadStringFile(const QString& strFilename)
{
  KListViewString *lvi;
  QWidget *w = widget();

  if (strFilename.isEmpty())
    return;

//TODO: convert it to QFile and handle remote files!
  // a) Open the file
  FILE *fStrings;
  fStrings = fopen(strFilename.ascii(),"r");

  if (fStrings == NULL) // Can't open file
    {
      KMessageBox::error(w, i18n("<qt>Can't open the file <b>%1</b> and load the string list.</qt>").arg(strFilename));
      return ;
    }

  // b) Read header of the KFR file
  KFRHeader head;

  if (fread(&head, sizeof(KFRHeader), 1, fStrings) != 1)
    {
      KMessageBox::error(w, i18n("<qt>Can't read data from file <b>%1</b>.</qt>").arg(strFilename));
      fclose(fStrings);
      return ;
    }

  // Check the file is a KFileReplace file
  if (strcmp(head.szPgm, "KFileReplace") != 0)
    {
      KMessageBox::error(w, i18n("<qt><b>%1</b> is not a KFileReplace file. Can't continue.</qt>").arg(strFilename));
      fclose(fStrings);
      return ;
    }

  // Add file to "load strings form file" menu
  if (!m_recentStringFileList.contains(strFilename))
    {
      m_recentStringFileList.append(strFilename);
      ((KRecentFilesAction *) actionCollection()->action("strings_load_recent"))->setItems(m_recentStringFileList);
    }

  // c) Read the file strings
  int nOldTextSize, nNewTextSize;
  int nErrors = 0;
  char *szString; // Dynamic memory alloc
  int nStrSize;

  // Empty the current list
  m_view -> getStringView() -> clear();

  for (int i=0; i < head.nStringsNb; i++)
    {
      nErrors += (fread(&nOldTextSize, sizeof(int), 1, fStrings)) != 1;
      nErrors += (fread(&nNewTextSize, sizeof(int), 1, fStrings)) != 1;
      if (nErrors > 0)
        {
          KMessageBox::error(w, i18n("<qt>Can't read data from file <b>%1</b></qt>").arg(strFilename));
          fclose(fStrings);
          return ;
        }

      // Allocate new string
      nStrSize = ((nOldTextSize > nNewTextSize) ? nOldTextSize : nNewTextSize) + 2;
      szString = new char[nStrSize+10];
      memset(szString, 0, nStrSize);
      if (szString == 0)
        {
          KMessageBox::error(w, i18n("Out of memory."));
          fclose(fStrings);
          return ;
        }

      // Read "Search" text
      if (fread(szString, nOldTextSize, 1, fStrings) != 1)
        {
          KMessageBox::error(w, i18n("<qt>Can't read data from file <b>%1</b></qt>").arg(strFilename));
          fclose(fStrings);
          return ;
        }

      // Add item to list
      lvi = new KListViewString(m_view -> getStringView(), szString);
      Q_CHECK_PTR( lvi );
      lvi -> setPixmap(0, m_view -> getIconString());

      // Read "Replace" text
      memset(szString, 0, nStrSize);
      if (nNewTextSize > 0) // If there is a Replace text
        {
          if (fread(szString, nNewTextSize, 1, fStrings) != 1)
            {
              KMessageBox::error(w, i18n("<qt>Can't read data from file <b>%1</b></qt>").arg(strFilename));
              fclose(fStrings);
              return ;
            }
          lvi -> setText(1, szString);
        }

      delete szString;
    }

  // d) Close the file
  fclose(fStrings);
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotStringsLoad()
{
  QString strFilename;

  // Select the file to load from
  strFilename = KFileDialog::getOpenFileName(QString::null, i18n("*.kfr|KFileReplace strings (*.kfr)\n*|All Files (*)"), widget(), i18n("Load strings from file ..."));

  loadStringFile(strFilename);
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotStringsInvertCur()
{
  QListViewItem *lviCurItem;
  lviCurItem = m_view -> getStringView() -> currentItem();
  if (lviCurItem == 0)
    return;

  QString strSearch, strReplace;
  strSearch = lviCurItem -> text(0);
  strReplace = lviCurItem -> text(1);

  // Cannot invert the string if search string will be empty
  if (strReplace.isEmpty())
    {
      KMessageBox::error(widget(), i18n("<qt>Cannot invert string <b>%1</b>, because the search string would be empty.</qt>").arg(strSearch));
      return;
    }

  lviCurItem -> setText(0, strReplace);
  lviCurItem -> setText(1, strSearch);
}


// ===========================================================================================================================
void KFileReplacePart::slotStringsInvertAll()
{
  QListViewItem *lviCurItem;
  QListViewItem *lviFirst;
  int nItemPos;
  QString strSearch, strReplace;
  QString strMess;

  nItemPos = 0;
  lviCurItem = lviFirst = m_view -> getStringView() -> firstChild();
  if (lviCurItem == NULL)
    return ;

  do
    {
      strSearch = lviCurItem -> text(0);
      strReplace = lviCurItem -> text(1);

      // Cannot invert the string if search string will be empty
      if (strReplace.isEmpty())
        {
          KMessageBox::error(widget(), i18n("<qt>Cannot invert string <b>%1</b>, because the search string would be empty.</qt>").arg(strSearch));
          return;
        }

      lviCurItem -> setText(0, strReplace);
      lviCurItem -> setText(1, strSearch);

      lviCurItem = lviCurItem -> nextSibling();
    } while(lviCurItem && lviCurItem != lviFirst);
}

// ===========================================================================================================================
void KFileReplacePart::slotOpenRecentStringFile(const KURL& urlFile)
{
  QString strFilename;
  QFileInfo fi;

  if (g_bThreadRunning == true) // Thread running: it has not finished since the last call of this function
    return ;

  // Download file if need (if url is "http://...")
  if (!(KIO::NetAccess::download(urlFile, strFilename))) // Open the Archive
    return;

  // Check it's not a directory
  fi.setFile(strFilename);
  if (fi.isDir())
    {
      KMessageBox::error(widget(), i18n("Can't open directories."));
      return;
    }

  loadStringFile(strFilename);
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotOptionsRecursive()
{
  m_settings.bRecursive = !(m_settings.bRecursive);
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotOptionsBackup()
{
  m_settings.bBackup = !(m_settings.bBackup);
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotOptionsCaseSensitive()
{
  m_settings.bCaseSensitive = !(m_settings.bCaseSensitive);
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotOptionsVariables()
{
  m_settings.bVariables = !(m_settings.bVariables);
  updateCommands();
}

// ===========================================================================================================================
void KFileReplacePart::slotOptionsWildcards()
{
  m_settings.bWildcards = !(m_settings.bWildcards);
  updateCommands();
}


// ===========================================================================================================================
void KFileReplacePart::slotOptionsPreferences()
{
  KOptionsDlg dlg(widget(), 0, m_settings);

  if (dlg.exec() == QDialog::Rejected) // If Cancel
    return ;

  m_settings = dlg.getSettings();

  saveOptions();
  updateCommands();
}

void KFileReplacePart::setWhatsThis()
{
  actionCollection()->action("file_simulate")->setWhatsThis(i18n("The same operation as the replace one, but do not make any changes in files. "
                                                                 "This is not a simple search, because you will see the exact changes that could "
                                                                 "be done (with regexp or variables for example.)"));

  actionCollection()->action("options_wildcards")->setWhatsThis(i18n("Enable use of the wildcards (* for expressions, and ? for single character if not modified "
                                                                     "in the wildcards options). For example, you can search for <b>KMsgBox::message(*)</b> "
                                                                     "and replace with <b>KMessageBox::error(*)</b>"));
  actionCollection()->action("options_backup")->setWhatsThis(i18n("Create a copy of the original replaced files with the BAK extension before replacing"));
  actionCollection()->action("options_case")->setWhatsThis(i18n("The lowers and uppers are differents. For example, if you search for <b>Linux</b> and "
                                                                "there is <b>linux</b>, then the string won't be found/replaced."));
  actionCollection()->action("options_var")->setWhatsThis(i18n("Enable use of the variables, as the date & time or the name of the current file. "
                                                               "The variable must be in the replace string, with the format [$Name:  $]. When doing the replace, "
                                                               "the value of the variable "
                                                               "is written instead of the variable. If disabled, the replace string will be copied without "
                                                               "substitution. Please, read help for"
                                                               "more details on the variables names and formats."));
  actionCollection()->action("options_recursive")->setWhatsThis(i18n("Search/Replace files in the sub-directories of the main directory selected "
                                                                     "when creating a new project at startup."));
}



// ===========================================================================================================================
void KFileReplacePart::reportBug()
{
  KBugReport dlg(widget(), true, createAboutData());
  dlg.exec();
}

// ===========================================================================================================================
void KFileReplacePart::appHelpActivated()
{
  kapp->invokeHelp(QString::null, "kfilereplace");
}

void KFileReplacePart::showAboutApplication()
{
  if(m_dlgAbout == NULL)
    {
      m_dlgAbout = new KAboutKFileReplace(widget(), 0, false);
      if(m_dlgAbout == NULL) { return; }
    }

  if(m_dlgAbout->isVisible() == false)
    {
      m_dlgAbout->show();
    }
  else
    {
      m_dlgAbout->raise();
    }

  // Update menu & toolbar commands
  updateCommands();
}
/*
// ===========================================================================================================================
void KFileReplacePart::dragEnterEvent(QDragEnterEvent *event)
{
        event -> accept(QUriDrag::canDecode(event));
}

// ===========================================================================================================================
void KFileReplacePart::dropEvent(QDropEvent *event)
{
  QStrList urls;

  if (QUriDrag::decode(event, urls))
    {
      if (urls.first())
        {
          slotOpenRecentStringFile(urls.first());
        }
    }
  updateCommands();
}

*/
#include "kfilereplacepart.moc"
