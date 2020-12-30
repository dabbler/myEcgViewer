/*
 *
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
#ifndef BEATINFO_H
#define BEATINFO_H

#include "wfdb/wfdb.h"
#include "wfdb/ecgmap.h"
#include "wfdb/ecgcodes.h"

/** {{{ class BeatInfo
	@brief Declare a BeatInfo class
*/
class BeatInfo
{
public:
	BeatInfo()
	{
		pos_samps = 0;
		type = 0;
                subtype = 0;
	}

	BeatInfo( int32_t p, int32_t t )
	{
		pos_samps = p;
		type = t;
                subtype = 0;
	};

        void putSubtype(char sub) {
            subtype = sub;
        }

	void putAnnotationString( char *annoAux )
	{
		if ( annoAux ) {
			char str[64];
			int byteLen = annoAux[0];

			strncpy( str, &(annoAux[1]), byteLen );
			str[ byteLen ] = 0;

			annotationString = QString( str );
		}
	}

	BeatInfo &operator =( const BeatInfo &other )
	{
		// QQQ("beatinfo.log") << "assigning beat at pos " << pos_samps << " to " << other.pos_samps;
                annotationString = other.annotationString;
		pos_samps = other.pos_samps;
		type = other.type;
		return *this;
	};

	bool operator == ( const BeatInfo &other ) const
	{
		// QQQ("beatinfo.log") << "comparing beat at pos " << pos_samps << " to " << other.pos_samps;
		return ( pos_samps == other.pos_samps ) && ( type == other.type );
	};

	bool operator < ( const BeatInfo &other ) const
	{
		// QQQ("beatinfo.log") << "comparing beat at pos " << pos_samps << " to " << other.pos_samps;
		return ( pos_samps < other.pos_samps );
	};

public:
	QString annotationString;
	int32_t pos_samps;
	int	type;
        char subtype;
};
/* }}} */

#endif	// BEATINFO_H
