
#ifndef INFOBOX_H
#define INFOBOX_H

#include <QtWidgets>


namespace Ui {
	class InfoBox;
}

class InfoBox : public QWidget
{
	Q_OBJECT

public:
	InfoBox( QWidget *parent = NULL, QString msg = "", int timeOut = 3000 );
	~InfoBox();

	void addWidgetBelow( QWidget *widge );
	void addWidgetHeight( int hgtAddOn );

protected:
	void changeEvent(QEvent * e);
	void paintEvent(QPaintEvent * event);
	void resizeEvent(QResizeEvent * /* event */ );
	void mouseMoveEvent(QMouseEvent * event);
	void mousePressEvent(QMouseEvent * event);
	void mouseDoubleClickEvent( QMouseEvent * e );
	QString getBackgroundImage();

private:
	QPixmap * pixmap;
	QPoint dragPosition;
	bool isDragging;

	QTimer *tim;

private:
	Ui::InfoBox * ui;
};

#endif							// INFOBOX_H
