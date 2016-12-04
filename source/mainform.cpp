#include "mainform.h"

#undef max
#undef min
#include "qcustomplot.h"

#include <Log.h>
#include <QMessageBox>

LOG_INIT( "SPO2.mainform" );

MainForm::MainForm( QMainWindow* aParent )
: QMainWindow( aParent )
, mDatabase( nullptr )
, mSpoTrendQCustomPlot( new QCustomPlot( this ) )
, mHeartRateTrendQCustomPlot( new QCustomPlot( this ) )
{
	ui.setupUi(this);
	setupSpoAndHeartRateCustomPlot();

	ui.uiStackedWidget->setCurrentIndex( STACKED_WIDGET_FIRST_PAGE );

	connect( ui.uiNewStudyPushButton, &QPushButton::released, this, &MainForm::pushButtonClicked );
	connect( ui.uiExitFromStudyPushButton, &QPushButton::released, this, &MainForm::pushButtonClicked );
	connect( ui.uiNewStudyDeatilsListWidget, &QListWidget::currentTextChanged, this, &MainForm::newStudyDeatilsListWidgetCurrentTextChaned );
	connect( ui.uiEmergencyPushButton, &QPushButton::released, this, &MainForm::pushButtonClicked );
	connect( ui.uiSaveAndAcquireStudyPushButton, &QPushButton::released, this, &MainForm::pushButtonClicked );
	connect( ui.uiStartPushButton, &QPushButton::released, this, &MainForm::pushButtonClicked );
	connect( ui.uiStopPushButton, &QPushButton::released, this, &MainForm::pushButtonClicked );
	connect( ui.uiPatientBirthDateDateEdit, &QDateEdit::dateChanged, this, [=] ( const QDate& aDate )
	{
		ui.uiPatientAgeLineEdit->setText( QString::number( calculateAgeFromBirthDate( aDate ) ) );
	} );

}

MainForm::~MainForm()
{
	if ( mSpoTrendQCustomPlot != nullptr )
	{
		delete mSpoTrendQCustomPlot;
	}
}

