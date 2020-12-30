/**
 * @file mainwindow.cpp
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
#include <QtGui>	// if we include <QtGui> there is no need to include every class used: <QString>, <QFileDialog>,...

#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "showsignal.h"
#include "ecgdata.h"
#include "infobox.h"

#include "configdialog.h"

MainWindow *glb_mainwindow = NULL;

int glb_bump_slopiness = 70;
int glb_bump_halfwidth = 10;



/** {{{ MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
    @brief MainWindow constructor
*/
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    isVisibleChan[0] = true;
    isVisibleChan[1] = true;
    isVisibleChan[2] = true;

    mdiArea = new QMdiArea;
    mdiArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    mdiArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setCentralWidget(mdiArea);
    connect( mdiArea, SIGNAL(subWindowActivated(QMdiSubWindow *)),this, SLOT(mdiChildActivated(QMdiSubWindow *)) );

    windowMapper = new QSignalMapper(this);
    connect( windowMapper, SIGNAL(mapped(QWidget *)), this, SLOT(setActiveSubWindow(QWidget *)) );

    signalMapperMruFiles = new QSignalMapper(this);
    connect( signalMapperMruFiles, SIGNAL(mapped(int)), this, SLOT(openMruFile(int)) );

    mdiArea->setAcceptDrops(true);
    setAcceptDrops(true);

    connect( this, SIGNAL(fileDragDropped(QString)), this, SLOT(openEcgFile(QString)) );

    createActions();
    createMenus();
    createToolBars();
    createStatusBar();
    mdiChildActivated( NULL );

    readSettings();

    setWindowTitle( tr("ECG") );
    setWindowIcon( QIcon(":/out/images/heart1.png") );
    setUnifiedTitleAndToolBarOnMac(true);

}
/* }}} */


/** {{{ MainWindow::~MainWindow()
    @brief MainWindow destructor
 */
MainWindow::~MainWindow()
{
    delete ui;
}
/* }}} */


/** {{{ void MainWindow::changeEvent(QEvent *e)
 */
void MainWindow::changeEvent(QEvent *e)
{
    QMainWindow::changeEvent(e);
    switch (e->type()) {
    case QEvent::LanguageChange:
        ui->retranslateUi(this);
        break;
    default:
        break;
    }
}
/* }}} */


/** {{{ Drag and drop of files to open them */

/** {{{ void MainWindow::dropEvent(QDropEvent *event)
 */
void MainWindow::dropEvent(QDropEvent *event)
{
    QList<QUrl> urlList;
    QString fName;
    QFileInfo info;

    if ( event->mimeData()->hasUrls() ) {

        urlList = event->mimeData()->urls(); // returns list of QUrls

        // if just text was dropped, urlList is empty (size == 0)

        // if at least one QUrl is present in list
        foreach ( QUrl aUrl, urlList ) {
            fName = aUrl.toLocalFile(); // convert first QUrl to local path
            info.setFile( fName ); // information about file
            if ( info.isFile() ) {
                // qDebug() << fName << "is a file";
                emit fileDragDropped(fName);
            } else {
                qDebug() << fName << "is NOT a file";
            }
        }
	}

    event->acceptProposedAction();

    return;
}
/* }}} */


/** {{{ void MainWindow::dragEnterEvent(QDragEnterEvent *event)
 */
void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    event->acceptProposedAction();
}
/* }}} */


/** {{{ void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
 */
void MainWindow::dragLeaveEvent(QDragLeaveEvent *event)
{
    event->accept();
}
/* }}} */


/* }}} */


