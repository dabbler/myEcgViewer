/**
 * @file utils.cpp
 *
 * Copyright (C) 2018 Datrix
 *
 * This program is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see https://www.gnu.org/licenses/.
 *
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


