/***************************************************************************
                          KAddStringDlg.h  -  description
                             -------------------
    begin                : Sat Oct 16 1999
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

#ifndef KADDSTRINGDLG_H
#define KADDSTRINGDLG_H

#include <qwidget.h>
#include <qmultilineedit.h>

#include <qdialog.h>
#include <klocale.h>
class KPushButton;
class QLabel;
class QMultiLineEdit;

// ===========================================================================================================================
class KAddStringDlg : public QDialog
{
   Q_OBJECT
public: KAddStringDlg(QWidget *parent=0, const char *name=0);
	~KAddStringDlg();
	QString m_strSearch;
	QString m_strReplace;

protected:
	void initDialog();

	KPushButton *m_btnOk;
	KPushButton *m_btnCancel;
	QLabel *m_labelSearch;
	QMultiLineEdit *m_editSearch;
	QLabel *m_labelReplace;
	QMultiLineEdit *m_editReplace;

protected slots:
	void slotOk();

public:
	void setSearchText(const QString &strText) {m_editSearch -> setText(strText);}
	void setReplaceText(const QString &strText) {m_editReplace -> setText(strText);}

};

#endif //KADDSTRINGDLG_H

