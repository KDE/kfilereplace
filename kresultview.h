/***************************************************************************
                          kresultview.h  -  description
                             -------------------
    begin                : sam oct 16 15:28:00 CEST 1999
    copyright            : (C) 1999 by François Dupoux
                                  (C) 2004 Emiliano Gulmini <emi_barbarossa@yahoo.it>
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

#ifndef KRESULTVIEW_H
#define KRESULTVIEW_H

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

// include files for Qt
#include <qlistview.h>

// include files for KDE
#include <kpopupmenu.h>

class KFileReplaceApp;

// ================================================================
class KResultView : public QListView
{
        Q_OBJECT

public:
        KResultView( QWidget *parent = 0, const char *name = 0);
        ~KResultView();
        QString currentItem();

public:
        QListViewItem* addFullItem(bool bSuccess, const QString& szName, const QString& szDirectory, uint nOldSize, uint nNewSize, int nNbRepl, const QString& szErrMsg=QString::null);
        int updateItem(QListViewItem *lvi, bool bSuccess, uint nNewSize, int nNbRepl, const QString& szErrMsg=QString::null);
        bool increaseStringCount(QListViewItem *lvi, QString strOld, QString strNew, QString strReplace, const char *szSearch, int nSearchLen, bool bShowDetails);
        QPixmap iconString();

public slots:
        void slotResultProperties();
        void slotResultOpen();
        void slotResultOpenWith();
        void slotResultEdit();
        void slotResultDirOpen();
        void slotResultDelete();
        void slotResultTreeExpand();
        void slotResultTreeReduce();

        void slotMouseButtonClicked (int nButton, QListViewItem *lvi, const QPoint &pos, int column);

 private:
        void expand(QListViewItem *lviCurrent, bool bExpand);


private:
        KFileReplaceApp *m_app;
        KPopupMenu *m_kpmResult;
        QListViewItem *m_lviCurrent;
        QPixmap m_pmIconSuccess;
        QPixmap m_pmIconError;
        QPixmap m_pmIconString;
        QPixmap m_pmIconSubString;

};

#endif // KRESULTVIEW_H














