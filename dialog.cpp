#include "dialog.h"
#include "ui_dialog.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <QString>

Dialog::Dialog(QWidget *parent)
	: QDialog(parent)
	, ui(new Ui::Dialog)
	{
		ui->setupUi(this);
		ui->gercekHiz->display("-------");
		serialBuffer = "";
		parsed_data = "";
		// Başlangıç koşulları
		arduino_is_available = false;
		arduino_port_name = "";
		arduino = new QSerialPort;

		/***
		 *
		 *
		***/
		/*
		qDebug() << "Number of available ports:" << QSerialPortInfo::availablePorts().length();
		foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
				qDebug() << "Has Vendor ID:" << serialPortInfo.hasVendorIdentifier();
				if(serialPortInfo.hasVendorIdentifier()){
						qDebug() << "Vendor ID:" << serialPortInfo.vendorIdentifier();
					}
				qDebug() << "Has Product ID:" << serialPortInfo.hasProductIdentifier();
				if(serialPortInfo.hasVendorIdentifier()){
						qDebug() << "Product ID:" << serialPortInfo.productIdentifier();
						qDebug() << "_______________________________";
					}

			}
		*/
		foreach (const QSerialPortInfo &serialPortInfo, QSerialPortInfo::availablePorts()) {
				if(serialPortInfo.hasVendorIdentifier() && serialPortInfo.hasProductIdentifier()){
						if(serialPortInfo.vendorIdentifier() == arduino_vendor_id){
								if(serialPortInfo.productIdentifier() == arduino_product_id){
										arduino_port_name = serialPortInfo.portName();
										arduino_is_available = true;
									}
							}
					}
			}

		if(arduino_is_available){
				// open and configure the serialport
				arduino->setPortName(arduino_port_name);
				arduino->open(QSerialPort::ReadWrite);
				arduino->setBaudRate(QSerialPort::Baud9600);
				arduino->setDataBits(QSerialPort::Data8);
				arduino->setParity(QSerialPort::NoParity);
				arduino->setStopBits(QSerialPort::OneStop);
				arduino->setFlowControl(QSerialPort::NoFlowControl);
				QObject::connect(arduino, SIGNAL(readyRead()), this, SLOT(readSerial()));
		}else{ // give error mesage
				QMessageBox::warning(this, "Port error", "Couldn't find the Arduino" );
			}

	}

Dialog::~Dialog()
	{
		if(arduino->isOpen()){
				arduino->close();
			}
		delete ui;
	}


void Dialog::on_KP_value_valueChanged(int value)
{
	ui ->label->setText(QString("<span style=\" font-size:10pt; font-weight:600;\">KP: %1</span>").arg(value/10.0));
	Dialog::pidParameters(QString("p%1").arg(value/10.0));
	qDebug() << value/10.0;
}

void Dialog::on_KI_value_valueChanged(int value)
{
	ui ->label_2->setText(QString("<span style=\" font-size:10pt; font-weight:600;\">KI: %1</span>").arg(value/100.0));
	Dialog::pidParameters(QString("i%1").arg(value/100.0));
	qDebug() << value/100.0;
}

void Dialog::on_KD_value_valueChanged(int value)
{
	ui ->label_3->setText(QString("<span style=\" font-size:10pt; font-weight:600;\">KD: %1</span>").arg(value/100.0));
	Dialog::pidParameters(QString("d%1").arg(value/100.0));
	qDebug() << value/100.0;
}

void Dialog::on_motorHiz_valueChanged(int value)
{

	ui ->hizGosterge->display(value);
	Dialog::pidParameters(QString("m%1").arg(value));
	qDebug() << value;
}


void Dialog::pidParameters(QString komut)
{
	if(arduino -> isWritable()){
			arduino -> write(komut.toStdString().c_str());
		}else{
			qDebug() <<	"Couldn't write serial";
		}
}

void Dialog::motorHiz(QString hiz){
		if(arduino -> isWritable()){
				arduino -> write(hiz.toStdString().c_str());
			}else{}

}

void Dialog::on_saatYonu_clicked(bool checked)
{
	if(checked){
			on_saatinTersi_clicked(false);
			Dialog::motorSaatYonu(QString("y0"));
		}
}

void Dialog::on_saatinTersi_clicked(bool checked)
{
	if(checked){
			on_saatYonu_clicked(false);
			Dialog::motorSaatYonu(QString("y1"));
		}
}

void Dialog::motorSaatYonu(QString yon){
		if(arduino -> isWritable()){
				arduino -> write(yon.toStdString().c_str());
			}else{}
	}

void Dialog::readSerial(){
		/*
		 * readyRead() doesn't guarantee that the entire message will be received all at once.
		 * The message can arrive split into parts.  Need to buffer the serial data and then parse for the temperature value.
		 *
		 */
		QStringList buffer_split = serialBuffer.split(","); //  split the serialBuffer string, parsing with ',' as the separator

		//  Check to see if there less than 3 tokens in buffer_split.
		//  If there are at least 3 then this means there were 2 commas,
		//  means there is a parsed temperature value as the second token (between 2 commas)
		if(buffer_split.length() < 3){
			// no parsed value yet so continue accumulating bytes from serial in the buffer.
			serialData = arduino->readAll();
			serialBuffer = serialBuffer + QString::fromStdString(serialData.toStdString());

			serialData.clear();
		}else{
			// the second element of buffer_split is parsed correctly, update the temperature value on temp_lcdNumber
			serialBuffer = "";
			qDebug() << buffer_split << "\n";
			parsed_data = buffer_split[1];
			realRPM = parsed_data.toDouble(); // convert to fahrenheit
			qDebug() << "RPM: " << realRPM << "\n";
			parsed_data = QString::number(realRPM, 'g', 4); // format precision of temperature_value to 4 digits or fewer
			Dialog::displayRPM(parsed_data);
		}

	}
void Dialog::displayRPM(QString value)
{
		ui ->gercekHiz->display(value);
}
