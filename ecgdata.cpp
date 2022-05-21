/**
 * @file ecgdata.cpp
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
 *
 * @note	http://www.physionet.org/physiotools/wag/signal-5.htm describes each of the possible formats
 *
*/

#include <QFile>
#include <QFileInfo>
#include <QDataStream>
#include <QDebug>
#include <QString>
#include <QStringList>
#include <QProgressDialog>
#include <math.h>

#include "myheader.h"
#include "ecgdata.h"
#include "showsignal.h"

// #define STORE_INTO_CHDATA(ch,i,val) { chdata[ch].append(val); }
#define STORE_INTO_CHDATA(ch,i,val) { quint16 thisVal = val; /* fileEcgCache[ch].seek(i); */ fileEcgCache[ch].write( (const char*) &thisVal, sizeof(quint16) ); }
			

    /* show progress & cancel saving in mid-stream if requested */
#define SHOW_PROGRESS_AND_WATCHFOR_CANCEL(idx) \
                if ( ((idx/4*4) % 1000) == 0 ) { /* qDebug() << idx/4*4; */ emit data_loaded_so_far(idx); QCoreApplication::processEvents(); } \
                if ( ! data_loading ) { break; } \


/* TODO add more comments to this file */




/** {{{ EcgData::EcgData()
  @brief Define a constructor for holding the ECG data
 */
EcgData::EcgData( QWidget *parent )
{
    Q_UNUSED(parent);
    device_range_mV = 10;
    range_per_sample = 50000;
    channel_count = 3;
    datalen_secs = 0;
    samps_per_chan_per_sec = 16000/80;
    signal_format_specifier = 311;
    bytes_per_samp = 4;
    viewableDateTime = QDateTime( QDate::currentDate(), QTime(0,0,0) );
}
/* }}} */


/** {{{ EcgData::EcgData( QString filename )
  @Brief
 */
EcgData::EcgData( QString filename, QWidget *parent )
{
    file_name = filename;
    device_range_mV = 10;
    range_per_sample = 50000;
    channel_count = 3;
    datalen_secs = 0;
    samps_per_chan_per_sec = 16000/80;
    signal_format_specifier = 311;
    bytes_per_samp = 4;
    data_loading = true;
    viewableDateTime = QDateTime( QDate::currentDate(), QTime(0,0,0) );

    QProgressDialog progress("Loading data...", "Cancel Data load", 0, 24*60*60*256*3 );
    progress.setWindowModality(Qt::ApplicationModal);
    QObject::connect( &progress, SIGNAL(canceled()), this, SLOT(cancel_data_loading()));
    QObject::connect( this, SIGNAL(load_size(int)), &progress, SLOT(setMaximum(int)));
    QObject::connect( this, SIGNAL(data_loaded_so_far(int)), &progress, SLOT(setValue(int)));
    QObject::connect( this, SIGNAL(loading_finished()), &progress, SLOT(cancel()));

	QObject::connect( this, SIGNAL(pacer_spike_found(long)), parent, SLOT(store_pacer_position(long)) );

	progress.show();

	QCoreApplication::processEvents();

    Load( parse_header(filename) );

	/* load annotation file */
    ShowSignal *ss = qobject_cast<ShowSignal *>( parent );
	QString recordName(filename);
	recordName.mid( 0, recordName.lastIndexOf(".") );
    ss->load_annotation_file( recordName.toLatin1().data(), (char*)"atr" );
}
/* }}} */


/** {{{ void EcgData::cancel_data_loading()
    @brief Cancel the loading of the ecg data
*/
void EcgData::cancel_data_loading()
{
    data_loading = false;
    emit loading_finished();
}
/* }}} */


/** {{{ EcgData::~EcgData()
  @brief Define a destructor
 */
EcgData::~EcgData()
{
}
/* }}} */


