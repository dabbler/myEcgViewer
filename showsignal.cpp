/**
 * @file showsignal.cpp
*/

#include <QDebug>
#include <QString>

#include "myheader.h"
#include "mainwindow.h"
#include "showsignal.h"
#include "utils.h"


/** {{{ ShowSignal::ShowSignal()
    @brief Define a constructor for my canvas
*/
ShowSignal::ShowSignal(QWidget *parent, EcgData *theEcgData )
    : QWidget(parent), m_ecgdata(theEcgData)
{
    if ( m_ecgdata == NULL ) {
        m_ecgdata = new EcgData;
    }
    curpos_samples = 0;
	first_beat_found = -1;
	cached_middle_beat_found = -1;
    m_testspeed = false;
    m_test_antialiasing = false;
	yOffsetDragged = 0;
    pacerPosition.clear();

    comboViewType = new QComboBox();
    comboViewType->addItem("", (int) VIEWTYPE_NONE );
    comboViewType->addItem("8 Second Strip", (int) VIEWTYPE_8_SECOND_STRIP );
    comboViewType->addItem("Full Disclosure", (int) VIEWTYPE_FULL_DISCLOSURE );
    connect( comboViewType, SIGNAL(currentIndexChanged(int)), this, SLOT(setViewType(int)) );
    setViewType( VIEWTYPE_8_SECOND_STRIP );

    setAutoFillBackground(true);

    /* set the background to black */
    QPalette pal = palette();
    pal.setColor(backgroundRole(), QColor("#202020") );
    pal.setColor(backgroundRole(), QColor("white") );
    setPalette(pal);


    setAttribute(Qt::WA_DeleteOnClose);
    isUntitled = true;

    xScaling = 1.0;
    yScaling = 1.0;

    gain_mm_per_mV = 10;
    zoom_amount = 1.0;
    m_zoom_x = 0.0;
    m_zoom_y = 0.0;
    setMouseTracking(true);

    is_printing = false;
    display_extra = DISPLAY_EXTRA_NONE;

    setObjectName("ShowSignal");
}
/* }}} */


/** {{{ ShowSignal::~ShowSignal()
*/
ShowSignal::~ShowSignal()
{
    delete comboViewType;
    delete m_ecgdata;
}
/* }}} */


/** {{{ void ShowSignal::showEvent( QShowEvent * event )
    @brief Show event which happens whenever this window gets displayed
*/
void ShowSignal::showEvent( QShowEvent * event )
{
    Q_UNUSED(event);

    glb_mainwindow->getComboEcgGainWidget()->setCurrentIndex( glb_mainwindow->getComboEcgGainWidget()->findText( QString::number(gain_mm_per_mV) + " mm/mV" ) );
}

/* }}} */


/** {{{ void ShowSignal::hideEvent( QHideEvent * event )
    @brief Hide event which happens whenever this window gets hidden
*/
void ShowSignal::hideEvent( QHideEvent * event )
{
    Q_UNUSED(event);

}

/* }}} */


/** {{{ void ShowSignal::focusInEvent( QFocusEvent *event )
    @brief Show event which happens whenever this window gets displayed
*/
void ShowSignal::focusInEvent( QFocusEvent *event )
{
    Q_UNUSED(event);

}

/* }}} */


/** {{{ void ShowSignal::focusOutEvent( QFocusEvent *event )
    @brief Hide event which happens whenever this window gets hidden
*/
void ShowSignal::focusOutEvent( QFocusEvent *event )
{
    Q_UNUSED(event);

}

/* }}} */



/** {{{ void ShowSignal::setViewType( int newViewType )
 */
void ShowSignal::setViewType( int newViewType )
{
    // qDebug() << "ShowSignal::setViewType(" << newViewType << ") for" << windowTitle();
    setComboViewType( newViewType );
    zoom_amount = 1.0;
    update();
}
/* }}} */


/** {{{ void ShowSignal::store_pacer_position( long samplePos )
 */
void ShowSignal::store_pacer_position( long samplePos )
{
    samplePos -= 100;    /**< Compensate for the 201 tap filter that misreports where the pacer spike was detected. */


    if ( samplePos < 0 ) {
        // qDebug() << "\n\n" << windowTitle() << " : " << "\n";
    } else {

#ifdef DEBUG_PRINT_PACER_DETECTIONS
        int sampleRate = 256;

        // sampleRate = m_ecgdata->samps_per_chan_per_sec;

        qDebug() << qPrintable( QString("%1:%2:%3.%4")
				.arg( ((samplePos/sampleRate) / 60 / 60), 2, 10, QLatin1Char('0'))
				.arg( ((samplePos/sampleRate) / 60) % 60, 2, 10, QLatin1Char('0'))
				.arg( ((samplePos/sampleRate)     ) % 60, 2, 10, QLatin1Char('0'))
				.arg( ((10 * samplePos/sampleRate)     ) % 10, 1, 10, QLatin1Char('0'))
				);
#endif

        pacerPosition.append(samplePos);

        int minutePos = samplePos / m_ecgdata->samps_per_chan_per_sec / 60;
        paceBeatsPerMinute[minutePos] += 1;
    }
}
/* }}} */



/** {{{ int ShowSignal::load_annotation_file( char *ext )
 */
int ShowSignal::load_annotation_file( char *recordName, char *ext )
{
	int retVal;
	WFDB_Anninfo annoInfoAF;
	QString pathRecord;
	QString nameRecord;


	QFileInfo pathComponents(recordName);
	pathRecord = pathComponents.absolutePath();
	nameRecord = pathComponents.baseName();

	setwfdb( QString("./;;/;%1").arg(pathRecord).toLatin1().data() );

	qDebug() << "wfdbpath = " << getwfdb();

	qDebug() << QString("load_annotation_file(%1,%2)    setenv(DB,%3)").arg(nameRecord).arg(ext).arg(pathRecord);

#define SIMPLE_READ_ANNO
#ifdef SIMPLE_READ_ANNO
	annoInfoAF.name = ext;
	annoInfoAF.stat = WFDB_READ;
	if ( annopen( nameRecord.toLatin1().data(), &annoInfoAF, 1 ) >= 0 ) {
		m_beats.clear();
		WFDB_Annotation ann;
		while ( getann( 0, &ann ) == 0 ) {
// #define SHOW_ALL_ANNOTATIONS
#ifdef SHOW_ALL_ANNOTATIONS
			qDebug() << QString( "DBR: %1  annstr(%2) = %3    aux='%4'" )
				.arg( mstimstr( ann.time ) )
				.arg( ( int ) ann.anntyp )
				.arg( annstr( ann.anntyp ) )
				.arg( ( char * ) ann.aux )
				.toLatin1().constData();
#endif
            BeatInfo bb( ann.time, ann.anntyp );
            bb.putSubtype(ann.subtyp);
            bb.putAnnotationString( ( char * ) ann.aux );
            m_beats.append( bb );
		}

		retVal = true;

	} else {
		retVal = false;

		// qDebug() << QString("load_annotation_file(%1,%2)   ERROR = %3").arg(nameRecord).arg(ext).arg( wfdb_error() );
	}

#else

	if ( annopen( RECORDNAME, &annoInfoAF, 1 ) >= 0 ) {
		qDebug() << QString("parse_header()    annopen(%1.%2)").arg(RECORDNAME).arg(annoInfoAF.name);
		WFDB_Annotation ann;
		while ( getann( 0, &ann ) == 0 ) {

// #define TRY_OTHER_ANNOTATION_READER
#ifdef TRY_OTHER_ANNOTATION_READER
			qDebug() << QString( "%1  annstr(%2) = %3    aux='%4'" )
				.arg( mstimstr( ann.time ) )
				.arg( ( int ) ann.anntyp )
				.arg( annstr( ann.anntyp ) )
				.arg( ( char * ) &(ann.aux[1]) )
				.toLatin1().constData();
#endif
			switch ( ann.anntyp ) {
				case NORMAL:
				case LBBB:
				case RBBB:
				case ABERR:
				case PVC:
				case NPC:
				case SVPB:
				case VESC:
				case NESC:
				case PACE:
				case UNKNOWN:
					{
						BeatInfo bb( ann.time, ann.anntyp );
						bb.putAnnotationString( ( char * ) &(ann.aux[1]) );
						m_beats.append( bb );
					}
					break;

				case FUSION:
				case APC:
					{
						BeatInfo bb( ann.time, NORMAL );
						bb.putAnnotationString( ( char * ) &(ann.aux[1]) );
						m_beats.append( bb );
					}
					break;

				case NOTE:
				case RHYTHM:
				case '"':
					{
						static bool g_withinAF = false;
						QString val((char*)&(ann.aux[1]));
						int type = RHYTHM;

						if ( QString(val).contains("TACH") ) {
							qDebug() << val;
							val = "(SVT";
							type = 'T';
						}
						if ( QString(val).contains("BRAD") ) {
							qDebug() << val;
							val = "(SBR";
							type = 'B';
						}
						if ( QString(val).contains("PAUS") ) {
							qDebug() << val;
							val = "(PSE";
							type = 'u';
						}
						if ( QString(val).contains("NSR") ) {
							val = "(N";
							type = RHYTHM;
						}
						if ( QString(val).contains("(N") ) {
							if ( g_withinAF ) {
								g_withinAF = false;
								type = 'a';
								// qDebug() << "              OFF";
							}
						}
						if ( QString(val).contains("AF") ) {
							val = "(AFIB";
							type = 'A';
							g_withinAF = true;
							// qDebug() << val;
						}

						for ( int b = m_beats.size() - 1 ; b >= 0 ; b-- ) {
							if ( (m_beats.at(b).pos_samps - ann.time) < 5 ) {
								BeatInfo theBeat( m_beats.at(b) );
								theBeat.type = type;
								theBeat.putAnnotationString( ( char * ) val.toLatin1().data() );
								m_beats.replace(b,theBeat);
								break;
							}
						}
					}
					break;

				default:
					break;
			}
		}

	}
#endif

	return retVal;
}
/* }}} */



