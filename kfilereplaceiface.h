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
