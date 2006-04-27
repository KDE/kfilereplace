/***************************************************************************
               main.cpp  -  Main file for the KFileReplace shell
                             -------------------
    begin                : Thu Sep 16 14:14:09 2004
    copyright            : (C) 2004 by Andras Mantia <amantia@kde.org>
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include "kfilereplace.h"

#include <kapplication.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>

static const char description[] =
    I18N_NOOP("Batch search and replace tool");

static const char version[] = "0.1";

static KCmdLineOptions options[] =
    {
        { "+[PATH]", I18N_NOOP( "Starting folder" ), 0 },
        KCmdLineLastOption
    };

int main(int argc, char *argv[])
{
    KAboutData about("kfilereplace", I18N_NOOP("KFileReplace"), version, description,
                     KAboutData::License_GPL_V2, "(C) 2004-2005 Andras Mantia\n(C) 2004-2005 Emiliano Gulmini\n(C) 1999-2002 Francois Dupoux", I18N_NOOP("Part of the KDEWebDev module."),
                     "http://www.kdewebdev.org");

    about.addAuthor("Andras Mantia", I18N_NOOP("Shell author, KPart creator, co-maintainer"), "amantia@kde.org");
    about.addAuthor("Emiliano Gulmini", I18N_NOOP("Current maintainer, code cleaner and rewriter"),"emi_barbarossa@yahoo.it");
    about.addAuthor("Francois Dupou",
                          I18N_NOOP("Original author of the KFileReplace tool"),
                          "dupoux@dupoux.com");

    KCmdLineArgs::init(argc, argv, &about);
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;

    // see if we are starting with session management
    if (app.isRestored())
    {
        RESTORE(KFileReplace);
    }
    else
    {
        // no session.. just start up normally
        KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

        KFileReplace *kfr = new KFileReplace;
        kfr->show();

        if (args->count() == 0)
        {
            kfr->openURL(KUrl());
        }
        else
        {
            kfr->openURL(args->url(0));
        }
        args->clear();
    }

    return app.exec();
}
