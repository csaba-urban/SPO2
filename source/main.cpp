#include "mainform.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QSplashscreen>
#include <QtCore/Qthread>


int main(int argc, char *argv[])
{
	QApplication a(argc, argv);
	MainForm w;

	QPixmap pixmap( "cms50_splash.jpg" );
	QSplashScreen splashScreen( pixmap );
	
	splashScreen.show();
	a.processEvents();
	//QThread::sleep( 3 );
	
	w.show();
	splashScreen.finish( &w );

	return a.exec();
}
