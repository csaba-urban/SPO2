#include "mainform.h"
#include "serialport.h"

#include <QtWidgets/QApplication>
#include <QtWidgets/QSplashscreen>
#include <QtCore/Qthread>
#include <QtCore/QDebug>

#include <sqldatabase.h>
#include <patient.h>

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainForm w;


	DCB dcb;
	memset( &dcb, 0, sizeof( dcb ) );
	dcb.DCBlength = sizeof( dcb );
	dcb.BaudRate = 19200;
	//dcb.fBinary = 1;
	dcb.Parity = NOPARITY;
	dcb.fParity = FALSE;
	dcb.ByteSize = 8;
	dcb.StopBits = ONESTOPBIT;

	//SerialPort serialPort;
	//bool result = serialPort.open( "\\\\.\\COM5", dcb );
	//bool result = serialPort.open( "c:\\Users\\Urban Csaba\\Documents\\SoftwareProjects\\SPO2\\sample\\out.bin", 
	//	"c:\\Users\\Urban Csaba\\Documents\\SoftwareProjects\\SPO2\\sample\\sample.bin" );

	
	//const int INPUT_DATA_BLOCK_MAX_SIZE = 5;
	//static char inpData[ INPUT_DATA_BLOCK_MAX_SIZE ];

	//unsigned char* packetData;
	//int inpDataSize, packetDataLen;

	//while ( true )
	//{
	//	inpDataSize = serialPort.read( inpData, INPUT_DATA_BLOCK_MAX_SIZE );
	//	
	//	qDebug() << "-----------------";
	//	qDebug() << "Waveform: " << (int)inpData[ 1 ];
	//	qDebug() << "HR:       " << (int)inpData[ 3 ];
	//	qDebug() << "O2:       " << (int)inpData[ 4 ];
	//	qDebug() << "-----------------";
	//}

	SQLDatabase db;

	bool res = db.open( "c:\\Users\\Urban Csaba\\Documents\\SoftwareProjects\\SPO2\\database\\spo2_database.sqlite" );

	Patient p;
	db.getPatientByName("a", "a", "a",p );

	db.deletePatient( "1" );

	QList<Patient> pp;
	db.getAllPatient( pp );

	QPixmap pixmap( "cms50_splash.jpg" );
	QSplashScreen splashScreen( pixmap );
	
	splashScreen.show();
	a.processEvents();
	//QThread::sleep( 3 );
	
	w.show();
	splashScreen.finish( &w );

	return a.exec();
}