/** {{{ void ShowSignal::newFile()
    @brief Create a new file
*/
void ShowSignal::newFile()
{
    static int sequenceNumber = 1;

    isUntitled = true;
    curFile = tr("ECG dataset %1").arg(sequenceNumber++);
    setWindowTitle(curFile + "[*]");


    m_ecgdata = new EcgData( "ecg/ecg3ch.dat" );
    SetPos( (int) (14.9 * 60 * m_ecgdata->samps_per_chan_per_sec) );

#ifdef QT_DEBUG
    qDebug() << "ShowSignal::newFile() -> New(" << curFile << ")";
#endif

/*
    connect(document(), SIGNAL(contentsChanged()), this, SLOT(documentWasModified()));
*/
}
/* }}} */



/** {{{ ShowSignal::minimumSizeHint()
    @brief Set minimum canvas size
*/
QSize ShowSignal::minimumSizeHint() const
{
    return QSize(100, 100);
}
/* }}} */


/** {{{ ShowSignal::sizeHint()
    @brief Set canvas size
*/
QSize ShowSignal::sizeHint() const
{
    return parentWidget()->size();
}
/* }}} */


/** {{{ ShowSignal::resizeEvent()
    @brief Resize event
*/
void ShowSignal::resizeEvent( QResizeEvent * event )
{
    Q_UNUSED(event);
}

/* }}} */









/** {{{ void ShowSignal::wheelEvent( QWheelEvent *e )
    @brief Mouse wheel events
*/
void ShowSignal::wheelEvent( QWheelEvent *event )
{
    float spinning = (event->delta() > 0) ? zoom_amount/10.0 : -zoom_amount/10.0;

    if ( event->modifiers() & Qt::CTRL ) {
#ifdef ANTIQUE
        xScaling += (event->delta() > 0) ? 0.05 : -0.05;
        yScaling += (event->delta() > 0) ? 0.05 : -0.05;
#endif
    }
    if ( event->modifiers() & Qt::SHIFT ) {
    }
    if ( event->modifiers() & Qt::ALT ) {
    }

    if ( zoom_amount > 1.0 ) {
        m_zoom_x = (float) event->pos().x();
        m_zoom_y = (float) event->pos().y();
    } else {
        m_zoom_x = (float) width() / 2;
        m_zoom_y = (float) height() / 2;
    }

/*
qDebug() << event->pos();
*/


    double zoom_limit;

    switch ( getComboViewTypeIndex() ) {
        case VIEWTYPE_FULL_DISCLOSURE:
            zoom_limit = 30.0;
            break;
        case VIEWTYPE_8_SECOND_STRIP:
        default:
            zoom_limit = 10.0;
            break;
    }

    zoom_amount -= spinning;
    /* don't let the zoom grow too big */
    if ( zoom_amount > zoom_limit ) {
        zoom_amount = zoom_limit;
    }
    /* don't let the zoom grow too big */
    if ( zoom_amount < 0.25 ) {
        zoom_amount = 0.25;
    }
    /* if the zoom is pretty near zero, then make it zero */
    if ( fabs(zoom_amount - 1.0) < 0.1 ) {
        zoom_amount = 1.0;
    }

    update();
}
/* }}} */


/** {{{ void ShowSignal::mousePressEvent(QMouseEvent *event)
    @brief Mouse press event
*/
void ShowSignal::mousePressEvent(QMouseEvent *event)
{
	if ( event->buttons() & Qt::RightButton ) {
		zoom_amount = 1.0;
		update();
	}
	if ( event->buttons() & Qt::MidButton ) {
		display_extra = (display_extra + 1) % COUNT_OF_DISPLAY_EXTRA;
		update();
	}

	lastPos = event->pos();
#ifdef QT_DEBUG
	qDebug("mousePressEvent(%d,%d)", event->pos().x(), event->pos().y() );
#endif
}
/* }}} */


/** {{{ void ShowSignal::mouseMoveEvent(QMouseEvent *event)
    @brief Mouse move event
*/
void ShowSignal::mouseMoveEvent(QMouseEvent *event)
{
    int dx = event->x() - lastPos.x();
	int dy = event->y() - lastPos.y();

// qDebug() << "ShowSignal::mouseMoveEvent(" << event->pos();

    if ( event->buttons() & Qt::LeftButton ) {
        long offset = GetPos();
        double multiplier = 1.0;

        if ( event->modifiers() & Qt::CTRL ) {
            multiplier *= 1.0 / 3;
        }
        if ( event->modifiers() & Qt::SHIFT ) {
            multiplier *= 5.0;
        }
        if ( event->modifiers() & Qt::ALT ) {
            multiplier *= 2.0;
        }

		yOffsetDragged += ROUND2INT( multiplier * (dy) );

        offset -= ROUND2INT( multiplier * (dx) );
        SetPos( offset );
        update();

    } else if ( event->buttons() & Qt::RightButton ) {

		yOffsetDragged = 0;
		update();

    } else {

        if ( zoom_amount > 1.0 ) {
            m_zoom_x = (float) event->pos().x();
            m_zoom_y = (float) event->pos().y();
            update();
        }

#ifdef QT_DEBUG
        // qDebug("mouseMoveEvent(%d,%d)", event->pos().x(), event->pos().y() );
#endif

    }
    lastPos = event->pos();
}
/* }}} */




/** {{{ void ShowSignal::keyPressEvent( QKeyEvent &event)
  @brief Key press event
  */