/** {{{ void MainWindow::createActions()
    @brief Create actions ( New, Open, Save, ...)
*/
void MainWindow::createActions()
{
    ui->action_Open->setStatusTip( tr("Open an existing file") );
    ui->action_Print->setStatusTip( tr("Print this document") );
    ui->action_Print_Strip->setStatusTip( tr("Print ECG strips at 25%, 50% and 75% offset") );
    ui->actionOutput_to_Pdf->setStatusTip( tr("Print this document into a PDF file") );
    ui->actionBackwards->setStatusTip( tr("Backward through data") );
    ui->actionForwards->setStatusTip( tr("Forward through data") );
    ui->actionE_xit->setShortcut( tr("Ctrl+Q") );
    ui->actionE_xit->setStatusTip( tr("Exit the application") );
    ui->action_Close->setShortcut( tr("Ctrl+F4") );
    ui->action_Close->setStatusTip( tr("Close the active window") );
    ui->actionClose_All->setStatusTip( tr("Close all the windows") );
    ui->action_Tile->setStatusTip( tr("Tile the windows") );
    ui->actionCascade->setStatusTip( tr("Cascade the windows") );
    ui->action_Next->setStatusTip( tr("Move the focus to the next window") );
    ui->action_Previous->setStatusTip( tr("Move the focus to the previous window") );
    ui->actionAbout->setStatusTip( tr("Show the application's About box") );
    ui->actionLicensing->setStatusTip( tr("Show the application's Licensing box") );

    connect( ui->action_Open, SIGNAL(triggered()), this, SLOT(open()) );
    connect( ui->action_Print, SIGNAL(triggered()), this, SLOT(print()) );
    connect( ui->action_Print_Strip, SIGNAL(triggered()), this, SLOT(printStrip()) );
    connect( ui->actionOutput_to_Pdf, SIGNAL(triggered()), this, SLOT(printPDF()) );
    connect( ui->actionBackwards, SIGNAL(triggered()), this, SLOT(moveLeft()) );
    connect( ui->actionForwards, SIGNAL(triggered()), this, SLOT(moveRight()) );
    connect( ui->actionE_xit, SIGNAL(triggered()), qApp, SLOT(closeAllWindows()) );
    connect( ui->action_Close, SIGNAL(triggered()), mdiArea, SLOT(closeActiveSubWindow()) );
    connect( ui->actionClose_All, SIGNAL(triggered()), mdiArea, SLOT(closeAllSubWindows()) );
    connect( ui->action_Tile, SIGNAL(triggered()), mdiArea, SLOT(tileSubWindows()) );
    connect( ui->actionCascade, SIGNAL(triggered()), mdiArea, SLOT(cascadeSubWindows()) );
    connect( ui->action_Next, SIGNAL(triggered()), mdiArea, SLOT(activateNextSubWindow()) );
    connect( ui->action_Previous, SIGNAL(triggered()), mdiArea, SLOT(activatePreviousSubWindow()) );
    connect( ui->actionAbout, SIGNAL(triggered()), this, SLOT(about()) );
    connect( ui->actionLicensing, SIGNAL(triggered()), this, SLOT(licensing()) );


    actionSeparator = new QAction(this);
    actionSeparator->setSeparator(true);
    ui->menu_Window->addAction( actionSeparator );
}
/* }}} */


/** {{{ void MainWindow::createMenus()
    @brief Create menus
*/
void MainWindow::createMenus()
{
    updateWindowMenu();
    connect( ui->menu_Window, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()) );
    connect( ui->menu_File, SIGNAL(aboutToShow()), this, SLOT(updateWindowMenu()) );
}
/* }}} */


/** {{{ void MainWindow::createToolBars()
    @brief Create toolbars
*/
void MainWindow::createToolBars()
{
    comboEcgGain = new QComboBox();
    comboEcgGain->addItem("0.1 mm/mV", 0.1 );
    comboEcgGain->addItem("0.5 mm/mV", 0.5 );
    comboEcgGain->addItem("1 mm/mV", 1.0 );
    comboEcgGain->addItem("2 mm/mV", 2.0 );
    comboEcgGain->addItem("5 mm/mV", 5.0 );
    comboEcgGain->addItem("10 mm/mV", 10.0 );
    comboEcgGain->addItem("20 mm/mV", 20.0 );
    comboEcgGain->addItem("50 mm/mV", 50.0 );
    comboEcgGain->addItem("100 mm/mV", 100.0 );
    comboEcgGain->addItem("250 mm/mV", 250.0 );
    comboEcgGain->addItem("500 mm/mV", 500.0 );
    connect( comboEcgGain, SIGNAL(currentIndexChanged(int)), this, SLOT(updateVisibleChild(int)) );
    comboEcgGain->setCurrentIndex( 3 );

    ui->toolBarView->addWidget( comboEcgGain );

	lblPaceBeatsPerMinute = new QLabel();
	lblPaceBeatsPerMinute->setObjectName(QString::fromUtf8("lblPaceBeatsPerMinute"));
    ui->toolBarView->addWidget( lblPaceBeatsPerMinute );
	lblPaceBeatsPerMinute->hide();
}
/* }}} */


