#ifndef QGUI_H
#define QGUI_H

#include <QtGui/QMainWindow>
#include "ui_qgui.h"

class QGUI : public QMainWindow
{
	Q_OBJECT

public:
	QGUI(QWidget *parent = 0, Qt::WFlags flags = 0);
	~QGUI();

private:
	Ui::QGUIClass ui;
};

#endif // QGUI_H
