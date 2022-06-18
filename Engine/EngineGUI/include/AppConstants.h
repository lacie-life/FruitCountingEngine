#ifndef APPCONSTANTS_H
#define APPCONSTANTS_H

#include <QObject>
#include <QDebug>

#ifndef MACRO_DEFINE
#define MACRO_DEFINE

#define CONSOLE qDebug() << "[" << __FUNCTION__ << "] "

#endif

#endif // APPCONSTANTS_H