/** {{{ void MainWindow::createStatusBar()
    @brief Create status bar
*/
void MainWindow::createStatusBar()
{
    statusBar()->showMessage( tr("Ready") );
}
/* }}} */


/** {{{ void MainWindow::readSettings()
    @brief Read settings
*/
void MainWindow::readSettings()
{
    QSettings settings("Datrix", "DatrixECGViewer");
    QPoint pos = settings.value("pos", QPoint(200, 200)).toPoint();
    QSize size = settings.value("size", QSize(400, 400)).toSize();
    move(pos);
    resize(size);
}
/* }}} */


/** {{{ void MainWindow::writeSettings()
    @brief Write settings
*/
void MainWindow::writeSettings()
{
    QSettings settings("Datrix", "DatrixECGViewer");
    settings.setValue( "pos", pos() );
    settings.setValue( "size", size() );
}
/* }}} */


/** {{{ QWidget *MainWindow::activeMdiChild()
    @brief Return active MdiChild
*/
QWidget *MainWindow::activeMdiChild()
{
    if ( QMdiSubWindow *activeSubWindow = mdiArea->activeSubWindow() ) {
        return qobject_cast<QWidget *>( activeSubWindow->widget() );
    }
    return 0;
}
/* }}} */


/** {{{ QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
    @brief Find MdiChild that is associated with given fileName
*/
QMdiSubWindow *MainWindow::findMdiChild(const QString &fileName)
{
    QString canonicalFilePath = QFileInfo(fileName).canonicalFilePath();

    foreach (QMdiSubWindow *window, mdiArea->subWindowList()) {
        QWidget *ss = qobject_cast<QWidget *>( window->widget() );
        if (ss->windowTitle() == canonicalFilePath)
            return window;
    }
    return 0;
}
/* }}} */


/** {{{ void MainWindow::setActiveSubWindow(QWidget *window)
    @brief Set active SubWindow
*/
void MainWindow::setActiveSubWindow(QWidget *window)
{
    if ( ! window ) {
        return;
    }

	QMdiSubWindow *existing = qobject_cast<QMdiSubWindow *>(window);
	mdiArea->setActiveSubWindow(existing);
	connect_child_to_signals( qobject_cast<QWidget *>(existing->widget()) );
}
/* }}} */



/** {{{ void MainWindow::closeEvent(QCloseEvent *event)
    @brief Close event
*/
void MainWindow::closeEvent(QCloseEvent *event)
{
    mdiArea->closeAllSubWindows();
    if ( activeMdiChild() ) {
        event->ignore();
    } else {
        writeSettings();
        event->accept();
    }
}
/* }}} */



/** {{{ void MainWindow::newFile()
    @brief Create new file
*/
void MainWindow::newFile()
{
    // qDebug("MainWindow::newFile() -> New");

}
/* }}} */


/** {{{ void MainWindow::open()
    @brief Open ECG data file
*/
void MainWindow::open()
{
    QSettings settings("Datrix", "DatrixECGViewer");
	QString directoryMRU = "/mnt";
#ifdef Q_OS_WIN32
    directoryMRU = settings.value("Most Recently Used/pathOpen").toString();
#endif

    QFileDialog fileOpenDialog;
    QString fileName = fileOpenDialog.getOpenFileName( this, tr("Open Datrix ECG data file"), directoryMRU, tr("Raw ECG Data (*.dat)") );

    emit fileDragDropped(fileName);
}
/* }}} */


