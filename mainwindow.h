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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QtWidgets>

#include "ecgdata.h"
#include "showsignal.h"


namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{

    Q_OBJECT

public:
    MainWindow(QWidget *parent = 0);
    ~MainWindow();

protected:
    void changeEvent(QEvent *e);
    void closeEvent( QCloseEvent *event );
    void resizeEvent( QResizeEvent * event );
    void dropEvent( QDropEvent *event );
    void dragEnterEvent(QDragEnterEvent *event);
    void dragLeaveEvent(QDragLeaveEvent *event);


    void connect_child_to_signals( QWidget *child );

private:
    Ui::MainWindow *ui;


public:
    QComboBox *getComboEcgGainWidget() { return comboEcgGain; };

    bool isVisibleChan[3]; /* DEBUG: this really should be moved into the view; but I just need something quick for now. */

public slots:
    void moveLeft();
    void moveRight();

    void receiveData();

    void updateVisibleChild(int ignored);
    void updatePacerText( QString txt );

    void newFile();
    void open();
    void save();
    void saveAs();
    void print();
    void printStrip();
    void printPDF();
    void cut();
    void copy();
    void paste();
    void about();
    void keyPressEvent( QKeyEvent * event );
    void updateWindowMenu();
    ShowSignal *createMdiChild();
    void mdiChildActivated( QMdiSubWindow *mdiChild );
    void setActiveSubWindow( QWidget *window );
    void setFocusOnActiveWindow();
    void openEcgFile( QString fileName );

    void openMruFile( int mruFileIndex );

signals:
    void fileDragDropped( QString pathName );
    void moveLeft_signaled();
    void moveRight_signaled();
    void newFile_signaled();
    void open_signaled();
    void save_signaled();
    void saveAs_signaled();
    void print_signaled();
    void print_strip_signaled();
    void printPDF_signaled();
    void cut_signaled();
    void copy_signaled();
    void paste_signaled();
    void receiveData_signaled();
    void keyPressEvent_signaled( QKeyEvent * event );

private slots:
    void on_actionLicensing_triggered();

private:
    void createActions();
    void createMenus();
    void createToolBars();
    void createStatusBar();
    void readSettings();
    void writeSettings();
    QWidget *activeMdiChild();
    QMdiSubWindow *findMdiChild(const QString &fileName);

    QMdiArea *mdiArea;
    QSignalMapper *windowMapper;
    QSignalMapper *signalMapperMruFiles;

    QAction *actionSeparator;

    QComboBox *comboEcgGain;
	QLabel *lblPaceBeatsPerMinute;

};

extern MainWindow *glb_mainwindow;

extern int glb_bump_slopiness;
extern int glb_bump_halfwidth;

#endif // MAINWINDOW_H
