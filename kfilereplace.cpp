/***************************************************************************
               kfilereplace.cpp  -  KFileReplace shell implementation
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

//kde includes
#include <kedittoolbar.h>
#include <klibloader.h>
#include <klocale.h>
#include <kkeydialog.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
//app includes
#include "kfilereplace.h"
//Added by qt3to4:
#include <Q3PtrList>
#include <kactioncollection.h>
#include <kstdaction.h>

KFileReplace::KFileReplace()
 : KParts::MainWindow(0L, "KFileReplace")
{
    KLibFactory *factory = KLibLoader::self()->factory("libkfilereplacepart");
    if (factory)
    {
        m_part = static_cast<KParts::ReadOnlyPart *>(factory->create(this,
                "kfilereplace_part" ));

        if (m_part)
        {
            setCentralWidget(m_part->widget());
            KStdAction::quit(this, SLOT(close()), actionCollection());
            KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
            KStdAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());
            setStandardToolBarMenuEnabled(true);
            createGUI(m_part);
            removeDuplicatedActions();
        }
    }
    else
    {
        KMessageBox::error(this, i18n("Could not find the KFileReplace part."));
        close();
        return;
    }
}


KFileReplace::~KFileReplace()
{
}

void KFileReplace::openURL(const KUrl &url)
{
  m_part->openURL(url);
}

void KFileReplace::slotConfigureKeys()
{
	KKeyDialog dlg( KKeyChooser::AllActions, KKeyChooser::LetterShortcutsAllowed, this );
    QList<KXMLGUIClient*> clients = guiFactory()->clients();
  	foreach(KXMLGUIClient *client, clients)
    	dlg.insert ( client->actionCollection());
	dlg.configure();
}

void KFileReplace::slotConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config(), autoSaveGroup());
    KEditToolbar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

void KFileReplace::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config(), autoSaveGroup());
}


void KFileReplace::removeDuplicatedActions()
{
    KActionCollection* part_action_collection = m_part->actionCollection();
    KAction* part_about_action = part_action_collection->action("help_about_kfilereplace");
    KAction* part_report_action = part_action_collection->action("report_bug");
    KAction* part_help_action= part_action_collection->action("help_kfilereplace");

    if (!part_about_action || !part_report_action || !part_help_action || !part_action_collection) 
      return;

    QWidget* container = part_about_action->container(0);
    part_about_action->unplug(container);
    part_report_action->unplug(container);
    part_help_action->unplug(container);
    part_action_collection->remove(part_about_action);
    part_action_collection->remove(part_report_action);
    part_action_collection->remove(part_help_action);
}

#include "kfilereplace.moc"
