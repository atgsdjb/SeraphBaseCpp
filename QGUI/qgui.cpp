#include "qgui.h"

QGUI::QGUI(QWidget *parent, Qt::WFlags flags)
	: QMainWindow(parent, flags)
{
	ui.setupUi(this);
}

QGUI::~QGUI()
{

}