void ShowSignal::keyPressEvent( QKeyEvent * event )
{
	long offset = GetPos();

	int key = 0;
	if ( event->text().size() > 0 ) {
		key = (const char) event->text().at(0).toLatin1();
	} else {
		key = event->key();
	}

#ifdef QT_DEBUG
	// qDebug() << "ShowSignal::keyPressEvent: " << event->text() 	<< " having key: " << event->key() << " or " << key << "   WHERE Qt::Key_Delete = " << Qt::Key_Delete;
#endif

	long samps_per_sec = m_ecgdata->samps_per_chan_per_sec;

	double multiplier = 1.0;
	if ( event->modifiers() & Qt::CTRL ) {
		multiplier *= 1.0 / samps_per_sec;
	}
	if ( event->modifiers() & Qt::SHIFT ) {
		multiplier *= 10.0;
	}
	if ( event->modifiers() & Qt::ALT ) {
		multiplier *= 2.0;
	}


	switch ( key ) {

		case 'D':
		case 'd':
		case 127:
		case Qt::Key_Delete:
			break;

		case 'n': {
					  if ( ! m_beats.size() ) {
						  return;
					  }

					  long sample_count = m_ecgdata->samps_per_chan_per_sec * ECG_DISPLAY_WINDOW_SIZE_SECONDS;

					  int b = middle_beat_showing() + 1;
					  if ( b < m_beats.size() ) {
						  offset = m_beats[b].pos_samps - sample_count / 2 - 1;
					  }
				  }
				  break;

		case 'N': {
					  if ( ! m_beats.size() ) {
						  return;
					  }

					  long sample_count = m_ecgdata->samps_per_chan_per_sec * ECG_DISPLAY_WINDOW_SIZE_SECONDS;

					  int b = middle_beat_showing() - 1;
					  if ( b > 0 ) {
						  offset = m_beats[b].pos_samps - sample_count / 2 - 1;
					  }
				  }
				  break;


		case 'v': {
					  if ( ! m_beats.size() ) {
						  return;
					  }

					  long sample_count = m_ecgdata->samps_per_chan_per_sec * ECG_DISPLAY_WINDOW_SIZE_SECONDS;

					  /* TODO: this linear search is slow */
					  for ( int b = middle_beat_showing() + 1 ; b < m_beats.size() ; b++ ) {
						  if ( m_beats[b].type == PVC ) {
							  offset = m_beats[b].pos_samps - sample_count / 2 - 1;
							  break;
						  }
					  }
				  }
				  break;

		case 'V': {
					  if ( ! m_beats.size() ) {
						  return;
					  }

					  long sample_count = m_ecgdata->samps_per_chan_per_sec * ECG_DISPLAY_WINDOW_SIZE_SECONDS;

					  /* TODO: this linear search is slow */
					  for ( int b = middle_beat_showing() - 1 ; b > 0 ; b-- ) {
						  if ( m_beats[b].type == PVC ) {
							  offset = m_beats[b].pos_samps - sample_count / 2 - 1;
							  break;
						  }
					  }
				  }
				  break;


		case 'u': {
					  if ( ! m_beats.size() ) {
						  return;
					  }

					  long sample_count = m_ecgdata->samps_per_chan_per_sec * ECG_DISPLAY_WINDOW_SIZE_SECONDS;

					  /* TODO: this linear search is slow */
					  for ( int b = middle_beat_showing() + 1 ; b < m_beats.size() ; b++ ) {
						  if ( m_beats[b].type == 'u' ) {
							  offset = m_beats[b].pos_samps - sample_count / 2 - 1;
							  break;
						  }
					  }
				  }
				  break;

		case 'U': {
					  if ( ! m_beats.size() ) {
						  return;
					  }

					  long sample_count = m_ecgdata->samps_per_chan_per_sec * ECG_DISPLAY_WINDOW_SIZE_SECONDS;

					  /* TODO: this linear search is slow */
					  for ( int b = middle_beat_showing() - 1 ; b > 0 ; b-- ) {
						  if ( m_beats[b].type == 'u' ) {
							  offset = m_beats[b].pos_samps - sample_count / 2 - 1;
							  break;
						  }
					  }
				  }
				  break;


		case Qt::ALT:
				  break;
		case Qt::CTRL:
				  break;
		case Qt::SHIFT:
				  break;

		case Qt::Key_Plus:
				  gain_mm_per_mV += 1;
				  update();
				  break;
		case Qt::Key_Minus:
				  gain_mm_per_mV -= 1;
				  if ( gain_mm_per_mV <= 0 ) {
					  gain_mm_per_mV = 1;
				  }
				  update();
				  break;

		case Qt::Key_Left:
				  offset -= ROUND2INT( multiplier * (1 * samps_per_sec) );

				  break;
		case Qt::Key_Down:
				  offset += ROUND2INT( multiplier * (ECG_DISPLAY_WINDOW_SIZE_SECONDS * samps_per_sec) );
				  break;
		case Qt::Key_Up:
				  offset -= ROUND2INT( multiplier * (ECG_DISPLAY_WINDOW_SIZE_SECONDS * samps_per_sec) );
				  break;
		case Qt::Key_Home:
				  offset = 0;
				  break;
		case Qt::Key_End:
				  offset = (m_ecgdata->datalen_secs - ECG_DISPLAY_WINDOW_SIZE_SECONDS) * samps_per_sec;
				  break;
		case Qt::Key_PageDown:
				  offset += ROUND2INT( multiplier * (15*60 * samps_per_sec) );
				  break;
		case Qt::Key_PageUp:
				  offset -= ROUND2INT( multiplier * (15*60 * samps_per_sec) );
				  break;


		case 'p':
				  {
					  QVector<quint32>::iterator pPacer = qLowerBound( pacerPosition.begin(), pacerPosition.end(), (quint32) (GetPos() + samps_per_sec*ECG_DISPLAY_WINDOW_SIZE_SECONDS/2 + 2) );
					  offset = *pPacer - samps_per_sec * ECG_DISPLAY_WINDOW_SIZE_SECONDS/2;
				  }
				  break;

		case 'P':
				  {
					  QVector<quint32>::iterator pPacer = qLowerBound( pacerPosition.begin(), pacerPosition.end(), (quint32) (GetPos() + samps_per_sec*ECG_DISPLAY_WINDOW_SIZE_SECONDS/2 - 2) );
					  pPacer--;
					  offset = *pPacer - samps_per_sec * ECG_DISPLAY_WINDOW_SIZE_SECONDS/2;
				  }
				  break;


		case Qt::Key_Right:
				  offset += ROUND2INT( multiplier * (1 * samps_per_sec) );
				  break;
		case ' ':
				  offset += ROUND2INT( multiplier * (1 * samps_per_sec) );
				  break;

		case 'i':
				  m_test_antialiasing = !  m_test_antialiasing;
#ifdef QT_DEBUG
				  qDebug("m_test_antialiasing  = %d", m_test_antialiasing );
#endif
				  break;

		case '1':
				  break;

		case 'S':
		case 's':
				  {
#ifdef ANTIQUE
					  static QTimer *smooth_advance_timer = NULL;
					  if ( smooth_advance_timer == NULL ) {
						  smooth_advance_timer = new QTimer(this);
						  QObject::connect( smooth_advance_timer, SIGNAL(timeout()), this, SLOT(smooth_advance()));
					  }
					  m_testspeed = ! m_testspeed;
					  if ( m_testspeed ) {
						  offset = 0;
						  smooth_advance_timer->start(SCROLL_CHUNK * 1000/128);
						  // QTimer::singleShot( 0, this, SLOT(smooth_advance()) );
					  } else {
						  smooth_advance_timer->stop();
					  }
#endif
				  }
				  break;

	}

	if ( offset > (m_ecgdata->datalen_secs - ECG_DISPLAY_WINDOW_SIZE_SECONDS) * samps_per_sec ) {
		offset = (m_ecgdata->datalen_secs - ECG_DISPLAY_WINDOW_SIZE_SECONDS) * samps_per_sec;
	}
	if ( offset < 0 ) {
		offset = 0;
	}

	if ( offset != GetPos() ) {
		SetPos( offset );
		update();
	}
	event->ignore();
}
/* }}} */





/** {{{ ShowSignal::paintEvent()
  @brief  Paint events are sent to widgets that need to update themselves
  */
void ShowSignal::paintEvent( QPaintEvent * event )
{
    Q_UNUSED(event);

    QPainter painter(this);

    painter.setFont( QFont("Helvetica",22) );
    if ( m_test_antialiasing ) {
        painter.setRenderHint(QPainter::Antialiasing, true);
    }

    /* rescale the window when the window gets resized */
    float pixelSizeWidth = ( 8.0 /* sec */ * 2.5 /* cm */ * (painter.device()->logicalDpiX() / 2.54) );
    float pixelSizeHeight = ( (STRIPHEIGHT_MM + 10) * (painter.device()->logicalDpiY() / 25.4) );
    float scaleOntoScreen = parentWidget()->size().width() / pixelSizeWidth;

    if ( pixelSizeHeight * scaleOntoScreen > parentWidget()->size().height() ) {
        scaleOntoScreen = parentWidget()->size().height() / pixelSizeHeight;
    }

    painter.scale( scaleOntoScreen, scaleOntoScreen );

/*
    QQQ("workecg.log") << "ShowSignal::paintEvent()   "
        << " scaleOntoScreen = " << scaleOntoScreen
        << " ( (STRIPHEIGHT_MM + 9) * (painter.device()->logicalDpiY() * 2.54) ) = " << ( (STRIPHEIGHT_MM + 9) * (painter.device()->logicalDpiY() * 2.54) )
        << " parentWidget()->size().height() = " << parentWidget()->size().height()
        << " pixelSizeWidth = " << pixelSizeWidth
        << " pixelSizeHeight = " << pixelSizeHeight
        ;
*/

    painter.translate( m_zoom_x, m_zoom_y );
    painter.scale( zoom_amount, zoom_amount );
    painter.translate( -m_zoom_x, -m_zoom_y );
    // painter.translate( +(m_zoom_x-VIEWWIDTH/2), +(m_zoom_y-VIEWHEIGHT/2) );

    switch ( getComboViewTypeIndex() ) {
        case VIEWTYPE_FULL_DISCLOSURE:
            RenderFullDisclosure( &painter );
            break;
        case VIEWTYPE_8_SECOND_STRIP:
            Render( &painter );
            break;
    }
    emit focusChanged();
}
/* }}} */


/** {{{ void ShowSignal::Render( QPainter dc, QPrinter *printer )
  @brief Define the repainting behaviour
  */
void ShowSignal::Render( QPainter * dc, QPrinter *printer )
{
    Q_UNUSED(printer);
    ShowGrid( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS*5, STRIPHEIGHT_MM /* mm */, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, PAGE_MARGIN_TOP );
    ShowData( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, 0, PAGE_MARGIN_TOP );
    ShowAnnotation( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, PAGE_MARGIN_TOP );

    ShowHeader( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling);
}
/* }}} */


/** {{{ void ShowSignal::RenderStrip( QPainter dc, QPrinter *printer )
  @brief Define the repainting behaviour
  */
