/***************************************************************************
                           commandengine.h  -  kfr commands feature class
                                      -------------------
    begin                : fri aug  13 15:29:46 CEST 2004

    copyright            : (C) 2004 Emiliano Gulmini
    email                : emi_barbarossa@yahoo.it
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef COMMANDENGINE_H
#define COMMANDENGINE_H

// QT
class QString;
#include <qobject.h>

//KDE
class K3Process;

class CommandEngine : public QObject
{
  Q_OBJECT
  private:
    QString m_processOutput;
  public:
    CommandEngine() {}

  public:

    /** 
    These functions implement the KFR commands 
    */
    QString datetime(const QString& opt, const QString& arg);
    QString user(const QString& opt, const QString& arg);
    QString loadfile(const QString& opt, const QString& arg);
    QString empty(const QString& opt, const QString& arg);
    QString mathexp(const QString& opt, const QString& arg);
    QString random(const QString& opt, const QString& arg);
    QString stringmanip(const QString& opt, const QString& arg);
    QString variableValue(const QString &variable);

  private slots:
    void slotGetScriptOutput(K3Process*,char*,int);
    void slotGetScriptError(K3Process*,char*,int);
    void slotProcessExited(K3Process*);
};

#endif // COMMANDENGINE_H
