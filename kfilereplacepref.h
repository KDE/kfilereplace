#ifndef KFILEREPLACEPREF_H
#define KFILEREPLACEPREF_H

#include <kdialogbase.h>
#include <qframe.h>

class KFileReplacePrefPageOne;
class KFileReplacePrefPageTwo;

class KFileReplacePreferences : public KDialogBase
{
    Q_OBJECT
public:
    KFileReplacePreferences();

private:
    KFileReplacePrefPageOne *m_pageOne;
    KFileReplacePrefPageTwo *m_pageTwo;
};

class KFileReplacePrefPageOne : public QFrame
{
    Q_OBJECT
public:
    KFileReplacePrefPageOne(QWidget *parent = 0);
};

class KFileReplacePrefPageTwo : public QFrame
{
    Q_OBJECT
public:
    KFileReplacePrefPageTwo(QWidget *parent = 0);
};

#endif // KFILEREPLACEPREF_H