void ShowSignal::RenderStrip( QPainter * dc, QPrinter *printer )
{
    Q_UNUSED(printer);
    ShowGrid( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS*5, STRIPHEIGHT_MM /* mm */, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, PAGE_MARGIN_TOP + 0 * (STRIPHEIGHT_MM + PAGE_MARGIN_BETWEEN) );
    ShowData( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, 0, MM2DEVDOTS(dc,15 + PAGE_MARGIN_TOP + 0 * (STRIPHEIGHT_MM + PAGE_MARGIN_BETWEEN)), 1 );
    ShowAnnotation( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, PAGE_MARGIN_TOP + 0 * (STRIPHEIGHT_MM + PAGE_MARGIN_BETWEEN), 1 );

    ShowGrid( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS*5, STRIPHEIGHT_MM /* mm */, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, PAGE_MARGIN_TOP + 1 * (STRIPHEIGHT_MM + PAGE_MARGIN_BETWEEN) );
    ShowData( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, 0, MM2DEVDOTS(dc,15 + PAGE_MARGIN_TOP + 1 * (STRIPHEIGHT_MM + PAGE_MARGIN_BETWEEN)), 2 );
    ShowAnnotation( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, PAGE_MARGIN_TOP + 1 * (STRIPHEIGHT_MM + PAGE_MARGIN_BETWEEN), 2 );

    ShowGrid( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS*5, STRIPHEIGHT_MM /* mm */, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, PAGE_MARGIN_TOP + 2 * (STRIPHEIGHT_MM + PAGE_MARGIN_BETWEEN) );
    ShowData( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, 0, MM2DEVDOTS(dc,15 + PAGE_MARGIN_TOP + 2 * (STRIPHEIGHT_MM + PAGE_MARGIN_BETWEEN)), 3 );
    ShowAnnotation( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling, PAGE_MARGIN_TOP + 2 * (STRIPHEIGHT_MM + PAGE_MARGIN_BETWEEN), 3 );

    ShowHeader( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling);
}
/* }}} */


/** {{{ void ShowSignal::RenderFullDisclosure( QPainter * dc, QPrinter *printer )
  @brief Define the repainting behaviour
  */
void ShowSignal::RenderFullDisclosure( QPainter * dc, QPrinter *printer )
{
    QPen pen_text = QPen( QColor("black"), 0, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin );

    long sample_count = m_ecgdata->samps_per_chan_per_sec * m_ecgdata->datalen_secs - GetPos();

    /** count how many channels will be displayed */
    int channelsBeingPrinted = 0;
    for ( int ch = 0 ; ch < m_ecgdata->channel_count ; ch++ ) {
        if ( glb_mainwindow->isVisibleChan[ch] ) {
            channelsBeingPrinted++;
        }
    }
    channelsBeingPrinted = 1;

    int device_dots_per_mm = ROUND2INT( dc->device()->logicalDpiY() / 25.4 );

    int yPageTopPos = 10 * device_dots_per_mm;

    double scalingDownSize = 0.93 * 8.0 / 60.0;

    int yDistanceBetweenStrips;

    if ( printer ) {
        yDistanceBetweenStrips = (int) ((dc->device()->height() - yPageTopPos) / ((60.0) / channelsBeingPrinted + 1));
//		qDebug() << "yDistanceBetweenStrips B: " << yDistanceBetweenStrips << "            height() =" << dc->device()->height() << "heightMM() =" << dc->device()->heightMM() << "logicalDpiY() =" << dc->device()->logicalDpiY();
    } else {
        yDistanceBetweenStrips = (int) (scalingDownSize * STRIPHEIGHT_MM * 70/100 * channelsBeingPrinted / 2 * device_dots_per_mm);
//		qDebug() << "yDistanceBetweenStrips A: " << yDistanceBetweenStrips << "            height() =" << dc->device()->height() << "heightMM() =" << dc->device()->heightMM() << "logicalDpiY() =" << dc->device()->logicalDpiY();
    }



    QFont fontTimeDisplay("Helvetica",7);
    dc->setFont( fontTimeDisplay );
    QRect textrect;
    textrect = dc->boundingRect( textrect, Qt::AlignVCenter | Qt::AlignRight, "00:00:00 " );

    qreal baseline_offset = (STRIPHEIGHT_MM * device_dots_per_mm * (0 + 1) / (channelsBeingPrinted + 1)) * scalingDownSize;

    long saveCurrentPos = GetPos();

    int yPos = yPageTopPos;

    qreal save_gain_mm_per_mV = gain_mm_per_mV;
    for ( int line = 0 ; sample_count > 0 ; line++, sample_count -= 60 * m_ecgdata->samps_per_chan_per_sec ) {

        // qDebug() << "device size:" <<  line * yDistanceBetweenStrips * scalingDownSize * device_dots_per_mm << dc->device()->height() << dc->device()->logicalDpiY();

        if ( (yPos + yDistanceBetweenStrips + baseline_offset) > dc->device()->height() ) {
            if ( printer ) {
                yPos = yPageTopPos;
                printer->newPage();
            } else {
                break;
            }
        }

        SetPos( saveCurrentPos + line * 60 * m_ecgdata->samps_per_chan_per_sec );

        // ShowGrid( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS*5, STRIPHEIGHT_MM /* mm */, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling );
        // ShowAnnotation( dc, ECG_DISPLAY_WINDOW_SIZE_SECONDS, xScaling, yScaling );

        int secondsOfDataToShow = min( 60, sample_count / m_ecgdata->samps_per_chan_per_sec );

        ShowData( dc, 0, secondsOfDataToShow,
                scalingDownSize, scalingDownSize,
                textrect.width(),
                yPos + baseline_offset );


        /* draw the time of the beginning of visible data on the display */
        int second_pos = GetPos() / m_ecgdata->samps_per_chan_per_sec;
        QString str = m_ecgdata->viewableDateTime.addSecs(second_pos).toString("hh:mm:ss");

        dc->setPen(pen_text);
        dc->setFont( fontTimeDisplay );
        dc->drawText(
                0,
                ROUND2INT( textrect.height()/2 + yPos + baseline_offset / 2 ),
                textrect.width(), yDistanceBetweenStrips,
                Qt::AlignVCenter | Qt::AlignLeft, str );

        yPos += 1 * yDistanceBetweenStrips;
    }

    gain_mm_per_mV = save_gain_mm_per_mV;

    SetPos( saveCurrentPos );
}
/* }}} */


/** {{{ void ShowSignal::ShowGrid( QPainter * dc, int cols, int height_mm, int ecgSeconds, double xScale, double yScale, int y_startpos )
  @brief Show an ECG gridlike thing
  */
void ShowSignal::ShowGrid( QPainter * dc, int cols, int height_mm, int ecgSeconds, double xScale, double yScale, int y_startpos )
{
    double boxX_200ms = ( 0.2 * ((float)dc->device()->logicalDpiX() * 2.5 / 2.54) );
    double boxY_5mm = ( 5.0 * ((float)dc->device()->logicalDpiY() / 25.4) ) * Y_SCALE_RATIO;
    double boxY_height_mm = ( height_mm * ((float)dc->device()->logicalDpiY() / 25.4) ) * Y_SCALE_RATIO;
    double boxY_startpos = ( y_startpos * ((float)dc->device()->logicalDpiY() / 25.4) );

    int gridbox_cols = cols;
    int gridbox_rows = STRIPHEIGHT_MM / 5;
    double grid_width = ( gridbox_cols * (float) dc->device()->logicalDpiX() / 25.4 );
    double grid_height = ( gridbox_rows * (float) dc->device()->logicalDpiY() / 25.4 );

    QPen pen_solid( QColor("#c8c8c8") );
    QPen pen_dotted( QColor("#e0e0e0") );

    if ( is_printing ) {
        pen_solid = QPen( QColor("#404040"), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin );
        pen_dotted = QPen( QColor("#404040"), 3, Qt::DotLine, Qt::FlatCap, Qt::MiterJoin );
    }
    dc->setPen(pen_dotted);

    for ( int i = 0 ; i <= cols ; i++ ) {
        if ( (i % 5) == 0 ) {
            dc->setPen(pen_solid);
        }
        dc->drawLine(
                ROUND2INT(xScale * (double)i * boxX_200ms),
                boxY_startpos,
                ROUND2INT(xScale * (double)i * boxX_200ms),
                ROUND2INT(boxY_startpos + (yScale * boxY_height_mm))
                );
        if ( (i % 5) == 0 ) {
            dc->setPen(pen_dotted);
        }
    }

    /* draw all the horizontal lines */
    for ( double j = boxY_startpos ; j <= (boxY_startpos + boxY_height_mm) + 13 ; j += boxY_5mm ) {
        dc->drawLine( 0, ROUND2INT(yScale * j), ROUND2INT(xScale * boxX_200ms * 5*ecgSeconds), ROUND2INT(yScale * j) );
    }

    dc->setPen( QPen( QColor("#808080") ) );
    dc->setPen( QPen( QColor("black") ) );

    if ( display_extra == DISPLAY_EXTRA_GRID_DOTS ) {
        for ( double j = 0 ; j < gridbox_rows ; j += 1 ) {
            for ( int i = 0 ; i < gridbox_cols ; i += 1 ) {
                for ( int x = 1 ; x < 5 ; x++ ) {
                    for ( int y = 1 ; y < 5 ; y++ ) {
                        dc->drawPoint(
                                ROUND2INT(xScale * ((double) (i * 5 + x) / gridbox_cols * grid_width)),
                                ROUND2INT(yScale * ((double) (j * 5 + y) / gridbox_rows * grid_height))
                                );
                    }
                }
            }
        }
    }
}
/* }}} */