/** {{{ QString EcgData::parse_header( QString filename )
 * @filename: the name of the ECG data file
 *
 * Look for a similarly named .hea file that is associated with this ECG data
 * file to get the format of the data.
 *
 *	http://www.physionet.org/physiotools/wag/signal-5.htm describes each of the possible formats of ECG data
 *	Additionally, the following formats are recognized:
         format 1608:	16 bits per sample containing an  8 bit unsigned range of values LittleEndian
         format 1680:	16 bits per sample containing an  8 bit unsigned range of values    BigEndian
         format 1600:	16 bits per sample containing an 10 bit unsigned range of values LittleEndian
         format 1610:	16 bits per sample containing an 10 bit unsigned range of values LittleEndian
         format 1611:	16 bits per sample containing an 11 bit unsigned range of values LittleEndian
         format 1612:	16 bits per sample containing an 12 bit unsigned range of values LittleEndian
 *
 */
QString EcgData::parse_header( QString filename )
{
    QString unadulterated_ecgdata_filename = filename;
    int position = filename.lastIndexOf(".");
    QString ecgdata_extension = filename.mid(position+1);
	QString ecg_wfdb_record_name = filename.mid( 0, position );
    QString ecgheader_filename = ecg_wfdb_record_name + ".hea"; /* filename of associated header file */

	wfdbOpen( ecg_wfdb_record_name );

	if ( wfdbSignalInfo ) {
		samps_per_chan_per_sec = getifreq();
		signal_format_specifier = wfdbSignalInfo->fmt;
		if ( wfdbSignalInfo->gain == 0 ) {
			wfdbSignalInfo->gain = 200;
		}

		return unadulterated_ecgdata_filename;
	}

	/* ... else wfdb could not open it properly */


	qDebug() << qPrintable(tr("parse_header(%1)     ecgheader_filename = '%2'").arg(unadulterated_ecgdata_filename).arg(ecgheader_filename));

    /** set the default myCam parameters in case no HEA file is found */
    channel_count = 1;
    samps_per_chan_per_sec = 16000/80;

    if ( ! QFile::exists(ecgheader_filename) && QFile::exists(ECG_HEADER_UNIVERSAL) ) {
        ecgheader_filename = ECG_HEADER_UNIVERSAL;
    }

    QStringList tokenarray;

    QFile input( ecgheader_filename );

    if ( ! input.open(QIODevice::ReadOnly | QIODevice::Text) ) {
        qDebug() << "No ECG header file found.";
    } else {
        /* Use the data from the first 2 lines in the file to gather time and
         * date info, then also read:
         *	channel_count,
         *	samps_per_chan_per_sec, and
         *	signal_format_specifier
         */
        QTextStream text(&input);
        for ( int linecnt = 0 ; ! text.atEnd() ; linecnt++ ) {
            tokenarray.clear();
            QString line = text.readLine();
            line = line.simplified();
            tokenarray << line.split(' ');
            if ( linecnt == 0 ) {
                channel_count = tokenarray.value(1).toInt();
                samps_per_chan_per_sec = tokenarray.value(2).toInt();

                QString timestr = tokenarray.value(4);
                QString datestr = tokenarray.value(5);

                if ( ! timestr.contains(":") ) {
                    timestr = tokenarray.value(5);
                    datestr = tokenarray.value(6);
                }

                viewableDateTime = QDateTime::fromString( datestr + " " + timestr, "d/M/yyyy h:m:s" );

                qDebug() << " fromString(" << datestr + " " + timestr + ",'d/M/yyyy h:m:s') -> " << viewableDateTime;

                if ( ! viewableDateTime.isValid() ) {
                    viewableDateTime = QDateTime::fromString( timestr, "h:m:s" );
                    qDebug() << " FROMSTRING(" << timestr + ",'h:m:s') -> " << viewableDateTime;
                }
            }
            if ( linecnt == 1 ) {
                signal_format_specifier = tokenarray.value(1).toInt();
            }
        }
    }

    signal_format_specifier = 81;
    bytes_per_samp = 1;

    qDebug() << qPrintable(tr("parse_header(%1)     ecgheader_filename = '%2'").arg(filename).arg(ecgheader_filename));

    return unadulterated_ecgdata_filename;
}
/* }}} */


/** {{{ WFDB_Siginfo * EcgData::wfdbOpen( QString recordName )
 */
