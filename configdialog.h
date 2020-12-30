/**
 * @file configdialog.h
*/ 
#ifndef CONFIGDIALOG_H
#define CONFIGDIALOG_H

#include <QDialog>
#include <QtGui>
#include "pages.h"

QT_BEGIN_NAMESPACE
class QListWidget;
class QListWidgetItem;
class QStackedWidget;
QT_END_NAMESPACE

/** {{{ class ConfigDialog : public QDialog
	@brief Declare a ConfigDialog class
*/
class ConfigDialog : public QDialog
{
    Q_OBJECT

public:
    GeneralPage* generalPage;
    EcgPage* ecgPage;
    VisualizationPage* visualPage;
    UserPage* userPage;
    ConfigDialog();

    QListWidget *contentsWidget;
    QStackedWidget *pagesWidget;

public slots:
    void changePage(QListWidgetItem *current, QListWidgetItem *previous);
    void save();
    void discard();
    void changeLevel();
    void changeAnalysisType();

private:
    void createIcons();

};
/* }}} */
#endif