/** {{{ void ShowSignal::ShowData( QPainter *dc, int ecgSeconds, double xScale, double yScale, int x_startpos_devicedots, int y_startpos_devicedots )
  @brief Show the ECG data
  */
void ShowSignal::ShowData( QPainter *dc, int ecgSeconds, double xScale, double yScale, int x_startpos_devicedots, int y_startpos_devicedots, int whichStrip )
{
    double device_dots_per_mm = (qreal) ( dc->device()->logicalDpiY() / 25.4 ) * Y_SCALE_RATIO;

    int countVisibleChannels = glb_mainwindow->isVisibleChan[0] + glb_mainwindow->isVisibleChan[1] + glb_mainwindow->isVisibleChan[2];
    int countVisibleChannelsDisplayed = 0;

    if ( countVisibleChannels > m_ecgdata->channel_count ) {
        countVisibleChannels = m_ecgdata->channel_count;
    }

    /* for each channel */
    for ( int ch = 0 ; ch < m_ecgdata->channel_count ; ch++ ) {
        if ( glb_mainwindow->isVisibleChan[ch] ) {
            countVisibleChannelsDisplayed++;
            qreal baseline_offset = STRIPHEIGHT_MM * device_dots_per_mm * countVisibleChannelsDisplayed / (countVisibleChannels + 1);
            ShowData( dc, ch, ecgSeconds, xScale, yScale, x_startpos_devicedots, y_startpos_devicedots + baseline_offset * yScale, whichStrip );
        }
    }
}
/* }}} */


/** {{{ void ShowSignal::ShowData( QPainter * dc, int whichChannel, int ecgSeconds, double xScale, double yScale, int x_startpos_devicedots, int y_startpos_devicedots, int whichStrip )
  @brief Show the ECG data
  */
void ShowSignal::ShowData( QPainter *dc, int whichChannel, int ecgSeconds, double xScale, double yScale, int x_startpos_devicedots, int y_startpos_devicedots, int whichStrip )
{
    Q_UNUSED(whichStrip);
    int pen_thickness = 0;
    double range_per_sample = m_ecgdata->range_per_sample;
    long samples_across_grid = m_ecgdata->samps_per_chan_per_sec * ecgSeconds;
    long sample_count = m_ecgdata->samps_per_chan_per_sec * min(ecgSeconds,m_ecgdata->datalen_secs);

    if ( sample_count <= 0 ) {
        return;
    }

    quint16 *chData = m_ecgdata->get( whichChannel, GetPos(), ecgSeconds * m_ecgdata->samps_per_chan_per_sec );

    /** {{{ find DC bias */
    qreal dcBias = 0.0;
    for ( int i = 0 ; i < sample_count ; i++ ) {
        dcBias += (qreal)chData[i];
    }
    dcBias /= sample_count;
    dcBias -= range_per_sample/2;
    /* }}} */

    double device_dots_per_sec = (qreal) ( dc->device()->logicalDpiX() * 2.5 / 2.54 );
    double device_dots_per_mm = (qreal) ( dc->device()->logicalDpiY() / 25.4 );

    qreal mV_per_digital_sample = (qreal) m_ecgdata->device_range_mV / (qreal) range_per_sample;

    /* just display a normally colored line for the data at high speed */

    displayPoints[whichChannel].clear();
    for ( int i = 0 ; i < sample_count ; i++ ) {
        QPointF thisPoint;
        thisPoint.setX( xScale * (qreal) (i * (device_dots_per_sec * ecgSeconds) / samples_across_grid) + (qreal) x_startpos_devicedots );
        thisPoint.setY( yScale * (qreal) ( (range_per_sample/2.0 - (qreal)chData[i]) * (device_dots_per_mm * gain_mm_per_mV * mV_per_digital_sample)) + (qreal) y_startpos_devicedots );
        displayPoints[whichChannel].append( thisPoint );
        // QQQ("dbg.log") << "ShowSignal::ShowData()      chData[" << i << "] = " << chData[i] << "       yields -> " << thisPoint.y();
    }

    QPen pen_solid( QColor("#ff0000"), pen_thickness, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );
    if ( is_printing ) {
        pen_solid = QPen( QColor("black") );
    }

    dc->setPen(pen_solid);
    dc->drawPolyline( displayPoints[whichChannel].constData(), sample_count );


    /* display line depicting ADC Zero (baseline) upon request */
    if ( display_extra == DISPLAY_EXTRA_ADC_ZERO ) {
        dc->setPen( QPen( QColor("blue"), 1, Qt::SolidLine ) );
        dc->drawLine(
                xScale * (qreal) (0 * (device_dots_per_sec * ecgSeconds) / samples_across_grid) + x_startpos_devicedots,
                yScale * (qreal) (0 * (device_dots_per_mm * gain_mm_per_mV * mV_per_digital_sample)) + y_startpos_devicedots,
                xScale * (qreal) (sample_count * (device_dots_per_sec * ecgSeconds) / samples_across_grid) + x_startpos_devicedots,
                yScale * (qreal) (0 * (device_dots_per_mm * gain_mm_per_mV * mV_per_digital_sample)) + y_startpos_devicedots
                );
    }

}
/* }}} */


/** {{{ void ShowSignal::ShowHeader( QPainter * dc, int ecgSeconds, double xScale, double yScale )
  @brief Show the annotations
  */
void ShowSignal::ShowHeader( QPainter * dc, int ecgSeconds, double xScale, double yScale )
{
    QString str;
    QRect textrect;
    int device_dots_per_sec = ROUND2INT( dc->device()->logicalDpiX() * 2.5 / 2.54 );
    int device_dots_per_mm = ROUND2INT( dc->device()->logicalDpiY() / 25.4 );

    dc->setFont( QFont("Helvetica",12) );
    dc->setPen( QPen() );

    size_t pos =  m_ecgdata->file_name.toStdString().find_last_of("//");
    QString filename = m_ecgdata->file_name.mid(pos + 1);

    textrect = dc->boundingRect( 100, 100, 1000, 1000, Qt::AlignVCenter | Qt::AlignLeft, filename );
    dc->drawText(
            0,
			ROUND2INT(yScale * device_dots_per_mm * STRIPHEIGHT_MM ),
            textrect.width(), textrect.height(),
            Qt::AlignVCenter | Qt::AlignRight, filename );

    str = m_ecgdata->viewableDateTime.toString("dd/MM/yyyy hh:mm:ss");

    textrect = dc->boundingRect( 100, 100, 1000, 1000, Qt::AlignVCenter | Qt::AlignRight, str );
    dc->drawText(
            ROUND2INT(xScale * device_dots_per_sec * ecgSeconds) - textrect.width(),
			ROUND2INT(yScale * device_dots_per_mm * STRIPHEIGHT_MM ),
            textrect.width(), textrect.height(),
            Qt::AlignVCenter | Qt::AlignRight, str );

}
/* }}} */


/** {{{ void ShowSignal::ShowAnnotation( QPainter * dc, int ecgSeconds, double xScale, double yScale, int y_startpos, int whichStrip )
  @brief Show the annotations
  */
