/**
 * @file SironaHolterViewer.cpp
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

#include "myheader.h"
#include "mainwindow.h"
#include "version.h"

char *glbExeFileName;

extern MainWindow *glb_mainwindow;

extern void show_splash();

/** {{{ int main(int argc, char *argv[])
    @brief Start the application
*/
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    glbExeFileName = argv[0];

    glb_mainwindow = new MainWindow();

    int autoExit = false;

    for ( int a = 1 ; a < argc ; a++ ) {
        if ( QString(argv[a]).startsWith("-") ) {
            if ( QString(argv[a]) == "-x" ) {
                autoExit = true;
            }
        } else {
            glb_mainwindow->openEcgFile( argv[a] );
        }
    }

    if ( autoExit ) {
        return 0;
    } else {
        glb_mainwindow->show();
        show_splash();
        return app.exec();
    }
}
/* }}} */


/** {{{ void show_splash()
    @brief Show initialization image
*/
void show_splash()
{
    QPixmap pixmap(":/out/images/splash.png");
    QPainter p(&pixmap);
    p.setPen( QPen(Qt::white, 1) );
    QFont myfont = QFont("Helvetica",22);
    p.setFont( myfont );

    QRect textrect = p.boundingRect ( 0, 0, pixmap.width(), pixmap.height(), Qt::AlignVCenter | Qt::AlignRight, "Mp" );
    int fonthgt = textrect.height();

    myfont.setPointSize(32);
    p.setFont( myfont );
    p.drawText( 0, -fonthgt * 4, pixmap.width(), pixmap.height(), Qt::AlignCenter, QApplication::tr("Datrix ECG Viewer") );

    myfont.setPointSize(26);
    p.setFont( myfont );
    p.drawText( 0, -fonthgt * 6/2, pixmap.width(), pixmap.height(), Qt::AlignCenter, QApplication::tr("Demo") );

    myfont.setPointSize(20);
    p.setFont( myfont );
    p.drawText( 0, 0, pixmap.width(), pixmap.height(), Qt::AlignCenter, glb_version.replace("$","").replace("Rev","Version") );

    myfont.setPointSize(10);
    p.setFont( myfont );
    p.drawText( 0, fonthgt * 3, pixmap.width(), pixmap.height(), Qt::AlignCenter, QApplication::tr("\xa9 Copyright 2017 Datrix LLC") );

    QSplashScreen *splash = new QSplashScreen(pixmap);
    splash->show();
    QTimer::singleShot( 3000, splash, SLOT(close()) );
}
/* }}} */


