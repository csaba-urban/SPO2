#include "mainform.h"

MainForm::MainForm( QMainWindow* aParent )
: QMainWindow( aParent )
{
	ui.setupUi(this);

	connect( ui.uiNewStudyPushButton, SIGNAL( released() ), this, SLOT( startNewStudy() ) );

}

MainForm::~MainForm()
{

}

void MainForm::startNewStudy()
{
	ui.uiStackedWidget->setCurrentIndex( 1 ); //show the new study stack page

}