void ShowSignal::ShowAnnotation( QPainter * dc, int ecgSeconds, double xScale, double yScale, int y_startpos, int whichStrip )
{
    QString str;
    QRect textrect;
    int device_dots_per_sec = ROUND2INT( dc->device()->logicalDpiX() * 2.5 / 2.54 );
    int device_dots_per_mm = ROUND2INT( dc->device()->logicalDpiY() / 25.4 );

    long sample_count = m_ecgdata->samps_per_chan_per_sec * ecgSeconds;

    dc->setFont( QFont("Helvetica",8) );
    dc->setPen( QPen() );

    /* draw the time of the beginning of visible data on the display */
    int second_pos = GetPos() / m_ecgdata->samps_per_chan_per_sec;
    if (whichStrip != 0) {
        second_pos = (m_ecgdata->size() * whichStrip / 4) / m_ecgdata->samps_per_chan_per_sec;
    }

    QTime offsetTimeStart(0,0);
    QTime offsetTimeEnd(0,0);
    offsetTimeStart = offsetTimeStart.addSecs(second_pos);
    offsetTimeEnd = offsetTimeEnd.addSecs(second_pos + ecgSeconds);
    str = m_ecgdata->viewableDateTime.addSecs(second_pos).toString(" hh:mm:ss");

	if ( str.isEmpty() ) {
		str = offsetTimeStart.toString(" hh:mm:ss") + " - " + offsetTimeEnd.toString("hh:mm:ss");
	} else {
		str += "   [offset" + offsetTimeStart.toString(" hh:mm:ss") + " - " + offsetTimeEnd.toString("hh:mm:ss") + "]";
	}

    textrect = dc->boundingRect( 100, 100, 1000, 1000, Qt::AlignVCenter | Qt::AlignLeft, str );
    dc->drawText(
            0,
			ROUND2INT(yScale * device_dots_per_mm * STRIPHEIGHT_MM + y_startpos * device_dots_per_mm - textrect.height()),
            textrect.width(), textrect.height(),
            Qt::AlignVCenter | Qt::AlignRight, str );

    /* draw the current gain on the display. */
    str = QString("%1 mm/sec").arg(25);
    str += "    ";
    str += QString("%1 mm/mV ").arg(gain_mm_per_mV);

    textrect = dc->boundingRect( 100, 100, 1000, 1000, Qt::AlignVCenter | Qt::AlignRight, str );
    dc->drawText(
            ROUND2INT(xScale * device_dots_per_sec * ecgSeconds) - textrect.width(),
            ROUND2INT(yScale * device_dots_per_mm * STRIPHEIGHT_MM + y_startpos * device_dots_per_mm - textrect.height()),
            textrect.width(), textrect.height(),
            Qt::AlignVCenter | Qt::AlignRight, str );


    /* {{{ Use a binary search to find the closest position of a paced beat visible on the screen.  Then draw all the pacer positions visible. */
    textrect = dc->boundingRect( 100, 100, 1000, 1000, Qt::AlignCenter, tr("P") );
    int fontlinehgt = textrect.height();
    int fontlongtextwidth = textrect.width();

    QVector<quint32>::iterator pPacer = qLowerBound( pacerPosition.begin(), pacerPosition.end(), (quint32) GetPos() );
    quint32 endPos = GetPos() + ecgSeconds * m_ecgdata->samps_per_chan_per_sec;

    for ( ; *pPacer < endPos ; pPacer++ ) {
        long xdiff = *pPacer - GetPos();
        if ( (xdiff > 0) && (xdiff < sample_count) ) {
            xdiff = xScale * xdiff * (device_dots_per_sec * ecgSeconds) / sample_count;
            dc->drawText( xdiff - fontlongtextwidth, fontlinehgt * 7/8, fontlongtextwidth * 2, fontlinehgt, Qt::AlignCenter, "P" );
        }
    }
    /* }}} */

    int minutePos = GetPos() / m_ecgdata->samps_per_chan_per_sec / 60;
    if ( paceBeatsPerMinute[minutePos] > 0 ) {
        emit updatePacerText( QString("%1 Paced Beats during minute %2")
                .arg( paceBeatsPerMinute[minutePos] )
                .arg( minutePos )
                );
    } else {
        emit updatePacerText(QString());
    }




	if ( m_beats.size() == 0 ) {
		return;
	}

	QPen penNormal( QColor( "#0000ff" ), 3, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin );

	dc->setPen( penNormal );

	// dc->setFont( QFont("Helvetica",12) );
	textrect = dc->boundingRect( 100, 100, 1000, 1000, Qt::AlignCenter, tr( "UNKNOWN" ) );
	fontlinehgt = textrect.height();
	fontlongtextwidth = textrect.width();

	QPointF lastPtVariance;

	for ( int b = first_beat_showing() ; b < m_beats.size() ; b++ ) {
		long xdiff = m_beats[b].pos_samps - GetPos();
		if ( xdiff >= sample_count ) {
			break;
		}
		if ( ( xdiff > 0 ) && ( xdiff < sample_count ) ) {

			xdiff = xScale * xdiff * ( device_dots_per_sec * ecgSeconds ) / sample_count;

			/** draw beat classification */

			/* draw the beat type */
			{
				dc->save();
				dc->setPen( penNormal );
				if ( m_beats[b].type == NOISE ) {
					dc->setPen( QPen( QColor( "darkred" ) ) );
				}
				int yPos = fontlinehgt * 1/8;
				if ( m_beats[b].type == RHYTHM ) {
					dc->setFont( QFont("Helvetica",6) );
					yPos += fontlinehgt / 2;
				}
                dc->drawText( xdiff - fontlongtextwidth, yPos, fontlongtextwidth * 2, fontlinehgt, Qt::AlignCenter, beat_classification_name(m_beats[b]));
				dc->restore();
			}

			BeatInfo *lastbeat = previous_beat( b );

			/* draw HR */
			if ( m_beats[b].annotationString.isEmpty() && ( m_beats[b].type != PACE ) /* && (m_beats[b].type != NOISE) */ && ( lastbeat != NULL ) ) {
				QString strHR = QString::number( ROUND2INT( 60.0 * m_ecgdata->samps_per_chan_per_sec / ( ( m_beats[b].pos_samps - lastbeat->pos_samps ) ) ) );
				if ( (unsigned int) strHR.toInt() <= 300 ) {
					dc->drawText( xdiff - fontlongtextwidth, fontlinehgt * 9 / 8, fontlongtextwidth * 2, fontlinehgt, Qt::AlignCenter, strHR );
				}

			}
			if ( ! m_beats[b].annotationString.isEmpty() && ( lastbeat != NULL ) ) {
				dc->save();
				dc->setPen( penNormal );
				if ( m_beats[b].type == NOISE ) {
					dc->setPen( QPen( QColor( "darkred" ) ) );
				}
				dc->drawText( xdiff - fontlongtextwidth, fontlinehgt * 17 / 8, fontlongtextwidth * 2, fontlinehgt, Qt::AlignCenter, m_beats[b].annotationString );
				dc->restore();
			}
		}
	}

}
/* }}} */







/** {{{ long middle_beat_showing()
  @brief Middle beat showing
  */
long ShowSignal::middle_beat_showing()
{
	if ( cached_middle_beat_found < 0 ) {
		if ( m_beats.isEmpty() ) {
			return 0;
		}

		long sample_count = m_ecgdata->samps_per_chan_per_sec * ECG_DISPLAY_WINDOW_SIZE_SECONDS;

		/* linear search for the first beat showing on the screen */
		cached_middle_beat_found = findBeatNearPosition( GetPos() + sample_count / 2, SelectiveDirectionCanBeHigher );
	}
	return cached_middle_beat_found;
}
/* }}} */


/** {{{ long first_beat_showing()
  @brief First beat showing
  */
long ShowSignal::first_beat_showing()
{
	if ( middle_beat_showing() < 0 ) {
		return 0;
	} else {
		if ( m_beats.isEmpty() ) {
			return 0;
		}

		/* linear backward search for the first beat showing */
		first_beat_found = 0;
		for ( int b = middle_beat_showing() ; b > 0 ; b-- ) {
			if ( m_beats[b].pos_samps < GetPos() ) {
				first_beat_found = b + 1;	/* this beat is no longer on the display, so use the beat just after this one */
				break;
			}
		}
		return first_beat_found;
	}
}
/* }}} */


/** {{{ BeatInfo *ShowSignal::previous_beat( int beatIndex )
 */
BeatInfo *ShowSignal::previous_beat( int beatIndex )
{
	BeatInfo *retval = NULL;

	while ( --beatIndex >= 0 ) {
		if ( (m_beats[beatIndex].type != PACE) && (m_beats[beatIndex].type != RHYTHM) ) {
			retval = &( m_beats[beatIndex] );
			break;
		}
	}
	return retval;
}
/* }}} */


/** {{{ int ShowSignal::next_beat_of_a_type( int beatIndex, int beatType )
 */
int ShowSignal::next_beat_of_a_type( int beatIndex, int beatType )
{
	int retval = beatIndex;

	for ( int b = beatIndex + 1 ; b < m_beats.size() ; b++ ) {
		if ( m_beats[b].type == beatType ) {
			retval = b;
			break;
		}
	}
	return retval;
}
/* }}} */


/** {{{ int ShowSignal::prev_beat_of_a_type( int beatIndex, int beatType )
 */
int ShowSignal::prev_beat_of_a_type( int beatIndex, int beatType )
{
	int retval = -1;

	while ( --beatIndex >= 0 ) {
		if ( m_beats[beatIndex].type == beatType ) {
			retval = beatIndex;
			break;
		}
	}

	return retval;
}
/* }}} */


/** {{{ int ShowSignal::next_beat_of_AFRelated( int beatIndex )
 */
int ShowSignal::next_beat_of_AFRelated( int beatIndex )
{
	int retval = beatIndex;

	for ( int b = beatIndex + 1 ; b < m_beats.size() ; b++ ) {
		switch ( m_beats[b].type ) {
			case RHYTHM:
			case 'A':
			case 'a':
				retval = b;
				return retval;
				break;

			case NORMAL:
			default:
				break;
		}
	}
	return retval;
}
/* }}} */


/** {{{ int ShowSignal::prev_beat_of_AFRelated( int beatIndex )
 */
int ShowSignal::prev_beat_of_AFRelated( int beatIndex )
{
	int retval = 0;

	while ( --beatIndex >= 0 ) {
		switch ( m_beats[beatIndex].type ) {
			case RHYTHM:
			case 'A':
			case 'a':
				retval = beatIndex;
				return retval;
				break;

			case NORMAL:
			default:
				break;
		}
	}
	return retval;
}
/* }}} */



