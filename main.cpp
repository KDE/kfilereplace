/**************************************************************************
                          main.cpp  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by François Dupoux
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

static const char cvsId[]="$Id$";

#include "kfilereplace.h"
#include "resource.h"
#include "apistruct.h"

#include <kaboutdata.h>
#include <kcmdlineargs.h>

#include <stdio.h>

KApplication *g_app;

// ===========================================================================================================================
static KCmdLineOptions options[] =
{
    // Main commands
    {"r", 0, 0},
    {"replace", I18N_NOOP("Run the replace operation"), 0},
    {"s", 0, 0},
    {"search", I18N_NOOP("Run the search operation"), 0},

    // Some options
    {"x", 0, 0},
    {"exit", I18N_NOOP("Exit application after having done the command line operation"), 0},
    {"l", 0, 0},
    {"loadstr <strfile>", I18N_NOOP("Load file (*.kfr) which contains strings to search/replace (need if doing replace from cmd line)"), 0},
    {"example", I18N_NOOP("Show examples of use of the KFileReplace command line"), 0},

    // Working options
    {"e", 0, 0},
    {"norecursive", I18N_NOOP("Disable recursivity (on by default): Do not Search/Replace in all sub-directories"), 0},
    {"b", 0, 0},
    {"nobackup", I18N_NOOP("Disable backup creation (on by default): Do not create a backup of files before the replace operation"), 0},
    {"c", 0, 0},
    {"case", I18N_NOOP("Enable case sensitive (off by default): Lowers and uppers are differents when searching"), 0},
    {"j", 0, 0},
    {"wildcards", I18N_NOOP("Enable use of wildcards: (off by default): * and ? can be used in strings to search/replace"), 0},
    {"v", 0, 0},
    {"variables", I18N_NOOP("Enable use of variables (off by default): In the replace string, works with variables"), 0},
    {"y", 0, 0},
    {"symlinks", I18N_NOOP("Enable symbolic links folowwing: (off by default)"), 0},
    {"a", 0, 0},
    {"allstrings", I18N_NOOP("All strings must be found: (off by default)"), 0},

    // Project properties
    {"+[Directory]", I18N_NOOP("Directory where operation has to be done (for example: /home/me/website/)"), 0},
    {"+[Files]", I18N_NOOP("Type of files to search or replace (for example: *.htm?;*.xml)"), 0},
    {0, 0, 0}
};

// ===========================================================================================================================
int main(int argc, char* argv[])
{
  QString strArguDir, strArguFilter;
  
  KAboutData about("kfilereplace", "KFileReplace", VERSION, I18N_NOOP("Replace strings inside files"), 
		   KAboutData::License_GPL, "Francois Dupoux", 0, "http://kfilereplace.sourceforge.net/", "dupoux@dupoux.com");
  
  KCmdLineArgs::init(argc, argv, &about);
  KCmdLineArgs::addCmdLineOptions(options);
  
  KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
  
  // If we must show an example of use of commmand line
  if (args->isSet("example"))
    {
      printf (i18n("Examples of use of the KFileReplace command line\n"
		   "* kfilereplace --replace --exit --loadstr /home/me/strings.kfr /home/me/webpages *.htm?;*.xml\n"
		   "  In this example, it load the strings from the strings.kfr file, it run the replace\n"
		   "  in all *.htm?;*.xml files of the /home/me/webpages directory, and it exists.\n"
		   "* kfilereplace -l /home/me/strings.kfr /home/me/webpages *.html\n"
		   "  In this example, it opens a new project in the webpages directory, with all *.hmtl files\n"
		   "  and it loads strings from the strings.kfr file.\n"
		   "* kfilereplace --search /home/me/webpages *.html\n"
		   "  In this example, it search all *.html files of the webpages directory.\n\n").data());
      return 0;
    }
  
  KApplication app;
  g_app = &app;
  
  if (app.isRestored())
    {
      RESTORE(KFileReplaceApp);
    }
  else
    {
      if (args->count()) // If there are paramaters
	{
	  printf("MAIN.CPP: args->count = %ld\n", (long) args->count());
	  if ((args->count() >= 1) && (args->arg(0)))
	    strArguDir.sprintf(args->arg(0));
	  if ((args->count() >= 2) && (args->arg(1)))
	    strArguFilter.sprintf(args->arg(1));
	  
	  printf ("args = (%s) and (%s) and (%s)\n", strArguDir.data(), strArguFilter.data(), args->arg(2));		
	  
	}
      
      KFileReplaceApp* kfilereplace = new KFileReplaceApp(strArguDir, strArguFilter);
      
      // Read working options
      KSettings settings;
      settings = kfilereplace->getSettings();
      printf("==> main(): Set settings\n");
      if (!args->isSet("recursive")) settings.bRecursive = args->isSet("recursive");
      if (!args->isSet("backup")) settings.bBackup = args->isSet("backup");
      if (args->isSet("case")) settings.bCaseSensitive = args->isSet("case");
      if (args->isSet("wildcards")) settings.bWildcards = args->isSet("wildcards");
      if (args->isSet("variables")) settings.bVariables = args->isSet("variables");
      if (args->isSet("symlinks")) settings.bFollowSymLinks = args->isSet("symlinks");
      if (args->isSet("allstrings")) settings.bAllStringsMustBeFound = args->isSet("allstrings");
      kfilereplace->setSettings(settings);
      kfilereplace->updateCommands(); // update GUI
      
      // Arguments
      if (args->getOption("loadstr"))
	{
	  kfilereplace->loadStringFile(args->getOption("loadstr"));
	}	
      
      if (args->isSet("replace") && args->isSet("search"))
	{	
	  fprintf(stderr,i18n("error: search and replace options cannot be both enabled\n"));	
	  return -1;	
	}
      
      if (args->isSet("replace"))
	{
	  kfilereplace->slotFileReplace();	
	}
      else if (args->isSet("search"))
	{
	  kfilereplace->slotFileSearch();	
	}
      
      if (args->isSet("exit"))
	{
	  return 0;
	}
      
      args->clear(); // free memory
      kfilereplace->show();
    }
  
  return app.exec();
}  
 
















