/**
 * @file configdialog.h
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
