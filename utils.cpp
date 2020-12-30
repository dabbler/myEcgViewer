/**
 * @file utils.cpp
*/ 

#include <QtGui>

#include <math.h>

#include "myheader.h"
#include "utils.h"

/* {{{ void printf_aligned(QPainter& dc, int aligned, int x, int y, int fontsize_pts, QString fmt )
*/
void printf_aligned( QPainter& dc, int aligned, int x, int y, int fontsize_pts, QString fmt, ... )
{
    Q_UNUSED(dc);
    Q_UNUSED(aligned);
    Q_UNUSED(x);
    Q_UNUSED(y);
    Q_UNUSED(fontsize_pts);

    va_list args;
	QString str;

	va_start(args, fmt);

    va_end(args);

}
/* }}} */


/* {{{ long convert_hms_to_secs( QString userstr )
 */
long convert_hms_to_secs( QString userstr )
{
#ifdef ANTIQUE
	long offsetsecs = atof((char*)userstr.BeforeFirst(':').c_str());
	userstr = userstr.AfterFirst(':');

	if ( userstr.BeforeFirst(':').Len() > 0 ) {
		offsetsecs *= 60;
		offsetsecs += atof((char*)userstr.BeforeFirst(':').c_str());
		userstr = userstr.AfterFirst(':');
	}

	if ( userstr.BeforeFirst(':').Len() > 0 ) {
		offsetsecs *= 60;
		offsetsecs += atof((char*)userstr.BeforeFirst(':').c_str());
		userstr = userstr.AfterFirst(':');
	}
#else
    Q_UNUSED(userstr);
    long offsetsecs = 0;
#endif

	return offsetsecs;
}
/* }}} */