/** {{{ int ShowSignal::findBeatNearPosition( int samplePos, int direction = SelectiveDirectionEitherPart );
 * @brief Binary search for a beat.
 * @param samplePos Find closes beat to this sample position.
 * @param direction
 *
 * @return index position of the closest beat, or -1 if there are no beats.
 */
int ShowSignal::findBeatNearPosition( int samplePos, int direction )
{
	if ( m_beats.count() == 0 ) {
		return -1;
	}

	// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2").arg(samplePos).arg(m_beats.size());

	int imin = 0;
	int beatCount = m_beats.count();
	int imax = beatCount - 1;

	while ( imax > imin ) {
		int imid = ( imin + imax ) / 2;
		int beatVal = m_beats.at( imid ).pos_samps;

		if ( samplePos < beatVal ) {
			imax = imid - 1;
			if ( imax < imin ) {
				imax = imin;
			}
		} else if ( samplePos > beatVal ) {
			imin = imid + 1;
			if ( imin > imax ) {
				imin = imax;
			}
		} else {
			// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    imid = %3").arg(samplePos).arg(m_beats.size()).arg(imid);
			return imid;
		}
	}
	// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    imin = %3  imax = %4   pos_samps = %5") .arg(samplePos) .arg(m_beats.size()) .arg(imin) .arg(imax) .arg(m_beats.at(imin).pos_samps) ;

	if ( m_beats.at( imin ).pos_samps == samplePos ) {
		// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    imin = %3").arg(samplePos).arg(m_beats.size()).arg(imin);
		return imin;
	}

	if ( direction == SelectiveDirectionCanBeHigher ) {
		if ( m_beats.at( imin ).pos_samps < samplePos ) {
			if ( imin + 1 < beatCount ) {
				// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    imin+1 = %3").arg(samplePos).arg(m_beats.size()).arg(imin+1);
				return imin + 1;
			} else {
				// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    -1 A").arg(samplePos).arg(m_beats.size());
				return -1;
			}
		}
	} else if ( direction == SelectiveDirectionCanBeLower ) {
		if ( m_beats.at( imin ).pos_samps > samplePos ) {
			if ( imin > 0 ) {
				// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    imin-1 = %3").arg(samplePos).arg(m_beats.size()).arg(imin-1);
				return imin - 1;
			} else {
				// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    -1 B").arg(samplePos).arg(m_beats.size());
				return -1;
			}
		}
	} else if ( direction == SelectiveDirectionClosest ) {

		int prevSampleIndex = imin;
		int nextSampleIndex;

		if ( m_beats.at( imin ).pos_samps > samplePos ) {
			if ( imin > 0 ) {
				prevSampleIndex = imin - 1;
			} else {
				// qDebug() << QString("findBeatNearPosition(arg(samplePos).arg(m_beats.size()).arg(imin);
				return imin;
			}
		}

		nextSampleIndex = prevSampleIndex + 1;

		if ( nextSampleIndex >= m_beats.count() ) {
			// qDebug() << QString("findBeatNearPosition(arg(samplePos).arg(m_beats.size()).arg(prevSampleIndex);
			return prevSampleIndex;
		}

		int prevSample = m_beats.at( prevSampleIndex ).pos_samps;
		int nextSample = m_beats.at( nextSampleIndex ).pos_samps;

		if ( ( samplePos - prevSample ) < ( nextSample - samplePos ) ) {
			// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    prevSampleIndex A = %3").arg(samplePos).arg(m_beats.size()).arg(prevSampleIndex);
			return prevSampleIndex;
		} else {
			// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    nextSampleIndex = %3").arg(samplePos).arg(m_beats.size()).arg(nextSampleIndex);
			return nextSampleIndex;
		}
	}

	// qDebug() << QString("findBeatNearPosition(%1,direction)   out of m_beats.size() = %2    imin B = %3").arg(samplePos).arg(m_beats.size()).arg(imin);
	return imin;
}
/* }}} */



/* {{{ QString beat_classification_name(BeatInfo beat)
 */
QString ShowSignal::beat_classification_name(BeatInfo beat)
{
    switch (beat.type) {

		case NOTQRS:		/* not-QRS (not a getann/putann code) */
			return QObject::tr( " " );
		case NORMAL:		/* normal beat */
			return QObject::tr( "N" );
		case LBBB:		/* left bundle branch block beat */
			return QObject::tr( "LBBB" );
		case RBBB:		/* right bundle branch block beat */
			return QObject::tr( "RBBB" );
		case ABERR:		/* aberrated atrial premature beat */
			return QObject::tr( "ABERR" );
		case PVC:		/* premature ventricular contraction */
			return QObject::tr( "V" );
		case FUSION:		/* fusion of ventricular and normal beat */
			return QObject::tr( "F" );
		case NPC:		/* nodal (junctional) premature beat */
			return QObject::tr( "NPC" );
		case APC:		/* atrial premature contraction */
			return QObject::tr( "APC" );
		case SVPB:		/* premature or ectopic supraventricular beat */
			return QObject::tr( "S" );
		case VESC:		/* ventricular escape beat */
			return QObject::tr( "VESC" );
		case NESC:		/* nodal (junctional) escape beat */
			return QObject::tr( "NESC" );
		case PACE:		/* paced beat */
			return QObject::tr( "|" );
		case UNKNOWN:		/* unclassifiable beat */
			return QObject::tr( "?" );
		case NOISE:		/* signal quality change */
            if (beat.subtype) {
                return QObject::tr("!(");
            }
            else {
                return QObject::tr(")!");
            }
        case ARFCT:		/* isolated QRS-like artifact */
			return QObject::tr( "ARFCT" );
		case STCH:		/* ST change */
			return QObject::tr( "STCH" );
		case TCH:		/* T-wave change */
			return QObject::tr( "TCH" );
		case SYSTOLE:		/* systole */
			return QObject::tr( "SYSTOLE" );
		case DIASTOLE:		/* diastole */
			return QObject::tr( "DIASTOLE" );
		case NOTE:		/* comment annotation */
			return QObject::tr( "NOTE" );
		case MEASURE:		/* measurement annotation */
			return QObject::tr( "MEASURE" );
		case PWAVE:		/* P-wave peak */
			return QObject::tr( "PWAVE" );
		case BBB:		/* left or right bundle branch block */
			return QObject::tr( "BBB" );
		case PACESP:		/* non-conducted pacer spike */
			return QObject::tr( "PACESP" );
		case TWAVE:		/* T-wave peak */
			return QObject::tr( "TWAVE" );
		case RHYTHM:		/* rhythm change */
			return QObject::tr( "RHYTHM" );
		case UWAVE:		/* U-wave peak */
			return QObject::tr( "UWAVE" );
		case LEARN:		/* learning */
			return QObject::tr( "LEARN" );
		case FLWAV:		/* ventricular flutter wave */
			return QObject::tr( "FLWAV" );
		case VFON:		/* start of ventricular flutter/fibrillation */
			return QObject::tr( "VFON" );
		case VFOFF:		/* end of ventricular flutter/fibrillation */
			return QObject::tr( "VFOFF" );
		case AESC:		/* atrial escape beat */
			return QObject::tr( "AESC" );
		case SVESC:		/* supraventricular escape beat */
			return QObject::tr( "SVESC" );
		case LINK:		/* link to external data (aux contains URL) */
			return QObject::tr( "LINK" );
		case NAPC:		/* non-conducted P-wave (blocked APB) */
			return QObject::tr( "NAPC" );
		case PFUS:		/* fusion of paced and normal beat */
			return QObject::tr( "PFUS" );
			//case WFON:		/* waveform onset */
		case PQ:		/* PQ junction (beginning of QRS) */
			return QObject::tr( "PQ" );
			return QObject::tr( "WFON" );
			//case JPT:		/* J point (end of QRS) */
		case WFOFF:		/* waveform end */
			return QObject::tr( "JPT" );
			return QObject::tr( "WFOFF" );
		case RONT:		/* R-on-T premature ventricular contraction */
			return QObject::tr( "RONT" );

    case 42:    // change in signal connection
        return QString(tr("LOFFx%1")).arg((int)beat.subtype, 0, 16);     // print hex
    case 43:
        return tr("Ev");
    case 44:
        switch (beat.subtype) {
        case 'a':
            return tr("AF(");
        case 'b':
            return tr("B(");
        case 'p':
            return tr("P(");
        case 't':
            return tr("T(");
        default:
            return QString("[%1](").arg(beat.subtype);
        }
    case 45:
        switch (beat.subtype) {
        case 'a':
            return tr(")AF");
        case 'b':
            return tr(")B");
        case 'p':
            return tr(")P");
        case 't':
            return tr(")T");
        default:
            return QString(")[%1]").arg(beat.subtype);
        }

		default:
            return QString("[%1]").arg( beat.type );
	}
}
/* }}} */




/** {{{ void ShowSignal::SetPos( long start_time_secs )
  @brief Select where this ECG data starts
  */
