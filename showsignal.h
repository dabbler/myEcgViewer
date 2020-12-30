#ifndef SHOWSIGNAL_H
#define SHOWSIGNAL_H

/**
 * @file showsignal.h
*/ 


#include <QtWidgets>
#include <QWidget>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QtPrintSupport/QPrintPreviewDialog>
#include "beatinfo.h"
#include "ecgdata.h"

// #include "mainwindow.h"	// DEBUG: just used for isVisibleChan[] for now

#ifndef GL_MULTISAMPLE
#define GL_MULTISAMPLE  0x809D
#endif

#define SelectiveDirectionEitherPart	(0)
#define SelectiveDirectionCanBeLower	(1)
#define SelectiveDirectionCanBeHigher	(2)
#define SelectiveDirectionClosest		(3)

#define MODE_BEAT_NONE				( 0)
#define MODE_BEAT_NORMAL			( 1)
#define MODE_BEAT_IDENTIFICATION	( 2)
#define MODE_BEAT_DELETION			( 3)

#define VIEWTYPE_NONE				( 0)
#define VIEWTYPE_8_SECOND_STRIP		( 1)
#define VIEWTYPE_FULL_DISCLOSURE	( 2)

#define DISPLAY_EXTRA_NONE				( 0)
#define DISPLAY_EXTRA_ADC_ZERO			( 1)
#define DISPLAY_EXTRA_GRID_DOTS			( 2)
#define COUNT_OF_DISPLAY_EXTRA			(DISPLAY_EXTRA_GRID_DOTS - DISPLAY_EXTRA_NONE + 1)

#define VIEWWIDTH		(draw_width)
#define VIEWHEIGHT		(draw_height)

#define STRIPHEIGHT_MM	(80)

#define PAGE_MARGIN_TOP	(0)
#define PAGE_MARGIN_BETWEEN	(3)

#define MM2DEVDOTS(dc,x)	((x) * ((float)(dc)->device()->logicalDpiY() / 25.4))

#define SCROLL_CHUNK	(4)
#define ECG_DISPLAY_WINDOW_SIZE_SECONDS		(8)

#define min(a,b)	( (a) < (b) ? (a) : (b) )

// #define Y_SCALE_RATIO (0.8)
#define Y_SCALE_RATIO (1.0)


/* {{{ class ShowSignal
   @brief	Displays an ECG signal on the screen
*/
class ShowSignal : public QWidget
{
    Q_OBJECT

public:
	ShowSignal( QWidget *parent = 0, EcgData *theEcgData = 0 );
	~ShowSignal();

    QSize minimumSizeHint() const;
    QSize sizeHint() const;

    void setComboViewTypeAction( QAction *act ) { comboViewTypeAction = act; };
    void showComboViewType() { if ( comboViewTypeAction ) { comboViewTypeAction->setVisible(true); } };
    void hideComboViewType() { if ( comboViewTypeAction ) { comboViewTypeAction->setVisible(false); } };

    void setComboViewType( int theIndex ) { if ( comboViewType ) { comboViewType->setCurrentIndex(theIndex); } };
    int getComboViewTypeIndex() { if ( comboViewType ) { return comboViewType->currentIndex(); }; return -1; };
    QComboBox *getComboViewTypeWidget() { return comboViewType; };

	int load_annotation_file( char *recordName, char *ext );

protected:
	void focusInEvent( QFocusEvent *event );
	void focusOutEvent( QFocusEvent *event );
	void showEvent( QShowEvent * event );
	void hideEvent( QHideEvent * event );
    void paintEvent( QPaintEvent *event );
	void resizeEvent( QResizeEvent * event );


    QComboBox *comboViewType;
	QAction *comboViewTypeAction;

	float zoom_amount;
	float m_zoom_x;
	float m_zoom_y;

	QVector<quint32> pacerPosition;
	QHash<int,int> paceBeatsPerMinute;

