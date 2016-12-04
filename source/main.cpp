#include "mainform.h"


#include <QtWidgets/QApplication>
#include <QtWidgets/QSplashscreen>
#include <QtCore/Qthread>
#include <QtCore/QDebug>
#include <QMessageBox>
#include <QDir>

#include <sqldatabase.h>
#include <patient.h>
#include <Log.h>

#include <shlobj.h>
#include <iostream>

LOG_INIT( "SPO2.main" );

bool getLogFolder( const QString& aSoftwareName, QString& aLogFolder, wchar_t* aError )
{
	WCHAR commonApplicationDataFolder[ MAX_PATH + 1 ] = { 0 };
	if ( !SUCCEEDED( SHGetFolderPath( 0, CSIDL_COMMON_APPDATA, 0, SHGFP_TYPE_CURRENT, commonApplicationDataFolder ) ) )
	{
		aError = L"Cannot get 'COMMON_APPDATA' folder location.";
		std::wcerr << aError << std::endl;
		return false;
	}

	aLogFolder = QString::fromStdWString( std::wstring( commonApplicationDataFolder ) + L"/" + aSoftwareName.toStdWString() + L"/Log" );
	return true;
}

void createLogFolder()
{
	wchar_t* error = nullptr;
	QString logFolder;
	if ( !getLogFolder( "SPO2", logFolder, error ) )
	{
		qDebug() << "Failed to get log folder. Reason: " << error;
	}

	QDir dir;
	if ( !dir.mkpath( logFolder ) )
	{
		qDebug() << "Failed to create log folder path: ." << logFolder;
	}
}

int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainForm w;

	createLogFolder();

	QString configPath( QApplication::applicationDirPath() + "\\logConfig.xml" );
	Log::configure( configPath.toStdString().c_str() );

	LOG_INFO( "SPO2 started." );

	SQLDatabase db;

	if ( !db.open( QApplication::applicationDirPath() + "\\spo2_database.sqlite" ) )
	{
		QString error( QString( "Failed to open the database. Reason: %1" ).arg( db.lastError() ) );
		LOG_ERROR( error.toStdString().c_str() );
		QMessageBox::critical( nullptr, "SPO2", error );
	}

	w.setDatabase( db );

	QPixmap pixmap( "cms50_splash.jpg" );
	QSplashScreen splashScreen( pixmap );
	
	splashScreen.show();
	a.processEvents();
	//QThread::sleep( 3 );
	
	w.show();
	splashScreen.finish( &w );

	int retVal = a.exec();
	LOG_INFO( "SPO2 exited." );
	return retVal;
}
