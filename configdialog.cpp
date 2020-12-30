/**
 * @file configdialog.cpp
*/
#include <QtGui>

#include "configdialog.h"
#include "pages.h"

/** {{{ ConfigDialog::ConfigDialog()
	@brief Define a constructor for ConfigDialog 
*/
ConfigDialog::ConfigDialog()
{

    setWindowIcon(QIcon(":/out/images/gear.png"));
    setMinimumWidth(700);
    setMinimumHeight(300);

    contentsWidget = new QListWidget;
    contentsWidget->setViewMode(QListView::IconMode);
    //contentsWidget->setWordWrap(true);
    contentsWidget->setIconSize(QSize(96, 84));
    contentsWidget->setMovement(QListView::Static);
    contentsWidget->setMaximumWidth(128);
    contentsWidget->setSpacing(12);
    contentsWidget->setMinimumHeight(300);
    contentsWidget->setGridSize(QSize(124, 60));

    pagesWidget = new QStackedWidget;
    generalPage = new GeneralPage;
    ecgPage = new EcgPage;
    userPage = new UserPage;
    visualPage = new VisualizationPage;

    pagesWidget->addWidget(generalPage);
    pagesWidget->addWidget(ecgPage);
    pagesWidget->addWidget(userPage);
    pagesWidget->addWidget(visualPage);

    QPushButton *closeButton = new QPushButton(tr("Close"));
    QPushButton *saveButton = new QPushButton(tr("Save"));
    QPushButton *discardButton = new QPushButton(tr("Discard"));


    createIcons();
    contentsWidget->setCurrentRow(0);


    QHBoxLayout *horizontalLayout = new QHBoxLayout;
    horizontalLayout->addWidget(contentsWidget);
    horizontalLayout->addWidget(pagesWidget, 1);

    QHBoxLayout *buttonsLayout = new QHBoxLayout;
    buttonsLayout->addStretch(1);
    buttonsLayout->addWidget(saveButton);
    buttonsLayout->addWidget(discardButton);
    buttonsLayout->addWidget(closeButton);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(horizontalLayout);
    mainLayout->addStretch(1);
    mainLayout->addSpacing(12);
    mainLayout->addLayout(buttonsLayout);
    setLayout(mainLayout);

    setWindowTitle(tr("Configuration"));



    connect(saveButton, SIGNAL(clicked()), this, SLOT(save()));
    connect(discardButton, SIGNAL(clicked()), this, SLOT(discard()));
    connect(closeButton, SIGNAL(clicked()), this, SLOT(close()));

    connect(userPage->userLevel, SIGNAL(currentIndexChanged(int)), this, SLOT(changeLevel()));
    connect(userPage->ecgType, SIGNAL(currentIndexChanged(int)), this, SLOT(changeAnalysisType()));
}
/* }}} */

/** {{{ void ConfigDialog::createIcons()
	@brief Create icons for different pages needed for configuration	
*/
void ConfigDialog::createIcons()
{
    QListWidgetItem *generalButton = new QListWidgetItem(contentsWidget);
    generalButton->setIcon(QIcon(":/out/images/gear.png"));
    generalButton->setText(tr("General"));
    generalButton->setTextAlignment(1);
    generalButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *ecgParamButton = new QListWidgetItem(contentsWidget);
    ecgParamButton->setIcon(QIcon(":/out/images/heart1.png"));
    ecgParamButton->setText(tr("ECG Parameters"));
    ecgParamButton->setTextAlignment(1);
    ecgParamButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *userProfButton = new QListWidgetItem(contentsWidget);
    userProfButton->setIcon(QIcon(":/out/images/user_32.png"));
    userProfButton->setText(tr("User Profile"));
    userProfButton->setTextAlignment(1);
    userProfButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    QListWidgetItem *visualizationButton = new QListWidgetItem(contentsWidget);
    visualizationButton->setIcon(QIcon(":/out/images/monitor_32.png"));
    visualizationButton->setText(tr("Visualisation"));
    visualizationButton->setTextAlignment(1);
    visualizationButton->setFlags(Qt::ItemIsSelectable | Qt::ItemIsEnabled);

    connect(contentsWidget,
            SIGNAL(currentItemChanged(QListWidgetItem *, QListWidgetItem *)),
            this, SLOT(changePage(QListWidgetItem *, QListWidgetItem*)));
}
/* }}} */

/** {{{ void ConfigDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
	@brief Change configuration page 
*/
void ConfigDialog::changePage(QListWidgetItem *current, QListWidgetItem *previous)
{
    if (!current)
        current = previous;

    pagesWidget->setCurrentIndex(contentsWidget->row(current));
}
/* }}} */

/** {{{ void ConfigDialog::save()
	@brief Save the configuration settings
*/
void ConfigDialog::save()
{
    qDebug()<<"SAVE";
    generalPage->saveGeneralSettings();
    ecgPage->saveECGSettings();
    visualPage->saveVisualSettings();
    close();

}
/* }}} */

/** {{{ void ConfigDialog::discard()
	@brief Discard the changes
*/
void ConfigDialog::discard()
{
    generalPage->readGeneralSettings();
    ecgPage->readECGSettings();
    visualPage->readVisualSettings();
}
/* }}} */

/** {{{ void ConfigDialog::changeLevel()
	@brief Change level from quick menu (beginner, intermediate and expert)
*/
void ConfigDialog::changeLevel()
{
    if(userPage->selectedLevel() == 1)
    {
       visualPage->spinBox->setValue(10);
       visualPage->comboBox->setCurrentIndex(1);
    }

    if(userPage->selectedLevel() == 2)
    {
       visualPage->spinBox->setValue(50);
       visualPage->comboBox->setCurrentIndex(3);
    }

    if(userPage->selectedLevel() == 3)
    {
       visualPage->spinBox->setValue(99);
       visualPage->comboBox->setCurrentIndex(4);
    }

}
/* }}} */

/** {{{ void ConfigDialog::changeAnalysisType()
	@brief Change the analysis type (slow analysis and quick analysis)
*/ 
void ConfigDialog::changeAnalysisType()
{
    if(userPage->selectedType() == 1)
    {
      ecgPage->testLine->setText("SLOW ANALYSIS");
      ecgPage->ecgLine->setText("Accurate");
    }

    if(userPage->selectedType() == 2)
    {
      ecgPage->testLine->setText("QUICK ANALYSIS");
      ecgPage->ecgLine->setText("Quick");
    }
}
/* }}} */
