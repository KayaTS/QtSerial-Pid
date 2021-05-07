#include "dialog.h"

#include <QApplication>

int main(int argc, char *argv[])
	{
		QApplication a(argc, argv);
		Dialog w;
		w.setWindowTitle("Dc Motor Kontrol");
		w.show();
		return a.exec();
	}
