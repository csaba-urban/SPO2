#include "mainform.h"

#include <QMessageBox>

MainForm::MainForm( QMainWindow* aParent )
: QMainWindow( aParent )
{
	ui.setupUi(this);

	ui.uiStackedWidget->setCurrentIndex( STACKED_WIDGET_FIRST_PAGE );

	connect( ui.uiNewStudyPushButton, &QPushButton::released, this, &MainForm::pushButtonClicked );
	connect( ui.uiExitFromStudyPushButton, &QPushButton::released, this, &MainForm::pushButtonClicked );
	connect( ui.uiNewStudyDeatilsListWidget, &QListWidget::currentTextChanged, this, &MainForm::newStudyDeatilsListWidgetCurrentTextChaned );

	connect( ui.uiPatientBirthDateDateEdit, &QDateEdit::dateChanged, this, [=] ( const QDate& aDate )
	{
		ui.uiPatientAgeLineEdit->setText( QString::number( calculateAgeFromBirthDate( aDate ) ) );
	} );

}

MainForm::~MainForm()
{

}

void MainForm::pushButtonClicked()
{
	//New Study button pushed
	if ( sender() == ui.uiNewStudyPushButton )
	{
		ui.uiNewStudyDeatilsListWidget->setCurrentRow( 0 );
		ui.uiStackedWidget->setCurrentIndex( STACKED_WIDGET_NEW_STUDY_PAGE ); //show the new study stack page
	}

	// Exit (from New Study) pushed
	if ( sender() == ui.uiExitFromStudyPushButton )
	{
		if ( QMessageBox::question( this, QStringLiteral( "SPO2" ), QStringLiteral( "The Study will be closed without save the changes.\nDo you want to continue?" ), QMessageBox::StandardButton::Yes, QMessageBox::StandardButton::No ) == QMessageBox::StandardButton::No )
		{
			return;
		}

		exitFromStudy();
	}
}

void MainForm::newStudyDeatilsListWidgetCurrentTextChaned( const QString& aCurrentText )
{
	if ( aCurrentText == "Patient" )
	{
		ui.uiNewStudyDetailsStackedWidget->setCurrentIndex( STACKED_WIDGET_NEW_STUDY_PATIENT_PAGE );
	}

	if ( aCurrentText == "Medical Status" )
	{
		ui.uiNewStudyDetailsStackedWidget->setCurrentIndex( STACKED_WIDGET_NEW_STUDY_MEDICAL_STATUS_PAGE );
	}

	if ( aCurrentText == "Diagnoses" )
	{
		ui.uiNewStudyDetailsStackedWidget->setCurrentIndex( STACKED_WIDGET_NEW_STUDY_DIAGNOSES_PAGE );
	}
}

void MainForm::clearStudyTextBoxes()
{
	ui.uiPatientMiddleNameLineEdit->clear();
	ui.uiPatientLastNameLineEdit->clear();
	ui.uiPatientFirstNameLineEdit->clear();
	ui.uiPatientIDLineEdit->clear();
	ui.uiPatientBirthDateDateEdit->setDate( QDate( 2000, 01, 01 ) );
	ui.uiPatientWeightLineEdit->clear();
	ui.uiPatientHeightLineEdit->clear();
	ui.uiPatientHistoryTextEdit->clear();
	ui.uiPatientDiagnosesTextEdit->clear();
}

void MainForm::exitFromStudy()
{
	clearStudyTextBoxes();
	ui.uiStackedWidget->setCurrentIndex( STACKED_WIDGET_FIRST_PAGE );
}

int MainForm::calculateAgeFromBirthDate( const QDate& aBirthDate )
{
	double age = QDate::currentDate().year() - aBirthDate.year();
	age += aBirthDate.addYears( age ).daysTo( QDate::currentDate() ) / 365.0;
	if ( QDate::currentDate() < aBirthDate )
	{
		age -= 1.0;
	}

	return (int)round( age );
}
