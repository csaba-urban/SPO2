#ifndef MAINFORM_H
#define MAINFORM_H

#include <QtWidgets/QWidget>
#include "ui_mainform.h"

class MainForm : public QWidget
{
	Q_OBJECT

public:
	MainForm(QWidget *parent = 0);
	~MainForm();

private:
	Ui::MainFormClass ui;
};

#endif // MAINFORM_H
