#ifndef MAINFORM_H
#define MAINFORM_H

#include <QtWidgets/qmainwindow.h>

#include "ui_mainform.h"
#include "patient.h"
#include "sqldatabase.h"
#include "serialport.h"

#define STACKED_WIDGET_FIRST_PAGE		0
#define STACKED_WIDGET_NEW_STUDY_PAGE	1
#define STACKED_WIDGET_ACQUISITION_PAGE	2

#define STACKED_WIDGET_NEW_STUDY_PATIENT_PAGE			0
#define STACKED_WIDGET_NEW_STUDY_MEDICAL_STATUS_PAGE	1
#define STACKED_WIDGET_NEW_STUDY_DIAGNOSES_PAGE			2

class QCustomPlot;

class MainForm : public QMainWindow
{
	Q_OBJECT

public:
	MainForm( QMainWindow* aParent = 0 );
	~MainForm();

	void setDatabase( SQLDatabase& aDatabase );

public slots:
	void pushButtonClicked();
	void newStudyDeatilsListWidgetCurrentTextChaned( const QString& aCurrentText );
	void dataAvailableOnSerialPort();

private:
	Ui::MainForm ui;
	SQLDatabase* mDatabase;
	SerialPort mSerialPort;

	QCustomPlot* mSpoTrendQCustomPlot;
	QCustomPlot* mHeartRateTrendQCustomPlot;

	QFile mDataFile;
	QDataStream mDataFileStream; //this stream will be used for storing the incoming data from the serial port

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

	/*
		collects the patient infos from the UI
	*/
	Patient getPatient();
	
	/*
		collects the study infos from the UI and the related objects
	*/
	Study getStudy( const QByteArray& aData );

	/*
		starts the acquisition process
	*/
	void doAcquisition( const Patient& aPatient );

	void setHRValue( int aHR );
	void setSPO2Value( int aSPO2 );
	void setupSpoAndHeartRateCustomPlot();
	void updateMinMaxValues( int aHR, int aSPO2 );

	Patient getEmergencyPatient();


	void showStatusBarMessage( const QString& aMessage );

	/*
		starts and ends a study session
	*/
	bool startSession();
	bool endSession();
};

#endif // MAINFORM_H