WFDB_Siginfo * EcgData::wfdbOpen( QString recordName )
{
	wfdbSignalInfo = NULL;

	QFileInfo pathComponents(recordName);
	QString pathRecord = pathComponents.absolutePath();
	QString nameRecord = pathComponents.baseName();

	setwfdb( QString("./;;/;%1").arg(pathRecord).toLatin1().data() );

	channel_count = isigopen( recordName.toLatin1().data(), NULL, 0 );	/* find out if this is wfdb compatible and how many channels there are */

	qDebug() << qPrintable( tr( "after isigopen(%1,NULL,0)      channel_count = %2" ).arg( recordName ).arg( channel_count ) );
	if ( channel_count >= 1 ) {
		wfdbSignalInfo = ( WFDB_Siginfo * ) malloc( channel_count * sizeof( WFDB_Siginfo ) );
		qDebug() << qPrintable( tr( "before isigopen(%1,wfdbSignalInfo,%2)" ).arg( recordName ).arg( channel_count ) );
		channel_count = isigopen( recordName.toLatin1().data(), wfdbSignalInfo, channel_count );
		qDebug() << qPrintable( tr( "after isigopen(%1,wfdbSignalInfo,%2)" ).arg( recordName ).arg( channel_count ) );
		samps_per_chan_per_sec = getifreq();
		signal_format_specifier = wfdbSignalInfo->fmt;
		if ( wfdbSignalInfo->gain == 0 ) {
			wfdbSignalInfo->gain = 200;
		}
	}

	return wfdbSignalInfo;
}
/* }}} */



/** {{{ EcgData::store_edfheader_field( QString fieldname, int fieldsize );
 */
void EcgData::store_edfheader_field( QByteArray header, QString fieldname, int fieldsize )
{
    static int stringoffset = 0;

    if ( fieldname == "<init>" ) {
        edfheader["ns"] = "0";
        stringoffset = 0;
        return;
    }

    /* if we are now processing the part of the header having multiple signals... */
    if ( edfheader["ns"].toInt() > 0 ) {
        for ( int ns = 0 ; ns < edfheader["ns"].toInt() ; ns++ ) {
            edfheader[fieldname + QString::number(ns)] = header.mid(stringoffset,fieldsize);
            stringoffset += fieldsize;
        }
    } else {
        edfheader[fieldname] = header.mid(stringoffset,fieldsize);
        stringoffset += fieldsize;
    }

    return;
}
/* }}} */


/** {{{ ror8 - rotate an 8-bit value right
 * @word: value to rotate
 * @shift: bits to roll
 */
static inline  unsigned char ror8( unsigned char word, unsigned int shift)
{
    return (word >> shift) | (word << (8 - shift));
}
/* }}} */



/** {{{ void EcgData::Load()
  @brief Load the data of the given signal file into channels arrays
  */