long ShowSignal::SetPos( long start_time_samps )
{
    if ( start_time_samps < 0 ) {
        start_time_samps = 0;
    }
    if ( start_time_samps >= ((m_ecgdata->datalen_secs - ECG_DISPLAY_WINDOW_SIZE_SECONDS) * m_ecgdata->samps_per_chan_per_sec) ) {
        start_time_samps = ((m_ecgdata->datalen_secs - ECG_DISPLAY_WINDOW_SIZE_SECONDS) * m_ecgdata->samps_per_chan_per_sec) - 1;
    }
    if ( curpos_samples != start_time_samps ) {
		first_beat_found = -1;
		cached_middle_beat_found = -1;
        curpos_samples = start_time_samps;
#ifdef QT_DEBUG
        QString str = QString( tr("Current ECG Time: %1:%2:%3"))
            .arg( (int) (((curpos_samples + (ECG_DISPLAY_WINDOW_SIZE_SECONDS * m_ecgdata->samps_per_chan_per_sec)/2) / m_ecgdata->samps_per_chan_per_sec) / (60*60)) )
            .arg( (int) (((curpos_samples + (ECG_DISPLAY_WINDOW_SIZE_SECONDS * m_ecgdata->samps_per_chan_per_sec)/2) / m_ecgdata->samps_per_chan_per_sec) / (60)) % 60, 2, 10, QLatin1Char('0') )
            .arg( (int) (((curpos_samples + (ECG_DISPLAY_WINDOW_SIZE_SECONDS * m_ecgdata->samps_per_chan_per_sec)/2) / m_ecgdata->samps_per_chan_per_sec) / 1) % 60, 2, 10, QLatin1Char('0') );
        // qDebug( str.toLatin1() );
#endif
    }
    return curpos_samples;
}
/* }}} */




/** {{{ int ShowSignal::findClosestDataPointToMousePos( QPoint mousePt )
 */
int ShowSignal::findClosestDataPointToMousePos( QPoint mousePt )
{
    float distanceClosest = 1e10;
    int samplePosClosest = 0;

    for ( int ch = 0 ; ch < m_ecgdata->channel_count ; ch++ ) {
        for ( int i = 0 ; i < displayPoints[ch].size() ; i++ ) {
            float thisDistance = ( pow(displayPoints[ch].value(i).x() - mousePt.x(),2) + pow(displayPoints[ch].value(i).y() - mousePt.y(),2) );
            if ( thisDistance < distanceClosest ) {
                // qDebug() << i << "  FOUND sample = " << i << "   when comparing  mouse = " << mousePt << "     to   pt = " << displayPoints[0].value(i) << "   with distance = " << thisDistance;
                distanceClosest = thisDistance;
                samplePosClosest = i;
            }
        }
    }

    return ROUND2INT( (float) samplePosClosest * (float) QApplication::desktop()->physicalDpiX() / m_ecgdata->samps_per_chan_per_sec );
}
/* }}} */



/** {{{ void ShowSignal::smooth_advance()
    @brief Smooth advance of the ECG data
*/
void ShowSignal::smooth_advance()
{
    if ( m_testspeed ) {
        SetPos( GetPos() + SCROLL_CHUNK );
        update();
        // QTimer::singleShot( 0, this, SLOT(smooth_advance()) );
    } else {
        SetPos( GetPos() + SCROLL_CHUNK );
        update();
    }
    if ( GetPos() > (long) m_ecgdata->size() ) {
        SetPos( m_ecgdata->size() - m_ecgdata->sample_count() );
    }
}
/* }}} */


/** {{{ void ShowSignal::print()
    @brief Print the ECG data
*/
void ShowSignal::print()
{
#ifdef QT_DEBUG
    qDebug("ShowSignal::print()");
#endif

#ifndef QT_NO_PRINTER

    QPrinter printer(QPrinter::HighResolution);

    printer.setPageMargins( 5, 10, 5, 10, QPrinter::Millimeter );
    QPrintPreviewDialog printPrevDialog(&printer, this);
    connect( &printPrevDialog, SIGNAL(paintRequested(QPrinter *)), this, SLOT(printRender(QPrinter *)) );
    printPrevDialog.exec();

#endif // QT_NO_PRINTER

}
/* }}} */



/** {{{ void ShowSignal::print_strip()
    @brief Print the ECG data
*/
void ShowSignal::print_strip()
{
#ifdef QT_DEBUG
    qDebug("ShowSignal::print()");
#endif

#ifndef QT_NO_PRINTER

    QPrinter printer(QPrinter::HighResolution);

    printer.setPaperSize(QPrinter::Letter);
    printer.setPageMargins( 5, 10, 5, 10, QPrinter::Millimeter );
    QPrintPreviewDialog printPrevDialog(&printer, this);
    connect( &printPrevDialog, SIGNAL(paintRequested(QPrinter *)), this, SLOT(printRenderStrip(QPrinter *)) );
    printPrevDialog.exec();

#endif // QT_NO_PRINTER

}
/* }}} */



/** {{{ void ShowSignal::printRender( QPrinter * printer )
*/
void ShowSignal::printRender( QPrinter *printer )
{
    qDebug() << "ShowSignal::printRender()";
    QPainter painter(printer);
    painter.setRenderHint( QPainter::HighQualityAntialiasing );

    is_printing = true;
    switch ( getComboViewTypeIndex() ) {
        case VIEWTYPE_FULL_DISCLOSURE:
            RenderFullDisclosure( &painter, printer );
            break;
        case VIEWTYPE_8_SECOND_STRIP:
            Render( &painter, printer );
            break;
    }
    is_printing = false;
}
/* }}} */


/** {{{ void ShowSignal::printRenderStrip( QPrinter * printer )
*/
void ShowSignal::printRenderStrip( QPrinter *printer )
{
    qDebug() << "ShowSignal::printRender()";
    QPainter painter(printer);
    painter.setRenderHint( QPainter::HighQualityAntialiasing );

    is_printing = true;
    RenderStrip( &painter, printer );
    is_printing = false;
}
/* }}} */


/** {{{ void ShowSignal::printPDF()
    @brief Print ECG data on PDF document
*/
void ShowSignal::printPDF()
{
#ifdef QT_DEBUG
    qDebug("ShowSignal::printPDF()");
#endif

#ifndef QT_NO_PRINTER
    QString fileNameTemplate = windowTitle();
    fileNameTemplate.truncate( fileNameTemplate.lastIndexOf(".") );
    QString fileName = QString("%1.pdf").arg(fileNameTemplate);
    if ( QFile::exists(fileName) ) {
        for ( int filenum = 0 ; filenum < 10000 ; filenum++ ) {
            fileName = QString("%1-%2.pdf").arg(fileNameTemplate).arg(filenum,4,10,QChar('0'));
            if ( ! QFile::exists(fileName) ) {
                break;
            }
        }
    }

    fileName = QFileDialog::getSaveFileName( this, tr("Save PDF File"), fileName, tr("PDF (*.pdf);;All Files (*.*)"));

    if ( fileName.isEmpty() ) {
        qDebug() << "Cancel PDF export...";
        return;
    } else {
        qDebug() << "PDF export using filename:" << fileName;
    }

    QPrinter printer(QPrinter::HighResolution);
    printer.setPageMargins( 5, 10, 5, 10, QPrinter::Millimeter );

    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName(fileName);
    printer.newPage();

    QPainter painter(&printer);

    is_printing = true;
    switch ( getComboViewTypeIndex() ) {
        case VIEWTYPE_FULL_DISCLOSURE:
            RenderFullDisclosure( &painter, &printer );
            break;
        case VIEWTYPE_8_SECOND_STRIP:
            Render( &painter, &printer );
            break;
    }
    is_printing = false;

#endif // QT_NO_PRINTER

}
/* }}} */


/** {{{ void ShowSignal::moveLeft()
    @brief Move the ECG data to the left
*/
void ShowSignal::moveLeft()
{
    SetPos( GetPos() - 1 * m_ecgdata->samps_per_chan_per_sec );
    update();
}
/* }}} */


/** {{{ void ShowSignal::moveRight()
    @brief Move the ECG data to the right
*/
void ShowSignal::moveRight()
{
    SetPos( GetPos() + 1 * m_ecgdata->samps_per_chan_per_sec );
    update();
}
/* }}} */



/** {{{ void ShowSignal::cancel_data_saving()
    @brief Cancel the loading of the ecg data
*/
void ShowSignal::cancel_data_saving()
{
    qDebug() << "ShowSignal::cancel_data_saving()";
    is_saving_data = false;
    emit loading_finished();
}
/* }}} */


/** {{{ void ShowSignal::saveAs()
    @brief Save the file
*/
bool ShowSignal::saveAs()
{
    QString fileName = QFileDialog::getSaveFileName( this, tr("Save format file as") );

    if ( fileName.isEmpty() ) {
        return false;
    }

    return save( fileName );
}
/* }}} */


/** {{{ bool ShowSignal::save( QString fileName )
 */
bool ShowSignal::save( QString fileName )
{
    Q_UNUSED(fileName);
    return true;
}
/* }}} */

