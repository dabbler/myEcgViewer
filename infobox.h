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
