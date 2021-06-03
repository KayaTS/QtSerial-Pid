#include "dialog.h"
#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>

int main(int argc, char *argv[])
	{
		QApplication a(argc, argv);
		Dialog w;
		w.setWindowTitle("Dc Motor Kontrol");
		w.show();
		return a.exec();
	}