/** {{{ openEcgFile( QString fileName )
 */
void MainWindow::openEcgFile( QString fileName )
{
    ShowSignal *child = NULL;
    QSettings settings("Datrix", "DatrixECGViewer");
    QString directoryMRU = settings.value("Most Recently Used/pathOpen").toString();

    if ( ! fileName.isEmpty() ) {
        QFileInfo fileInfo(fileName);

        // qDebug() << "last folder used for opening an ECG file was:" << fileInfo.absolutePath();
        settings.setValue("Most Recently Used/pathOpen", fileInfo.absolutePath() );

        QMdiSubWindow *existing = findMdiChild(fileName);
        if (existing) {
            mdiArea->setActiveSubWindow(existing);
            connect_child_to_signals( qobject_cast<QWidget *>(existing->widget()) );
            return;
        }

        child = createMdiChild();
		connect_child_to_signals(child);
        child->setComboViewTypeAction( ui->toolBarView->addWidget( child->getComboViewTypeWidget() ) );
        child->m_ecgdata = new EcgData( fileName, child );
        child->setWindowTitle(fileName);
        if ( child->m_ecgdata ) {
            statusBar()->showMessage(tr("ECG file loaded"), 2000);
            child->setFocus(Qt::ShortcutFocusReason);
            child->show();
            child->showMaximized();

        } else {
            child->close();
        }
    }


    /* plop the filename into the beginning of the most recent used (mru) list */
    QStringList mruFileNames = settings.value( QString("MRU Files") ).toStringList();
    mruFileNames.removeOne(fileName);
    mruFileNames.prepend(fileName);
    if ( mruFileNames.size() > 9 ) {
        mruFileNames.removeLast();
    }
    settings.setValue( "MRU Files", mruFileNames );

    statusBar()->showMessage(tr("File %1 loaded").arg(fileName), 2000);

    return;
}
/* }}} */


/** {{{ void MainWindow::updatePacerText( QString txt )
 */
void MainWindow::updatePacerText( QString txt )
{
#ifdef ANTIQUE	/* removed per ticket #896 */
	lblPaceBeatsPerMinute->setText( QString("   %1    ").arg(txt) );
	if ( txt.isEmpty() ) {
		lblPaceBeatsPerMinute->hide();
	} else {
		lblPaceBeatsPerMinute->show();
	}
#else
    Q_UNUSED(txt);
    lblPaceBeatsPerMinute->hide();
#endif
}
/* }}} */


/** {{{ void MainWindow::connect_child_to_signals( QWidget *child )
    @brief Connect child to signals
*/
void MainWindow::connect_child_to_signals( QWidget *child )
{
    // qDebug() << "connect_child_to_signals(" << child->objectName() << ")";

    disconnect( this, SIGNAL(moveLeft_signaled()), 0, 0 );
    disconnect( this, SIGNAL(moveRight_signaled()), 0, 0 );
    disconnect( this, SIGNAL(newFile_signaled()), 0, 0 );
    disconnect( this, SIGNAL(open_signaled()), 0, 0 );
    disconnect( this, SIGNAL(save_signaled()), 0, 0 );
    disconnect( this, SIGNAL(saveAs_signaled()), 0, 0 );
    disconnect( this, SIGNAL(print_signaled()), 0, 0 );
    disconnect( this, SIGNAL(print_strip_signaled()), 0, 0 );
    disconnect( this, SIGNAL(printPDF_signaled()), 0, 0 );

    if ( child->objectName() == "ShowSignal" ) {

        ShowSignal *ss = (ShowSignal *) child;

        connect( this, SIGNAL(moveLeft_signaled()), ss, SLOT(moveLeft()) );
        connect( this, SIGNAL(moveRight_signaled()), ss, SLOT(moveRight()) );
        connect( this, SIGNAL(newFile_signaled()), ss, SLOT(newFile()) );
        connect( this, SIGNAL(save_signaled()), ss, SLOT(save()) );
        connect( this, SIGNAL(saveAs_signaled()), ss, SLOT(saveAs()) );
        connect( this, SIGNAL(print_signaled()), ss, SLOT(print()) );
        connect( this, SIGNAL(print_strip_signaled()), ss, SLOT(print_strip()) );
        connect( this, SIGNAL(printPDF_signaled()), ss, SLOT(printPDF()) );
        connect( this, SIGNAL(keyPressEvent_signaled(QKeyEvent*)), ss, SLOT(keyPressEvent(QKeyEvent*)) );

        connect( ss, SIGNAL(updatePacerText(QString)), this, SLOT(updatePacerText(QString)) );
    }
}
/* }}} */



