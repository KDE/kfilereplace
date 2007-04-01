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
#include <kshortcutsdialog.h>
#include <kmessagebox.h>
#include <kxmlguifactory.h>
//app includes
#include "kfilereplace.h"
//Added by qt3to4:
#include <kactioncollection.h>
#include <kstandardaction.h>

KFileReplace::KFileReplace()
  : KParts::MainWindow(0L, KDE_DEFAULT_WINDOWFLAGS)
{
    setObjectName("KFileReplace");
    KLibFactory *factory = KLibLoader::self()->factory("libkfilereplacepart");
    if (factory)
    {
        m_part = static_cast<KParts::ReadOnlyPart *>(factory->create(this, "KParts::ReadOnlyPart"));

        if (m_part)
        {
            m_part->setObjectName("kfilereplace_part");
            setCentralWidget(m_part->widget());
            KStandardAction::quit(this, SLOT(close()), actionCollection());
            KStandardAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
            KStandardAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());
            setStandardToolBarMenuEnabled(true);
            createGUI(m_part);
            removeDuplicatedActions();
        } else
        {
          KMessageBox::error(this, i18n("Could not load the KFileReplace part."));
          close();
          return;
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
  m_part->openUrl(url);
}

void KFileReplace::slotConfigureKeys()
{
	KShortcutsDialog dlg( KShortcutsEditor::AllActions, KShortcutsEditor::LetterShortcutsAllowed, this );
    QList<KXMLGUIClient*> clients = guiFactory()->clients();
  	foreach(KXMLGUIClient *client, clients)
    	dlg.addCollection(client->actionCollection());
	dlg.configure();
}

void KFileReplace::slotConfigureToolbars()
{
    saveMainWindowSettings(KGlobal::config()->group( autoSaveGroup()) );
    KEditToolbar dlg(factory());
    connect(&dlg, SIGNAL(newToolbarConfig()),
            this, SLOT(applyNewToolbarConfig()));
    dlg.exec();
}

void KFileReplace::applyNewToolbarConfig()
{
    applyMainWindowSettings(KGlobal::config()->group( autoSaveGroup()) );
}


void KFileReplace::removeDuplicatedActions()
{
    KActionCollection* part_action_collection = m_part->actionCollection();
    QAction* part_about_action = part_action_collection->action("help_about_kfilereplace");
    QAction* part_report_action = part_action_collection->action("report_bug");
    QAction* part_help_action= part_action_collection->action("help_kfilereplace");

    if (!part_about_action || !part_report_action || !part_help_action || !part_action_collection)
      return;

    QWidget* container = part_about_action->associatedWidgets().value(0);
    container->removeAction(part_about_action);
    container->removeAction(part_report_action);
    container->removeAction(part_help_action);
    part_action_collection->removeAction(part_about_action);
    part_action_collection->removeAction(part_report_action);
    part_action_collection->removeAction(part_help_action);
}

#include "kfilereplace.moc"
