#ifndef MAINFORM_H
#define MAINFORM_H

#include <QtWidgets/qmainwindow.h>
#include "ui_mainform.h"

class MainForm : public QMainWindow
{
	Q_OBJECT

public:
	MainForm( QMainWindow* aParent = 0 );
	~MainForm();

private:
	Ui::MainForm ui;

	
public slots:
	void startNewStudy();

};

#endif // MAINFORM_H
