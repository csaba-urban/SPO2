#ifndef MAINFORM_H
#define MAINFORM_H

#include <QtWidgets/qmainwindow.h>
#include "ui_mainform.h"

#define STACKED_WIDGET_FIRST_PAGE		0
#define STACKED_WIDGET_NEW_STUDY_PAGE	1
#define STACKED_WIDGET_ACQUISITION_PAGE	2

#define STACKED_WIDGET_NEW_STUDY_PATIENT_PAGE			0
#define STACKED_WIDGET_NEW_STUDY_MEDICAL_STATUS_PAGE	1
#define STACKED_WIDGET_NEW_STUDY_DIAGNOSES_PAGE			2


class MainForm : public QMainWindow
{
	Q_OBJECT

public:
	MainForm( QMainWindow* aParent = 0 );
	~MainForm();

public slots:
	void pushButtonClicked();
	void newStudyDeatilsListWidgetCurrentTextChaned( const QString& aCurrentText );

private:
	Ui::MainForm ui;

	/*
		clears the textboxes on the Patient, Medical Status and Diagnoses pages
	*/
	void clearStudyTextBoxes();

	/*
		exit from study, all entered patient informations will be lost from the UI
	*/
	void exitFromStudy();

	/*
		calculates the age from the given birthdate
	*/
	int calculateAgeFromBirthDate( const QDate& aBirthDate );
};

#endif // MAINFORM_H