/** {{{ void MainWindow::updateVisibleChild( int index )
    @brief repaint the visible child with the focus
*/
void MainWindow::updateVisibleChild( int index )
{
    Q_UNUSED(index);
    ShowSignal *ss = qobject_cast<ShowSignal *>( activeMdiChild() );
    if ( ss ) {
        ss->gain_mm_per_mV = glb_mainwindow->getComboEcgGainWidget()->itemData( glb_mainwindow->getComboEcgGainWidget()->currentIndex() ).toDouble();
        ss->update();
    }
}
/* }}} */


/** {{{ void MainWindow::save()
    @brief Save the file
*/
void MainWindow::save()
{
#ifdef ANTIQUE
    if ( activeMdiChild() && activeMdiChild()->save() )
#else
    ShowSignal *ss = qobject_cast<ShowSignal *>( activeMdiChild() );
    if ( ss && ss->save() )
#endif
    {
        statusBar()->showMessage(tr("File saved"), 2000);
    }
}
/* }}} */


/** {{{ void MainWindow::saveAs()
    @brief Save the file
*/
void MainWindow::saveAs()
{
    ShowSignal *ss = qobject_cast<ShowSignal *>( activeMdiChild() );
    if ( ss && ss->saveAs() ) {
        statusBar()->showMessage(tr("File saved"), 2000);
    }
}
/* }}} */


/** {{{ void MainWindow::print()
  @brief Print the ECG data
 */
void MainWindow::print()
{
#ifdef ANTIQUE
    if ( activeMdiChild() && activeMdiChild()->className() == "ShowSignal" ) {
        qobject_cast<ShowSignal *>activeMdiChild()->print();
        statusBar()->showMessage(tr("Document printed"), 2000);
    }
#else
    emit print_signaled();
#endif
}
/* }}} */


/** {{{ void MainWindow::printStrip()
  @brief Print ECG strips at 25%, 50% and 75% offset in the ECG data
 */
void MainWindow::printStrip()
{
#ifdef ANTIQUE
    if ( activeMdiChild() && activeMdiChild()->className() == "ShowSignal" ) {
        qobject_cast<ShowSignal *>activeMdiChild()->print();
        statusBar()->showMessage(tr("Document printed"), 2000);
    }
#else
    emit print_strip_signaled();
#endif
}
/* }}} */


/** {{{ void MainWindow::printPDF()
  Print ECG data on PDF document
 */
void MainWindow::printPDF()
{
    statusBar()->showMessage(tr("Document exported to PDF"), 2000);
#ifdef ANTIQUE
    if ( activeMdiChild() && activeMdiChild()->className() == "ShowSignal" ) {
        qobject_cast<ShowSignal *>activeMdiChild()->printPDF();
        statusBar()->showMessage(tr("Document exported to PDF"), 2000);
    }
#else
    emit printPDF_signaled();
#endif
}
/* }}} */


/** {{{ void MainWindow::moveLeft()
  @brief Move the ECG data to the left
 */
void MainWindow::moveLeft()
{
#ifdef ANTIQUE
    if ( activeMdiChild() ) {
        activeMdiChild()->moveLeft();
    }
#else
    emit moveLeft_signaled();
#endif
}
/* }}} */


