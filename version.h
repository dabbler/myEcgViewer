#ifndef __VERSION_H
#define __VERSION_H

#include <QtGui>

#define VERSION_MAJOR	2
#define VERSION_MINOR	1
#define SVN_REVISION	131

QString glb_version = QString("$Rev:  %1.%2.%3").arg(VERSION_MAJOR).arg(VERSION_MINOR).arg( QString::number(SVN_REVISION) );

#endif // __VERSION_H
