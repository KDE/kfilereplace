//
// C++ Implementation: kfilereplace
//
// Description:
//
//
// Author:  Andras Mantia <amantia@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//

//kde includes
#include <kedittoolbar.h>
#include <klibloader.h>
#include <klocale.h>
#include <kkeydialog.h>
#include <kmessagebox.h>

//app includes
#include "kfilereplace.h"

KFileReplace::KFileReplace()
 : KParts::MainWindow(0L, "KFileReplace")
{
    KLibFactory *factory = KLibLoader::self()->factory("libkfilereplacepart");
    if (factory)
    {
        m_part = static_cast<KParts::ReadOnlyPart *>(factory->create(this,
                "kfilereplace_part", "KParts::ReadOnlyPart" ));

        if (m_part)
        {
            setCentralWidget(m_part->widget());
            KStdAction::quit(this, SLOT(close()), actionCollection());
            KStdAction::keyBindings(this, SLOT(slotConfigureKeys()), actionCollection());
            KStdAction::configureToolbars(this, SLOT(slotConfigureToolbars()), actionCollection());
            createGUI(m_part);
            removeDuplicatedActions();
        }
    }
    else
    {
        KMessageBox::error(this, i18n("Could not find the KFileReplace part!"));
        close();
        return;
    }
}


KFileReplace::~KFileReplace()
{
}

void KFileReplace::openURL(const KURL &url)
{
  m_part->openURL(url);
}

void KFileReplace::slotConfigureKeys()
{
    KKeyDialog dlg( false, this );
    QPtrList<KXMLGUIClient> clients = guiFactory()->clients();
    for( QPtrListIterator<KXMLGUIClient> it( clients );
            it.current(); ++it )
    {
        dlg.insert( (*it)->actionCollection() );
    }
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

    QWidget* container = part_about_action->container(0);
    part_about_action->unplug(container);
    part_report_action->unplug(container);
    part_help_action->unplug(container);
    part_action_collection->remove(part_about_action);
    part_action_collection->remove(part_report_action);
    part_action_collection->remove(part_help_action);
}

#include "kfilereplace.moc"