/** {{{ void MainWindow::moveRight()
  @brief Move the ECG data to the right
 */
void MainWindow::moveRight()
{
#ifdef ANTIQUE
    if ( activeMdiChild() ) {
        activeMdiChild()->moveRight();
    }
#else
    emit moveRight_signaled();
#endif
}
/* }}} */



/** {{{ void MainWindow::cut()
  @brief Cut the ECG data
 */
void MainWindow::cut()
{
#ifdef ANTIQUE
    if ( activeMdiChild() ) {
        activeMdiChild()->cut();
    }
#else
    emit cut();
#endif
}
/* }}} */


/** {{{ void MainWindow::copy()
  @brief Copy the ECG data
 */
void MainWindow::copy()
{
#ifdef ANTIQUE
    if ( activeMdiChild() ) {
        activeMdiChild()->copy();
    }
#else
    emit copy();
#endif
}
/* }}} */


/** {{{ void MainWindow::paste()
    @brief Paste the ECG data
*/
void MainWindow::paste()
{
#ifdef ANTIQUE
    if ( activeMdiChild() ) {
        activeMdiChild()->paste();
    }
#else
    emit paste();
#endif
}
/* }}} */


/** {{{ void MainWindow::about()
    @brief Information about DatrixECGViewer
*/
void MainWindow::about()
{
    extern void show_splash();
    show_splash();
/*
    QMessageBox::about( this, tr("About DatrixECGViewer"), tr("The <b>DatrixECGViewer</b> holter ecg visualization software.") );
*/
}
/* }}} */


/** {{{ void MainWindow::openMruFile( int mruFileIndex )
 */
void MainWindow::openMruFile( int mruFileIndex )
{
    QSettings settings("Datrix", "DatrixECGViewer");
    QStringList mruFileNames = settings.value( QString("MRU Files") ).toStringList();
    QString fileName = mruFileNames.value(mruFileIndex);
    qDebug() << "MainWindow::openMruFile(" << mruFileIndex << ")    fileName = " << fileName;
    if ( ! fileName.isEmpty() ) {
		emit fileDragDropped(fileName);
    }
}
/* }}} */



void MainWindow::on_actionLicensing_triggered()
{
	InfoBox	*boxLicensing;

	QString msg("The software included in this product contains copyrighted software that is licensed under the GPLv3.\n\nYou may obtain the complete Corresponding Source code from our distribution site at https://github.com/dabbler/SironaEcgViewer.git");
	boxLicensing = new InfoBox( NULL, msg, 15000 );
	boxLicensing->addWidgetHeight( 100 );
}



/** {{{ void MainWindow::keyPressEvent( QKeyEvent * event )
    @brief Key press event
*/
void MainWindow::keyPressEvent( QKeyEvent * event )
{
    switch ( event->key() ) {

        case Qt::Key_Plus:
            // qDebug() << " MainWindow::keyPressEvent()  ->  Qt::Key_Plus:";
            break;
        case Qt::Key_Minus:
            // qDebug() << " MainWindow::keyPressEvent()  ->  Qt::Key_Minus:";
            break;
        case Qt::Key_Left:
            // qDebug() << " MainWindow::keyPressEvent()  ->  Qt::Key_Left:";
            break;
        case Qt::Key_Right:
            // qDebug() << " MainWindow::keyPressEvent()  ->  Qt::Key_Right:";
            break;
        case Qt::Key_Down:
            // qDebug() << " MainWindow::keyPressEvent()  ->  Qt::Key_Down:";
            break;
        case Qt::Key_Up:
            // qDebug() << " MainWindow::keyPressEvent()  ->  Qt::Key_Up:";
            break;
    }
#ifdef ANTIQUE
    if ( activeMdiChild() ) {
        ((ShowSignal *) activeMdiChild())->keyPressEvent(event);
    }
#else
    // qDebug() << "MainWindow::keyPressEvent() sending ->  emit keyPressEvent_signaled(event)";
    // emit keyPressEvent_signaled( event );
#endif
    event->ignore();
}
/* }}} */


