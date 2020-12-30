/**
 * @file ecgdata.h
*/

#if !defined(ECGDATA_H)
#define ECGDATA_H

#include <QtWidgets>

#include "wfdb/wfdb.h"
#include "wfdb/ecgmap.h"
#include "wfdb/ecgcodes.h"


#define CHANNEL_MAX		(12)

#define MASK_THESE_BITS(b,bits)	((unsigned long) ((b) & ((1 << (bits)) - 1)))
#define MASK_4_BIT(b)	MASK_THESE_BITS(b, 4)
#define MASK_8_BIT(b)	MASK_THESE_BITS(b, 8)
#define MASK_10_BIT(b)	MASK_THESE_BITS(b,10)
#define MASK_12_BIT(b)	MASK_THESE_BITS(b,12)


#define ECG_HEADER_UNIVERSAL	(QFileInfo(filename).absolutePath() + "/" + QString("ecg.hea"))


/* {{{ class EcgData
   @brief	class to manage streams of ECG data
*/
class EcgData : public QObject
{
    Q_OBJECT

public:
    EcgData( QWidget *parent = NULL );
    EcgData( QString filename, QWidget *parent = NULL );
    ~EcgData();

    ulong size() { return datalen_secs * samps_per_chan_per_sec; }	/* return samples per channel */

    QString parse_header( QString filename );
	WFDB_Siginfo * wfdbOpen( QString filename );
    int Load( QString filename );
    long sample_count();
    quint16 *get( int channel_num, long start_time_samps, long duration_samps );
    quint16 *get_data_channel( int channel_num ) { return chdata[channel_num]; }

    QString file_name;
    double range_per_sample;
    double device_range_mV;
    int channel_count;
    int datalen_secs;
    int samps_per_chan_per_sec;
    int signal_format_specifier;
    float bytes_per_samp;
    bool data_loading;

    QDateTime viewableDateTime;

	WFDB_Siginfo *wfdbSignalInfo;

private:
    void store_edfheader_field( QByteArray header, QString fieldname, int fieldsize );

    QHash<QString,QString>	edfheader;

    int edf_nr;
    int edf_ns;
    int edf_bytes_in_header;
    int edf_samps_per_record;
    float edf_record_duration_secs;

    QTemporaryFile fileEcgCache[3];
    quint16 * chdata[12];


public slots:
    void cancel_data_loading();

signals:
    void load_size( int filesize );
    void data_loaded_so_far( int loaded );
    void loading_finished();
    void pacer_spike_found( long samplePos );

};
/* }}} */

#endif
