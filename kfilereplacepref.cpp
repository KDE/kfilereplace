#include "kfilereplacepref.h"

#include <klocale.h>

#include <qlayout.h>
#include <qlabel.h>

KFileReplacePreferences::KFileReplacePreferences()
    : KDialogBase(TreeList, "KFileReplace Preferences",
                  Help|Default|Ok|Apply|Cancel, Ok)
{
    // this is the base class for your preferences dialog.  it is now
    // a Treelist dialog.. but there are a number of other
    // possibilities (including Tab, Swallow, and just Plain)
    QFrame *frame;
    frame = addPage(i18n("First Page"), i18n("Page One Options"));
    m_pageOne = new KFileReplacePrefPageOne(frame);

    frame = addPage(i18n("Second Page"), i18n("Page Two Options"));
    m_pageTwo = new KFileReplacePrefPageTwo(frame);
}

KFileReplacePrefPageOne::KFileReplacePrefPageOne(QWidget *parent)
    : QFrame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAutoAdd(true);

    new QLabel("Add something here", this);
}

KFileReplacePrefPageTwo::KFileReplacePrefPageTwo(QWidget *parent)
    : QFrame(parent)
{
    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->setAutoAdd(true);

    new QLabel("Add something here", this);
}

//#include "kfilereplacepref.moc"

#include "kfilereplacepref.moc"