/** {{{ void MainWindow::mdiChildActivated( QMdiSubWindow *mdiChild )
    @brief Update menus
*/
void MainWindow::mdiChildActivated( QMdiSubWindow *mdiChild )
{
    bool hasMdiChild = (activeMdiChild() != 0);
    ui->action_Print->setEnabled(hasMdiChild);
    ui->action_Print_Strip->setEnabled(hasMdiChild);
    ui->actionOutput_to_Pdf->setEnabled(hasMdiChild);
    ui->actionForwards->setEnabled(hasMdiChild);
    ui->actionBackwards->setEnabled(hasMdiChild);

    ui->action_Close->setEnabled(hasMdiChild);
    ui->actionClose_All->setEnabled(hasMdiChild);
    ui->action_Tile->setEnabled(hasMdiChild);
    ui->actionCascade->setEnabled(hasMdiChild);
    ui->action_Next->setEnabled(hasMdiChild);
    ui->action_Previous->setEnabled(hasMdiChild);
    actionSeparator->setVisible(hasMdiChild);

    // bool hasSelection = (activeMdiChild());


    /** Show only the viewType comboBox for the active window and hide all the others. */
    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    for ( int i = 0; i < windows.size() ; ++i ) {
        ShowSignal *ss = qobject_cast<ShowSignal *>( windows.at(i)->widget() );
        if ( windows.at(i) == mdiChild ) {
            ss->showComboViewType();
        } else {
            ss->hideComboViewType();
        }
    }
}
/* }}} */


/** {{{ void MainWindow::updateWindowMenu()
    @brief Update window menu
*/
void MainWindow::updateWindowMenu()
{
    /** clear out all the old window window names to recreate the window names later */
    foreach ( QAction *act, ui->menu_Window->actions() ) {
        if ( act->objectName() == "mdiwin" ) {
            ui->menu_Window->removeAction(act);
        }
    }

    QList<QMdiSubWindow *> windows = mdiArea->subWindowList();
    actionSeparator->setVisible( ! windows.isEmpty() );

    /** recreate all the window names for user selection */
    for (int i = 0; i < windows.size(); ++i) {
        QWidget *child = qobject_cast<QWidget *>( windows.at(i)->widget() );

        if ( child ) {
            QString text;
            if (i < 9) {
                text = tr("&%1 %2")
                    .arg( i + 1 )
                    .arg( child->windowTitle() );
            } else {
                text = tr("%1 %2")
                    .arg( i + 1 )
                    .arg( child->windowTitle() );
            }
            QAction *action  = ui->menu_Window->addAction(text);
            action->setObjectName("mdiwin");
            action->setCheckable(true);
            action->setChecked( child == activeMdiChild() );
            connect( action, SIGNAL(triggered()), windowMapper, SLOT(map()) );
            windowMapper->setMapping( action, windows.at(i) );
        }
    }




    QSettings settings("Datrix", "DatrixECGViewer");

    QStringList mruFileNames = settings.value( QString("MRU Files") ).toStringList();

    delete signalMapperMruFiles;
    signalMapperMruFiles = new QSignalMapper(this);
    connect( signalMapperMruFiles, SIGNAL(mapped(int)), this, SLOT(openMruFile(int)) );

    /** clear out all the old MRU Filenames to recreate the MRU Filenames later */
    foreach ( QAction *act, ui->menu_File->actions() ) {
        if ( act->objectName().contains("mruFile") ) {
            ui->menu_File->removeAction(act);
            delete act;
        }
    }

    for ( int eachPossibleFile = 0 ; eachPossibleFile < mruFileNames.size() ; eachPossibleFile++ ) {
        QString mruFileName = mruFileNames[eachPossibleFile];
        QAction *action  = ui->menu_File->addAction( mruFileName );
        action->setObjectName( QString("mruFile%1").arg(eachPossibleFile) );
        action->setShortcut( QKeySequence(tr("Ctrl+%1").arg(eachPossibleFile+1)) );
        connect( action, SIGNAL(triggered()), signalMapperMruFiles, SLOT(map()) );
        signalMapperMruFiles->setMapping( action, eachPossibleFile );
    }

}
/* }}} */


