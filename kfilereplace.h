/***************************************************************************
               kfilereplace.h  -  KFileReplace header
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
#ifndef KFILEREPLACE_H
#define KFILEREPLACE_H

#include <kparts/mainwindow.h>

/**
Shell application for the KFileReplace part.

@author Andras Mantia
*/
class KFileReplace : public KParts::MainWindow
{
  Q_OBJECT
public:
    KFileReplace();
    ~KFileReplace();
    void openURL(const KUrl &url);

private slots:
   void slotConfigureKeys();
   void slotConfigureToolbars();
   void applyNewToolbarConfig();

private:
   void removeDuplicatedActions();

   KParts::ReadOnlyPart *m_part;
};

#endif
