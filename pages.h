/**
 * @file pages.h
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

#ifndef PAGES_H
#define PAGES_H

#include <QWidget>
#include <QtGui>

#include "mainwindow.h"	// DEBUG: just used for isVisibleChan[] for now


/** {{{ class GeneralPage : public QWidget 
	@brief Declare a GeneralPage class
*/
class GeneralPage : public QWidget
{
public:
    GeneralPage();//(QWidget *parent =0);

    QLineEdit *doctorName;
    QLineEdit *hospitalName;
    QLineEdit *phoneNo;
    QLineEdit *faxNo;
    QLineEdit *addressH;

    void saveGeneralSettings();
    void readGeneralSettings();

};
/* }}} */


/** {{{ class EcgPage : public QWidget 
	@brief Declare a EcgPage class
*/
class EcgPage : public QWidget
{
public:
    EcgPage(QWidget *parent = 0);

    QLineEdit *testLine;
    QLineEdit *ecgLine;

    void saveECGSettings();
    void readECGSettings();
};
/* }}} */


/** {{{ class UserPage : public QWidget 
	@brief Declare a UserPage class
*/
class UserPage : public QWidget
{
public:
    UserPage(QWidget *parent = 0);

    QComboBox *userLevel;
    QComboBox *ecgType;
    int selectedLevel();
    int selectedType();
};
/* }}} */


/** {{{ class VisualizationPage : public QWidget 
	@brief Declare a VisualizationPage class
*/
class VisualizationPage : public QWidget
{
	Q_OBJECT

public:
    VisualizationPage(QWidget *parent = 0);

    QSpinBox *spinBox;
    QComboBox *comboBox;
    QCheckBox *cbChanDataVisible[3];

    void saveVisualSettings();
    void readVisualSettings();
	
public slots:
	void visibilityChan1( bool isChecked );
	void visibilityChan2( bool isChecked );
	void visibilityChan3( bool isChecked );

};
/* }}} */

#endif
