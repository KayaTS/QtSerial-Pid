#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QSerialPort>

QT_BEGIN_NAMESPACE
namespace Ui { class Dialog; }
QT_END_NAMESPACE

class Dialog : public QDialog
{
	Q_OBJECT

public:
	Dialog(QWidget *parent = nullptr);
	~Dialog();

private slots:
	void on_KP_value_valueChanged(int value);

	void on_KI_value_valueChanged(int value);

	void on_KD_value_valueChanged(int value);

	void on_motorHiz_valueChanged(int value);

	void Dialog::pidParameters(QString);

	void Dialog::motorHiz(QString);

	void on_saatYonu_clicked(bool);

	void on_saatinTersi_clicked(bool);

	void Dialog::motorSaatYonu(QString yon);

	void readSerial();

	void displayRPM(QString value);

private:
	Ui::Dialog *ui;
	QSerialPort *arduino;
	static const quint16 arduino_vendor_id = 6790;
	static const quint16 arduino_product_id = 29987;
	QByteArray serialData;
	QString serialBuffer;
	QString parsed_data;
	QString	displayData;
	double realRPM;
	QString arduino_port_name;
	bool arduino_is_available;
};
#endif // DIALOG_H