int EcgData::Load( QString filename )
{
    int i;
	long sampleCnt = 0L;

    qDebug() << QString("EcgData::Load(%1) %2 channels of fmt = %3   at %4 samples per second     ").arg(filename).arg(channel_count).arg(signal_format_specifier).arg(samps_per_chan_per_sec);


	if ( wfdbSignalInfo ) {

		WFDB_Sample *samp;

		samp = ( WFDB_Sample * ) malloc( channel_count * sizeof( WFDB_Sample ) );

		device_range_mV = 5;

		qDebug() << "\n" << QString( "wfdbSignalInfo : load(%1)     device_range_mV = %2      nsamp = %3" ).arg( filename ).arg( device_range_mV ).arg( ( int ) wfdbSignalInfo->nsamp ) << "\n";

		for ( int ch = 0 ; ch < 3 ; ch++ ) {
			fileEcgCache[ch].open();
			if ( fileEcgCache[ch].error() ) { qDebug() << qPrintable(QString("fileEcgCache[%1].open() -> %2").arg(ch).arg(fileEcgCache[ch].errorString())); }
		}

		int samplePos = 0;
		while ( getvec( samp ) > 0 ) {

			for ( int ch = 0; ch < channel_count; ch++ ) {
				if ( samp[ch] == -32768 ) {
					samp[ch] = ( 1 << wfdbSignalInfo->adcres ) / 2;
				}

				int32_t convertedSample = range_per_sample / 2 + ROUND2INT( ( ( double ) samp[ch] - ( double ) wfdbSignalInfo->adczero )
										  * range_per_sample / device_range_mV / wfdbSignalInfo->gain );

				STORE_INTO_CHDATA( ch, samplePos, convertedSample );
			}

			samplePos++;
			if ( (samplePos % 1000) == 0 ) {
				SHOW_PROGRESS_AND_WATCHFOR_CANCEL( (samplePos / 1000) % 10000 );
			}
		}

		samps_per_chan_per_sec = getifreq();
		datalen_secs = ( int ) ( samplePos / samps_per_chan_per_sec );

		qDebug() << QString( "wfdbSignalInfo : datalen_secs = %1       sps = %2" ).arg( datalen_secs ).arg( getifreq() );

	} else {
		if ( ! QFile::exists(filename) ) {
			return false;
		}

		QFile file(filename);
		file.open(QIODevice::ReadOnly);
		QDataStream filein(&file);

		// get the file size
		qint64 filesize = file.size();
		if ( filesize == -1 ) {
			return false;
		}
		emit load_size( (int) filesize );

		qDebug() << "\n" << qPrintable(tr("EcgData::Load()           WITHOUT wfdb             file size = %1").arg(filesize)) << "\n";

		// read the whole file into memory
		char *rawdata = new char[filesize];
		if ( filein.readRawData(rawdata, filesize) != filesize ) {
			delete[] rawdata;
			return false;
		}
		file.close();

		for ( int ch = 0 ; ch < 3 ; ch++ ) {
			fileEcgCache[ch].open();
			if ( fileEcgCache[ch].error() ) { qDebug() << qPrintable(QString("fileEcgCache[%1].open() -> %2").arg(ch).arg(fileEcgCache[ch].errorString())); }
		}


		datalen_secs = (int) (filesize / bytes_per_samp / samps_per_chan_per_sec / channel_count);

		qDebug() << qPrintable(tr("EcgData::Load()   about to parse file having signal_format_specifier = %1     and filesize = %2").arg(signal_format_specifier).arg(filesize));

		/* http://www.physionet.org/physiotools/wag/signal-5.htm describes the format. */
		switch ( signal_format_specifier ) {

			default:
			case 81:
				{
					emit load_size( ( int ) datalen_secs * samps_per_chan_per_sec );
					device_range_mV = 5;

					for ( i = 0; i < datalen_secs * samps_per_chan_per_sec; i++ ) {
						SHOW_PROGRESS_AND_WATCHFOR_CANCEL( i );
						for ( int ch = 0; ch < channel_count; ch++ ) {
							unsigned char rawValue = ( signed char ) rawdata[i * channel_count + ch] * 2 + 0x80;

							STORE_INTO_CHDATA( ch, i, ROUND2INT( rawValue * range_per_sample / pow( 2, 8 * bytes_per_samp ) ) );
						}
					}

					if ( !data_loading ) {
						/* reset the datalen_secs in case the data loading was canceled part way through */
						datalen_secs = ( int ) ( i / samps_per_chan_per_sec );
					}
				}
				break;
		}

		delete[] rawdata;
	}

	for ( int ch = 0 ; ch < channel_count ; ch++ ) {
        chdata[ch] = (quint16 *) fileEcgCache[ch].map( 0, fileEcgCache[ch].size() );

		if ( fileEcgCache[ch].error() ) {
			qDebug() << qPrintable(QString("fileEcgCache[%1].map( 0, %2 ) -> %3").arg(ch).arg(fileEcgCache[ch].size()).arg(fileEcgCache[ch].errorString()));
		}

		if ( chdata[ch] == NULL ) {
			datalen_secs = 0;
		}
	}

    return true;
}
/* }}} */


/* {{{ void EcgData::get()
   @Brief
 */
quint16 * EcgData::get( int channel_num, long start_time_samps, long duration_samps )
{
    if ( (ushort) channel_num > channel_count ) {
        channel_num = 0;
    }
    /* if past the end of valid data, then point to the end of data */
    if ( start_time_samps + duration_samps > datalen_secs * samps_per_chan_per_sec ) {
        start_time_samps = datalen_secs * samps_per_chan_per_sec - duration_samps;
    }
    if ( start_time_samps < 0 ) {
        start_time_samps = 0;
    }
    return &(get_data_channel(channel_num)[start_time_samps]);
}
/* }}} */


/* {{{ void EcgData::sample_count()
   @Brief
 */
long EcgData::sample_count()
{
    return samps_per_chan_per_sec * 8;
}
/* }}} */

