/***************************************************************************
                           commandengine.cpp  -  kfr commands feature class
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

// QT
#include <qdatetime.h>
#include <qfile.h>
#include <qtextstream.h>
#include <qdom.h>

// KDE
#include <kuser.h>
#include <krandomsequence.h>
#include <kprocess.h>

// local
#include "commandengine.h"

QString CommandEngine::datetime(const QString& opt, const QString& arg)
{
  Q_UNUSED(arg);
  if(opt == "iso")
    return QDateTime::currentDateTime(Qt::LocalTime).toString(Qt::ISODate);
  if(opt == "local")
    return QDateTime::currentDateTime(Qt::LocalTime).toString(Qt::LocalDate);
  return QString::null;
}

QString CommandEngine::user(const QString& opt, const QString& arg)
{
  Q_UNUSED(arg);
  KUser u;
  if(opt == "uid")
    return QString::number(u.uid(),10);
  if(opt == "gid")
    return QString::number(u.gid(),10);
  if(opt == "loginname")
    return u.loginName();
  if(opt == "fullname")
    return u.fullName();
  if(opt == "homedir")
    return u.homeDir();
  if(opt == "shell")
    return u.shell();
  return QString::null;
}

QString CommandEngine::loadfile(const QString& opt, const QString& arg)
{
  Q_UNUSED(arg);

  QFile f(opt);
  if(!f.open(IO_ReadOnly)) return QString::null;

  QTextStream t(&f);

  QString s = t.read();

  f.close();

  return s;
}

QString CommandEngine::empty(const QString& opt, const QString& arg)
{
  Q_UNUSED(opt);
  Q_UNUSED(arg);
  return "";
}

QString CommandEngine::mathexp(const QString& opt, const QString& arg)
{
  /* We will use bc 1.06 by Philip A. Nelson <philnelson@acm.org> */
  //Q_UNUSED(opt);
  Q_UNUSED(arg);

  QString tempOpt = opt;
  tempOpt.replace("ln","l");
  tempOpt.replace("sin","s");
  tempOpt.replace("cos","c");
  tempOpt.replace("arctan","a");
  tempOpt.replace("exp","e");
  
  QString program = "var=("+tempOpt+");print var";
  QString script = "echo '"+program+"' | bc -l;";

  KProcess* proc = new KProcess();

  proc->setUseShell(true);

  *(proc) << script;

   connect(proc, SIGNAL(receivedStdout(KProcess*,char*,int)), this, SLOT(slotGetScriptOutput(KProcess*,char*,int)));
   connect(proc, SIGNAL(receivedStderr(KProcess*,char*,int)), this, SLOT(slotGetScriptError(KProcess*,char*,int)));
   connect(proc, SIGNAL(processExited(KProcess*)), this, SLOT(slotProcessExited(KProcess*)));

  //Through slotGetScriptOutput, m_processOutput contains the result of the KProcess call
   if(not proc->start(KProcess::Block, KProcess::All))
     {
       //qWarning("EEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEEE");
     }
   else
     {
       proc->wait();
     }
   if(proc)
     delete proc;

   QString tempbuf = m_processOutput;
   m_processOutput = QString::null;

   return tempbuf;

}

QString CommandEngine::random(const QString& opt, const QString& arg)
{
  Q_UNUSED(arg);
  long seed;
  if(opt.isEmpty())
    {
      QDateTime dt;
      seed = dt.toTime_t();
    }
  else
    seed = opt.toLong();

  KRandomSequence seq(seed);
  return QString::number(seq.getLong(1000000),10);
}

QString CommandEngine::stringmanip(const QString& opt, const QString& arg)
{
  Q_UNUSED(opt);
  Q_UNUSED(arg);
  return "";
}

QString CommandEngine::variableValue(const QString &variable)
{
  QString s = variable;

  s.remove("[$").remove("$]").remove(" ");

  if(s.contains(":") == 0)
    return variable;
  else
    {
      QString leftValue = s.section(":",0,0),
              midValue = s.section(":",1,1),
              rightValue = s.section(":",2,2);

      QString opt = midValue;
      QString arg = rightValue;

      if(leftValue == "stringmanip")
        return stringmanip(opt, arg);
      if(leftValue == "datetime")
        return datetime(opt, arg);
      if(leftValue == "user")
        return user(opt, arg);
      if(leftValue == "loadfile")
        return loadfile(opt, arg);
      if(leftValue == "empty")
        return empty(opt, arg);
      if(leftValue == "mathexp")
        return mathexp(opt, arg);
      if(leftValue == "random")
        return random(opt, arg);

      return variable;
    }
}

//SLOTS
void CommandEngine::slotGetScriptError(KProcess* proc, char* s, int i)
{
  Q_UNUSED(proc);
  Q_UNUSED(proc);
  QCString temp(s,i+1);
  if(temp.isEmpty() or temp == "\n") return;
 // KMessageBox::error(0,QString::fromLocal8Bit(temp)+"\n","ERROR");
}

void CommandEngine::slotGetScriptOutput(KProcess* proc, char* s, int i)
{
  Q_UNUSED(proc);
  QCString temp(s,i+1);

  if(temp.isEmpty() or temp == "\n") return;

  m_processOutput += QString::fromLocal8Bit(temp);

  //qWarning("m_resultBuffer [%s]",m_processOutput.latin1());
}

void CommandEngine::slotProcessExited(KProcess* proc)
{
  Q_UNUSED(proc);
}

#include "commandengine.moc"