/** {{{ ShowSignal *MainWindow::createMdiChild()
    @brief Create mdi child
*/
ShowSignal *MainWindow::createMdiChild()
{
    ShowSignal *child = new ShowSignal(mdiArea);
    mdiArea->addSubWindow(child);
    /* connect - test */
    connect( child, SIGNAL(focusChanged()), this, SLOT(setFocusOnActiveWindow()) );

    return child;
}
/* }}} */


















/* {{{ MainWindow::resizeEvent()
*/
void MainWindow::resizeEvent( QResizeEvent * event )
{
	Q_UNUSED(event);
	ShowSignal *ss = qobject_cast<ShowSignal *>( activeMdiChild() );
	if ( ss ) {
		ss->resize( event->size() );
	}
	// qDebug("MainWindow::resizing to %d x %d", width(), height() );

	if ( width() * 55/100 > height() ) {
		ui->menubar->hide();
		addToolBar( Qt::RightToolBarArea, ui->toolBarFile );
		addToolBar( Qt::RightToolBarArea, ui->toolBarView );
		ui->toolBarView->hide();
		ui->menubar->hide();
		ui->statusbar->hide();
	} else {
		ui->menubar->show();
		addToolBar( Qt::TopToolBarArea, ui->toolBarFile );
		addToolBar( Qt::TopToolBarArea, ui->toolBarView );
		ui->toolBarView->show();
		ui->menubar->show();
		ui->statusbar->show();
	}
}

/* }}} */


/* {{{ void MainWindow::receiveData()
*/
void MainWindow::receiveData()
{
}
/* }}} */


/** {{{ void MainWindow::setFocusOnActiveWindow()
 */
void MainWindow::setFocusOnActiveWindow()
{
    if ( activeMdiChild() ) {
        activeMdiChild()->setFocus();
    }
}
/* }}} */


#ifdef ANTIQUE

const int iterations = 20;

/* {{{ void spin(int &iteration)
*/
void spin( int &iteration )
{
	const int work = 1000 * 1000 * 40;
	volatile int v = 0;
	for ( int j = 0; j < work; ++j ) {
		++v;
	}

	qDebug() << "iteration" << iteration << "in thread" << QThread::currentThreadId();
}
/* }}} */


/* {{{ void run()
*/
void run()
{
	// Prepare the vector.
	QVector<int> vector;
	for ( int i = 0; i < iterations; ++i ) {
		vector.append( i );
	}

	// Create a progress dialog.
	QProgressDialog dialog;
	dialog.setLabelText( QString( "Progressing using %1 thread(s)..." ).arg( QThread::idealThreadCount() ) );


	// Create a QFutureWatcher and conncect signals and slots.
	QFutureWatcher<void> futureWatcher;
	QObject::connect( &futureWatcher, SIGNAL( finished() ), &dialog, SLOT( reset() ) );
	QObject::connect( &dialog, SIGNAL( canceled() ), &futureWatcher, SLOT( cancel() ) );
	QObject::connect( &futureWatcher, SIGNAL( progressRangeChanged( int, int ) ), &dialog, SLOT( setRange( int, int ) ) );
	QObject::connect( &futureWatcher, SIGNAL( progressValueChanged( int ) ), &dialog, SLOT( setValue( int ) ) );

	// Start the computation.
	futureWatcher.setFuture( QtConcurrent::map( vector, spin ) );

	// Display the dialog and start the event loop.
	dialog.exec();

	futureWatcher.waitForFinished();

	// Query the future to check if was canceled.
	qDebug() << "Canceled?" << futureWatcher.future().isCanceled();
}
/* }}} */

#endif


