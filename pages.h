/**
 * @file pages.h
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