	QList<BeatInfo> m_beats;

public slots:
	void mousePressEvent( QMouseEvent *event );
	void mouseMoveEvent( QMouseEvent *event );
	void wheelEvent( QWheelEvent *e );
	void keyPressEvent( QKeyEvent * event );
	void setViewType( int newViewType );
	void printRender( QPrinter * printer );
    void printRenderStrip( QPrinter * printer );

	void smooth_advance();

	void store_pacer_position( long samplePos );

    void newFile();
    /** @brief Save the file */
	bool save( QString fileName = "" );
    bool saveAs();
    /** @brief Save the file with given filename*/
    bool saveFile(const QString &fileName) { Q_UNUSED(fileName);return true; }
    void print();
    void print_strip();
    void printPDF();

	void cancel_data_saving();

    void moveLeft();
    void moveRight();
	
signals:
	int focusChanged();

	void load_size( int filesize );
	void data_loaded_so_far( int loaded );
	void loading_finished();

	void updatePacerText( QString txt );

public:
	EcgData *m_ecgdata;
	qreal gain_mm_per_mV;

    QString currentFile() { return curFile; }
    QString userFriendlyCurrentFile() { return strippedName(curFile); }
	QString strippedName(const QString &fullFileName) { return QFileInfo(fullFileName).fileName(); }

	void cut() { return; }
	void copy() { return; }
	void paste() { return; }


private:

	QVector<QPointF> displayPoints[CHANNEL_MAX];

    QString curFile;	// used for MDI
    bool isUntitled;	// used for MDI

	bool is_printing;
	bool is_saving_data;
	int display_extra;

	uint object;
    QPoint lastPos;
    int xRot;
    int yRot;
    int zRot;
	int yOffsetDragged;

protected:
	void Render( QPainter * dc, QPrinter *printer = NULL );
    void RenderStrip( QPainter * dc, QPrinter *printer = NULL );
    void RenderFullDisclosure( QPainter * dc, QPrinter *printer = NULL );
    void ShowGrid( QPainter *dc, int cols, int height_mm, int ecgSeconds = ECG_DISPLAY_WINDOW_SIZE_SECONDS, double xScale = 1.0, double yScale = 1.0, int y_startpos = 0  );
    void ShowData( QPainter *dc, int ecgSeconds = ECG_DISPLAY_WINDOW_SIZE_SECONDS, double xScale = 1.0, double yScale = 1.0, int x_startpos_devicedots = 0, int y_startpos_devicedots = 0, int whichStrip = 0 );
    void ShowData( QPainter *dc, int whichChannel, int ecgSeconds, double xScale, double yScale, int x_startpos_devicedots, int y_startpos_devicedots, int whichStrip = 0 );
    void ShowHeader( QPainter * dc, int ecgSeconds = ECG_DISPLAY_WINDOW_SIZE_SECONDS, double xScale = 1.0, double yScale = 1.0 );
    void ShowAnnotation( QPainter *dc, int ecgSeconds = ECG_DISPLAY_WINDOW_SIZE_SECONDS, double xScale = 1.0, double yScale = 1.0, int y_startpos = 0, int whichStrip = 0 );
	int findClosestDataPointToMousePos( QPoint mousePt );
    QString beat_classification_name(BeatInfo beat);

	BeatInfo *previous_beat( int beatIndex );
	int next_beat_of_a_type( int beatIndex, int beatType );
	int prev_beat_of_a_type( int beatIndex, int beatType );
	int next_beat_of_AFRelated( int beatIndex );
	int prev_beat_of_AFRelated( int beatIndex );
	long first_beat_showing();
	long middle_beat_showing();
	int findBeatNearPosition( int samplePos, int direction );

	long SetPos( long start_time_samps );
	long GetPos() { return curpos_samples; };

private:
	int draw_width, draw_height;
	int draw_xpos, draw_ypos;

	double xScaling, yScaling;

public:

	long curpos_samples;

	long first_beat_found;
	long cached_middle_beat_found;

    QTimer		*m_timer;
	long	m_time_ends;

	bool	m_testspeed;
	bool	m_test_antialiasing;

};
/* }}} */


#endif // SHOWSIGNAL_H