void MainForm::setDatabase( SQLDatabase& aDatabase )
{
	mDatabase = &aDatabase;
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

	//Emergency button pushed
	if ( sender() == ui.uiEmergencyPushButton )
	{
		Patient patient( getEmergencyPatient() );

		if ( !mDatabase->insertPatient( patient ) )
		{
			QString error( mDatabase->lastError() );
			LOG_ERROR( error.toStdString().c_str() );
			QMessageBox::critical( this, "SPO2", error );
			return;
		}
		showStatusBarMessage( "Emergency Patient saved!" );
		doAcquisition( patient );
	}

	//Save & Acquire Study pushed
	if ( sender() == ui.uiSaveAndAcquireStudyPushButton )
	{
		Patient patient( getPatient() );

		if ( !mDatabase->insertPatient( patient ) )
		{
			QString error( mDatabase->lastError() );
			LOG_ERROR( error.toStdString().c_str() );
			QMessageBox::critical( this, "SPO2", error );
			return;
		}
		showStatusBarMessage( "Patient saved!" );
		doAcquisition( patient );
	}

	//Start pushed
	if ( sender() == ui.uiStartPushButton )
	{
		startSession();
	}


	//Stop pushed
	if ( sender() == ui.uiStopPushButton )
	{
		endSession();
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

void MainForm::dataAvailableOnSerialPort()
{
	static QTime time( QTime::currentTime() );

	const int INPUT_DATA_BLOCK_MAX_SIZE = 5;
	static char inpData[ INPUT_DATA_BLOCK_MAX_SIZE ];

	unsigned char* packetData;
	int packetDataLen;

	if ( mSerialPort.read( inpData, INPUT_DATA_BLOCK_MAX_SIZE ) == 0 )
	{
		return;
	}

	//write serial data to the file via this stream object
	int size = mDataFileStream.writeRawData( inpData, INPUT_DATA_BLOCK_MAX_SIZE );

	int HR = (int)inpData[ 3 ];
	int SPO2 = (int)inpData[ 4 ];

	//update labels
	setHRValue( HR );
	setSPO2Value( SPO2 );

	//update the Min/Max HR and SPO2 labels
	updateMinMaxValues( HR, SPO2 );

	//update trends
	// calculate two new data points:
	double key = time.elapsed() / 1000.0; // time elapsed since start of demo, in seconds
	static double lastPointKey = 0;
	if ( key - lastPointKey > 1 ) // at most add point every 2 ms
	{
		//SPO2 trend
		// add data to lines:
		mSpoTrendQCustomPlot->graph( 0 )->addData( key, SPO2 );
		lastPointKey = key;

		// make key axis range scroll with the data (at a constant range size of 8):
		mSpoTrendQCustomPlot->xAxis->setRange( key, 8, Qt::AlignRight );
		mSpoTrendQCustomPlot->replot();

		//HR trend
		mHeartRateTrendQCustomPlot->graph( 0 )->addData( key, HR );
		lastPointKey = key;

		// make key axis range scroll with the data (at a constant range size of 8):
		mHeartRateTrendQCustomPlot->xAxis->setRange( key, 8, Qt::AlignRight );
		mHeartRateTrendQCustomPlot->replot();
	}
	

	//qDebug() << "-----------------";
	qDebug() << "Waveform: " << (int)inpData[ 1 ];
	//qDebug() << "HR:       " << (int)inpData[ 3 ];
	//qDebug() << "O2:       " << (int)inpData[ 4 ];
	//qDebug() << "-----------------";
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

Patient MainForm::getPatient()
{
	Patient patient;
	patient.setFirstName( ui.uiPatientFirstNameLineEdit->text() );
	patient.setMiddleName( ui.uiPatientMiddleNameLineEdit->text() );
	patient.setLastName( ui.uiPatientLastNameLineEdit->text() );
	patient.setGenderName( ui.uiPatientGenderComboBox->currentText() );
	patient.setPatientID( ui.uiPatientIDLineEdit->text() );
	patient.setBirthDate( ui.uiPatientBirthDateDateEdit->date() );

	return patient;
}

Study MainForm::getStudy( const QByteArray& aData )
{
	Study study;
	Patient patient( getPatient() );

	study.setPatientID( patient.patientID() );
	study.setStudyDateTime( QDateTime::currentDateTime() );
	study.setPatientWeight( ui.uiPatientWeightLineEdit->text().toInt() );
	study.setPatientHeight( ui.uiPatientHeightLineEdit->text().toInt() );
	study.setPatientSmokingStatusAsStr( ui.uiSmokingStatusComboBox->currentText() );
	study.setPatientHistory( ui.uiPatientHistoryTextEdit->toPlainText() );
	study.setDiagnose( ui.uiPatientDiagnosesTextEdit->toPlainText() );
	study.setSPO2Record( aData );
	
	return study;
}

void MainForm::doAcquisition( const Patient& aPatient )
{
	ui.uiStackedWidget->setCurrentIndex( STACKED_WIDGET_ACQUISITION_PAGE );
	
	//setup the Patient info labels
	ui.uiAcqPatientNameValueLabel->setText( aPatient.name() );
	ui.uiAcqBirthDateValueLabel->setText( aPatient.birthDate().toString( "yyyy.MM.dd" ) );
	ui.uiAcqGenderValueLabel->setText( aPatient.genderString() );
}

void MainForm::setHRValue( int aHR )
{
	ui.uiHRValueLabel->setText( QString::number( aHR ) );
}

void MainForm::setSPO2Value( int aSPO2 )
{
	ui.uiSPO2ValueLabel->setText( QString::number( aSPO2 ) );
}

void MainForm::setupSpoAndHeartRateCustomPlot()
{
	//setup Spo trend
	mSpoTrendQCustomPlot->addGraph(); // blue line
	mSpoTrendQCustomPlot->graph( 0 )->setPen( QPen( QColor( 40, 110, 255 ) ) );

	QSharedPointer<QCPAxisTickerTime> timeTicker( new QCPAxisTickerTime );
	timeTicker->setTimeFormat( "%h:%m:%s" );
	mSpoTrendQCustomPlot->xAxis->setTicker( timeTicker );
	//mSpoTrendQCustomPlot->axisRect()->setupFullAxesBox();
	mSpoTrendQCustomPlot->yAxis->setRange( 70, 100 );

	// make left and bottom axes transfer their ranges to right and top axes:
	//connect( mSpoTrendQCustomPlot->xAxis, SIGNAL( rangeChanged( QCPRange ) ), mSpoTrendQCustomPlot->xAxis2, SLOT( setRange( QCPRange ) ) );
	//connect( mSpoTrendQCustomPlot->yAxis, SIGNAL( rangeChanged( QCPRange ) ), mSpoTrendQCustomPlot->yAxis2, SLOT( setRange( QCPRange ) ) );
	
	ui.uiSpoTrendGroupBox->layout()->addWidget( mSpoTrendQCustomPlot );

	//setup HR trend
	mHeartRateTrendQCustomPlot->addGraph(); // red line
	mHeartRateTrendQCustomPlot->graph( 0 )->setPen( QPen( QColor( 255, 110, 40 ) ) );

	mHeartRateTrendQCustomPlot->xAxis->setTicker( timeTicker );
	//mHeartRateTrendQCustomPlot->axisRect()->setupFullAxesBox();
	mHeartRateTrendQCustomPlot->yAxis->setRange( 0, 150 );

	ui.uiHeartRateTrendGroupBox->layout()->addWidget( mHeartRateTrendQCustomPlot );
}

void MainForm::updateMinMaxValues( int aHR, int aSPO2 )
{
	//init
	static int minHR = 200;
	static int maxHR = aHR;
	static int minSPO2 = 100;
	static int maxSPO2 = aSPO2;

	static QString minMaxHR( "%1 / %2" );
	static QString minMaxSPO( "%1 / %2" );

	if ( aHR > 0 )
	{
		minHR = std::min( minHR, aHR );
	}

	maxHR = std::max( maxHR, aHR );
	
	if ( aSPO2 > 0 )
	{
		minSPO2 = std::min( minSPO2, aSPO2 );
	}
	
	maxSPO2 = std::max( maxSPO2, aSPO2 );

	ui.uiSPOMinMaxValueLabel->setText( minMaxSPO.arg( QString::number( minSPO2 ), QString::number( maxSPO2 ) ) );
	ui.uiHRMinMaxValueLabel->setText( minMaxHR.arg( QString::number( minHR ), QString::number( maxHR ) ) );

	//autoscale
	//mSpoTrendQCustomPlot->yAxis->setRange( minSPO2-5, 100 );
	//mHeartRateTrendQCustomPlot->yAxis->setRange( minHR - 10, maxHR+10 );
}

Patient MainForm::getEmergencyPatient()
{
	Patient p;
	p.setFirstName( QString( "Emergency_%1" ).arg( QDateTime::currentDateTime().toString( "yyyyMMddMMmmss" ) ) );
	p.setBirthDate( QDate::currentDate() );
	p.setGenderName( Patient::EGender::EOther );

	return p;
}

void MainForm::showStatusBarMessage( const QString& aMessage )
{
	ui.statusBar->showMessage( aMessage, 5000 );
}

bool MainForm::startSession()
{
	//TODO
	DCB dcb;
	memset( &dcb, 0, sizeof( dcb ) );
	dcb.DCBlength = sizeof( dcb );
	dcb.BaudRate = 19200;
	//dcb.fBinary = 1;
	dcb.Parity = NOPARITY;
	dcb.fParity = FALSE;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;

	//create temp data file
	QString dataPath( QApplication::applicationDirPath() + "\\deviceData.dat" );
	mDataFile.setFileName( dataPath );
	if ( !mDataFile.open( QIODevice::ReadWrite ) )
	{
		QString error( QString( "Failed to create temporary data file (%1). Reason: %2" ).arg( mDataFile.errorString() ) );
		LOG_ERROR( error.toStdString().c_str() );
		QMessageBox::critical( this, "SPO2", error );
		return false;
	}
	
	mDataFileStream.setDevice( &mDataFile );

	//try to open the serial port
	if ( !mSerialPort.open( "\\\\.\\COM5", dcb ) )
	{
		QString error( QString( "Failed to open the serial port. Reason: %1" ).arg( mSerialPort.getLastError() ) );
		LOG_ERROR( error.toStdString().c_str() );
		QMessageBox::critical( this, "SPO2", error );
		return false;
	}

	showStatusBarMessage( "Device connected! Session begin..." );
	connect( &mSerialPort, &SerialPort::dataAvailable, this, &MainForm::dataAvailableOnSerialPort );

	return true;
}

bool MainForm::endSession()
{
	mSerialPort.close();
	showStatusBarMessage( "Device disconnected! Session end..." );

	QByteArray data( mDataFile.readAll() );
	
	Study study( getStudy( data ) );

	if ( !mDatabase->insertStudy( study ) )
	{
		QString error( mDatabase->lastError() );
		LOG_ERROR( error.toStdString().c_str() );
		QMessageBox::critical( this, "SPO2", error );
	}
	else
	{
		QMessageBox::information( this, "SPO2", "Study successfully saved into the database." );
	}

	//delete the data file
	mDataFile.close();
	mDataFile.remove();

	return true;
}