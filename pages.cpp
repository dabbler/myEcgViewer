/**
 * @file pages.cpp
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

#include "pages.h"
#include "configdialog.h"



/** {{{ GeneralPage::GeneralPage()
	@brief General page constructor 
*/
GeneralPage::GeneralPage()//(QWidget *parent)
    //: QWidget(parent)
{


    QGroupBox *packagesGroup = new QGroupBox(tr("General Configuration"));

    QLabel *hospitalLabel = new QLabel(tr("Hospital Name:"));
    QLabel *doctorLabel = new QLabel(tr("Doctor Name:"));
    QLabel *phoneLabel = new QLabel(tr("Phone:"));
    QLabel *faxLabel = new QLabel(tr("Fax:"));
    QLabel *addressLabel = new QLabel(tr("Address:"));

    hospitalName = new QLineEdit();
    doctorName = new QLineEdit();
    phoneNo = new QLineEdit();
    faxNo = new QLineEdit();
    addressH = new QLineEdit();

    hospitalName->setMaximumWidth(250);
    doctorName->setMaximumWidth(200);
    phoneNo->setMaximumWidth(150);
    faxNo->setMaximumWidth(150);
    addressH->setMaximumWidth(250);


    QGridLayout *packagesLayout = new QGridLayout;
    packagesLayout->addWidget(hospitalLabel, 0, 0);
    packagesLayout->addWidget(hospitalName, 0, 1);
    packagesLayout->addWidget(doctorLabel, 1, 0);
    packagesLayout->addWidget(doctorName, 1, 1);
    packagesLayout->addWidget(phoneLabel, 2, 0);
    packagesLayout->addWidget(phoneNo, 2, 1);
    packagesLayout->addWidget(faxLabel, 3, 0);
    packagesLayout->addWidget(faxNo, 3, 1);
    packagesLayout->addWidget(addressLabel, 4, 0);
    packagesLayout->addWidget(addressH, 4, 1);

    packagesGroup->setLayout(packagesLayout);

    packagesGroup->setLayout(packagesLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(packagesGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    readGeneralSettings();

}
/* }}} */


/** {{{ void GeneralPage::saveGeneralSettings()
	@brief Save the data entered by the user in the General page 
*/
void GeneralPage::saveGeneralSettings()
{
    QSettings settings("Configuration", "General");
    settings.setValue("hName", hospitalName->text());
    settings.setValue("dName", doctorName->text());
    settings.setValue("pNo", phoneNo->text());
    settings.setValue("fNo", faxNo->text());
    settings.setValue("hAddress", addressH->text());
}
/* }}} */


/** {{{ void GeneralPage::readGeneralSettings()
	@brief Read the saved settings
*/
void GeneralPage::readGeneralSettings()
{
    QSettings settings("Configuration", "General");

    QString hName = settings.value("hName").toString();
    QString dName = settings.value("dName").toString();
    QString pNo = settings.value("pNo").toString();
    QString fNo = settings.value("fNo").toString();
    QString hAddress = settings.value("hAddress").toString();

    hospitalName->setText(hName);
    doctorName->setText(dName);
    phoneNo->setText(pNo);
    faxNo->setText(fNo);
    addressH->setText(hAddress);

}
/* }}} */ 


/** {{{ EcgPage::EcgPage(QWidget *parent)
	@brief EcgPage constructor
*/
EcgPage::EcgPage(QWidget *parent)
    : QWidget(parent)
{

    QGroupBox *packagesGroup = new QGroupBox(tr("ECG Parameters"));

    QLabel *testLabel = new QLabel(tr("test"));
    QLabel *testECGLabel = new QLabel(tr("testECGDATA:"));

    testLine = new QLineEdit();
    ecgLine = new QLineEdit();

    testLine->setMaximumWidth(200);
    ecgLine->setMaximumWidth(200);

    QGridLayout *packagesLayout = new QGridLayout;
    packagesLayout->addWidget(testLabel, 0, 0);
    packagesLayout->addWidget(testLine, 0, 1);
    packagesLayout->addWidget(testECGLabel, 1, 0);
    packagesLayout->addWidget(ecgLine, 1, 1);

    packagesGroup->setLayout(packagesLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(packagesGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    readECGSettings();
}
/* }}} */


/** {{{ void EcgPage::saveECGSettings()
	@brief Save the data entered by the user in the ECGSettings page 
*/
void EcgPage::saveECGSettings()
{
    QSettings settings("Configuration", "ECG");

    settings.setValue("test1", testLine->text());
    settings.setValue("ecg", ecgLine->text());
}
/* }}} */


/** {{{ void EcgPage::readECGSettings()
	@brief Read the saved settings
*/ 
void EcgPage::readECGSettings()
{
    QSettings settings("Configuration", "ECG");

    QString test1 = settings.value("test1").toString();
    QString ecg = settings.value("ecg").toString();

    testLine->setText(test1);
    ecgLine->setText(ecg);
}
/* }}} */


/** {{{ UserPage::UserPage(QWidget *parent)
	@brief UserPage constructor
*/ 
UserPage::UserPage(QWidget *parent)
    : QWidget(parent)
{
    QGroupBox *packagesGroup = new QGroupBox(tr("User Profiles"));

    QLabel *levelLabel = new QLabel(tr("Level: "));
    QLabel *ecgLabel = new QLabel(tr("Analysis Type: "));

    userLevel = new QComboBox;
    userLevel->addItem(tr("- level -"));
    userLevel->addItem(tr("Biginer"));
    userLevel->addItem(tr("Intermediate"));
    userLevel->addItem(tr("Expert"));

    ecgType = new QComboBox;
    ecgType->addItem(tr("- type -"));
    ecgType->addItem(tr("Slow & Accurate Analysis"));
    ecgType->addItem(tr("Quick Analysis"));

    userLevel->setMaximumWidth(100);

    QGridLayout *packagesLayout = new QGridLayout;
    packagesLayout->addWidget(levelLabel, 0, 0);
    packagesLayout->addWidget(userLevel, 0, 1);
    packagesLayout->addWidget(ecgLabel, 1, 0);
    packagesLayout->addWidget(ecgType, 1, 1);

    packagesGroup->setLayout(packagesLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(packagesGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);


    //QObject::connect(userLevel, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLevel()));

}
/* }}} */


/** {{{ int UserPage::selectedLevel()
	@brief Select user level
*/ 
int UserPage::selectedLevel()
{
    int c = userLevel->currentIndex();
    return c;
}
/* }}} */


/** {{{ int UserPage::selectedType()
	@brief  Select a type of analysis
*/
int UserPage::selectedType()
{
    int c = ecgType->currentIndex();
    qDebug()<<c;
    return c;
}
/* }}} */


/** {{{ VisualizationPage::VisualizationPage(QWidget *parent)
	@brief VisualizationPage constructor
*/ 
VisualizationPage::VisualizationPage(QWidget *parent)
    : QWidget(parent)
{

    QGroupBox *packagesGroup = new QGroupBox(tr("Visualization"));

    QLabel *testLabel = new QLabel(tr("test"));
    QLabel *test1Label = new QLabel(tr("testECGDATA:"));

    spinBox = new QSpinBox();
    comboBox = new QComboBox();
    comboBox->addItem(tr("ECG 1"));
    comboBox->addItem(tr("ECG 2"));
    comboBox->addItem(tr("ECG 3"));
    comboBox->addItem(tr("ECG 4"));
    comboBox->addItem(tr("ECG 5"));

    spinBox->setMaximumWidth(200);
    comboBox->setMaximumWidth(200);

	cbChanDataVisible[0] = new QCheckBox(tr("Chan 1"));
	cbChanDataVisible[1] = new QCheckBox(tr("Chan 2"));
	cbChanDataVisible[2] = new QCheckBox(tr("Chan 3"));

	cbChanDataVisible[0]->setChecked( glb_mainwindow->isVisibleChan[0] );
	cbChanDataVisible[1]->setChecked( glb_mainwindow->isVisibleChan[1] );
	cbChanDataVisible[2]->setChecked( glb_mainwindow->isVisibleChan[2] );
	connect( cbChanDataVisible[0], SIGNAL(toggled(bool)), this, SLOT(visibilityChan1(bool)) );
	connect( cbChanDataVisible[1], SIGNAL(toggled(bool)), this, SLOT(visibilityChan2(bool)) );
	connect( cbChanDataVisible[2], SIGNAL(toggled(bool)), this, SLOT(visibilityChan3(bool)) );

    QGridLayout *packagesLayout = new QGridLayout;
    packagesLayout->addWidget(testLabel, 0, 0);
    packagesLayout->addWidget(spinBox, 0, 1);
    packagesLayout->addWidget(test1Label, 1, 0);
    packagesLayout->addWidget(comboBox, 1, 1);

    packagesLayout->addWidget( cbChanDataVisible[0], 3, 1 );
    packagesLayout->addWidget( cbChanDataVisible[1], 4, 1 );
    packagesLayout->addWidget( cbChanDataVisible[2], 5, 1 );

    packagesGroup->setLayout(packagesLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(packagesGroup);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    readVisualSettings();

}
/* }}} */


/** {{{ void VisualizationPage::saveVisualSettings()
	@brief Save settings entered by the user in the Visualization page
*/
void VisualizationPage::saveVisualSettings()
{
    QSettings settings("Configuration", "Visualization");

    QString spinStr;
    QString comboStr;

    spinStr.setNum(spinBox->value());
    comboStr.setNum(comboBox->currentIndex());

    settings.setValue("spin", spinStr );
    settings.setValue("combo", comboStr);
}
/* }}} */



/** {{{ void VisualizationPage::visibilityChan1()
	@brief Change the channel visibility state
*/
void VisualizationPage::visibilityChan1( bool isChecked )
{
	glb_mainwindow->isVisibleChan[0] = isChecked;
}
/* }}} */


/** {{{ void VisualizationPage::visibilityChan2()
	@brief Change the channel visibility state
*/
void VisualizationPage::visibilityChan2( bool isChecked )
{
	glb_mainwindow->isVisibleChan[1] = isChecked;
}
/* }}} */


/** {{{ void VisualizationPage::visibilityChan3()
	@brief Change the channel visibility state
*/
void VisualizationPage::visibilityChan3( bool isChecked )
{
	glb_mainwindow->isVisibleChan[2] = isChecked;
}
/* }}} */




/** {{{ void VisualizationPage::readVisualSettings()
	@brief Read the saved settings
*/ 
void VisualizationPage::readVisualSettings()
{
    QSettings settings("Configuration", "Visualization");

    QString spin = settings.value("spin").toString();
    QString combo = settings.value("combo").toString();


    bool ok;
    int spinInt = spin.toInt(&ok, 10);
    int comboInt = combo.toInt(&ok, 10);

    spinBox->setValue(spinInt);
    comboBox->setCurrentIndex(comboInt);

}
/* }}} */


