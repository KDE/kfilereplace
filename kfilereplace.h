//
// C++ Interface: kfilereplace
//
// Description:
//
//
// Author: Andras Mantia <amantia@kde.org>, (C) 2004
//
// Copyright: See COPYING file that comes with this distribution
//
//
#ifndef KFILEREPLACE_H
#define KFILEREPLACE_H

#include <kparts/mainwindow.h>

/**
Shell application for the KFileReplace part.

@author
*/
class KFileReplace : public KParts::MainWindow
{
  Q_OBJECT
public:
    KFileReplace();
    ~KFileReplace();
    void openURL(const KURL &url);

private slots:
   void slotConfigureKeys();
   void slotConfigureToolbars();
   void applyNewToolbarConfig();

private:
   void removeDuplicatedActions();

   KParts::ReadOnlyPart *m_part;
};

#endif
