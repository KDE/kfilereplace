/***************************************************************************
                          kfilereplaceif.h  -  description
                             -------------------
    copyright            : (C) 1999 by François Dupoux
                           (C) 2003 Andras Mantia
    email                : dupoux@dupoux.com, amantia@kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/
#ifndef KFILEREPLACEIFACE_H
#define KFILEREPLACEIFACE_H

#include <dcopobject.h>

class KFileReplaceIface : virtual public DCOPObject
{
  K_DCOP

k_dcop:
  virtual void openURL(const QString& url) = 0;
};

#endif // KFILEREPLACEIFACE_H
