

#include <QtGui>

#include "infobox.h"
#include "ui_infobox.h"




/** {{{ InfoBox::InfoBox(QWidget * parent) :
	QWidget(parent, Qt::FramelessWindowHint | Qt::WindowSystemMenuHint),
	ui(new Ui::InfoBox)
 */
InfoBox::InfoBox( QWidget *parent, QString msg, int timeOut ) :
	QWidget( parent, Qt::Dialog | Qt::WindowStaysOnTopHint | Qt::CustomizeWindowHint ), pixmap(NULL),
	ui(new Ui::InfoBox)
{
	ui->setupUi(this);

	setWindowOpacity(0.80);
	if ( parent ) {
		move( parent->pos() + parent->rect().center() - QPoint(size().width()/2, size().height()/2) );
	}
	show();

	tim = new QTimer(this);
	connect(tim, SIGNAL(timeout()), this, SLOT(close()));
	tim->start(timeOut);

	ui->label->setText(msg);

	QCoreApplication::processEvents();	/* give the label an opportunity to populate the text */
}
/* }}} */



/** {{{ InfoBox::~InfoBox()
 */
InfoBox::~InfoBox()
{
	hide();
	delete tim;
	delete ui;
}
/* }}} */


/** {{{ void InfoBox::changeEvent(QEvent * e)
 */
void InfoBox::changeEvent(QEvent * e)
{
	QWidget::changeEvent(e);
	switch ( e->type() ) {
		case QEvent::LanguageChange:
			ui->retranslateUi(this);
			break;
		default:
			break;
	}
}
/* }}} */


/** {{{ void InfoBox::addWidgetBelow( QWidget *widge )
 */
void InfoBox::addWidgetBelow( QWidget *widge )
{
	ui->verticalLayout->addWidget( widge );
	QSize sz = size();
	resize( QSize( sz.width(), sz.height() + 20 ) );
}
/* }}} */


/** {{{ void InfoBox::addWidgetHeight( int hgtAddOn )
 */
void InfoBox::addWidgetHeight( int hgtAddOn )
{
	QSize sz = size();
	resize( QSize( sz.width(), sz.height() + hgtAddOn ) );
}
/* }}} */


/** {{{ QString InfoBox::getBackgroundImage()
 */
QString InfoBox::getBackgroundImage()
{
	return "images/background.jpg";
}
/* }}} */


/** {{{ InfoBox::mouseDoubleClickEvent( QMouseEvent * e )
 */
void InfoBox::mouseDoubleClickEvent( QMouseEvent * e )
{
    if ( e->button() == Qt::LeftButton ) {
		close();
    }
}
/* }}} */


/* {{{ transparent bitmapped background functions */

/** {{{ void InfoBox::paintEvent(QPaintEvent *event)
*/
void InfoBox::paintEvent(QPaintEvent * event)
{
	Q_UNUSED(event);

	if ( ! pixmap ) {
		pixmap = new QPixmap;
		pixmap->load(getBackgroundImage());
	}

	if ( pixmap && ! pixmap->isNull() ) {
		*pixmap = pixmap->scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		setMask(pixmap->mask());
		QPainter painter(this);
		painter.drawPixmap(0, 0, *pixmap);
	}
}

/* }}} */


/** {{{ void InfoBox::resizeEvent(QResizeEvent * event )
*/
void InfoBox::resizeEvent(QResizeEvent * /* event */ )
{
	if ( pixmap && ! pixmap->isNull() ) {
		*pixmap = pixmap->scaled(width(), height(), Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
		setMask(pixmap->mask());
	}
	setWindowOpacity(0.80);
}

/* }}} */


/** {{{ void InfoBox::mousePressEvent(QMouseEvent *event)
*/
void InfoBox::mousePressEvent(QMouseEvent * event)
{
	isDragging = false;
	switch ( event->button() ) {
		case Qt::LeftButton:
			break;
		case Qt::RightButton:
			dragPosition = event->globalPos() - frameGeometry().topLeft();
			isDragging = true;
			event->accept();
			break;
		default:
			break;
	}
}

/* }}} */


/** {{{ void InfoBox::mouseMoveEvent(QMouseEvent *event)
*/
void InfoBox::mouseMoveEvent(QMouseEvent * event)
{
	if ( event->buttons() & Qt::RightButton ) {
		/* only move the widget is grabbing the top half of the widget */
		if ( isDragging ) {
			move(event->globalPos() - dragPosition);
			event->accept();
		}
	}
}

/* }}} */

/* }}} */



