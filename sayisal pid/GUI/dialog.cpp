#include "dialog.h"
#include "ui_dialog.h"
#include <QSerialPort>
#include <QSerialPortInfo>
#include <QDebug>
#include <QtWidgets>
#include <QtCharts/QSplineSeries>
#include <QString>
#include <QtCharts/QValueAxis>
#include <QElapsedTimer>
#include <QTime>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QApplication>
#include <QtCharts/QChartView>
#include <QtCharts/QLineSeries>
#include <QtWidgets/QLabel>

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

		//grafik icin time ayarlamalari

		timer.start();
		QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
		series->append(0, 0);
		/***
		 *
		***/
		chart->legend()->hide();
		chart->createDefaultAxes();
		chart->setTitle("Motor Hizi");
		chartView->chart()->setTheme(QtCharts::QChart::ChartThemeBrownSand);
		ui->verticalLayout_3->addWidget(chartView);

		QPalette pal = qApp->palette();
		pal.setColor(QPalette::Window, QRgb(0x40434a));
		pal.setColor(QPalette::WindowText, QRgb(0xd6d6d6));
		qApp->setPalette(pal);

		/** İlk ayar

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
		**/
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
	ui ->label->setText(QString("<span style=\" font-size:10pt; font-weight:600;\">KP: %1</span>").arg(value/100.0));
	Dialog::pidParameters(QString("p%1").arg(value/100.0));
}

void Dialog::on_KI_value_valueChanged(int value)
{
	ui ->label_2->setText(QString("<span style=\" font-size:10pt; font-weight:600;\">KI: %1</span>").arg(value/100.0));
	Dialog::pidParameters(QString("i%1").arg(value/100.0));
}

void Dialog::on_KD_value_valueChanged(int value)
{
	ui ->label_3->setText(QString("<span style=\" font-size:10pt; font-weight:600;\">KD: %1</span>").arg(value/100.0));
	Dialog::pidParameters(QString("d%1").arg(value/100.0));
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

		QStringList buffer_split = serialBuffer.split(","); //  split the serialBuffer string, parsing with ',' as the separator

		if(buffer_split.length() < 3){

			serialData = arduino->readAll();
			serialBuffer = serialBuffer + QString::fromStdString(serialData.toStdString());

			serialData.clear();
		}else{

			serialBuffer = "";
			parsed_data = buffer_split[1];
			realRPM = parsed_data.toDouble();
			parsed_data = QString::number(realRPM, 'g', 4);
			Dialog::displayRPM(parsed_data);
			Dialog::displayChart(realRPM);
		}

	}
void Dialog::displayRPM(QString value)
{
		ui ->gercekHiz->display(value);
}

void Dialog::displayChart(double rpm)
{
		QtCharts::QLineSeries *series = new QtCharts::QLineSeries(chart);
		series->append(timer.elapsed()/100, rpm);
		*series << QPointF(timer.elapsed()/100, rpm);
		//QtCharts::QChart *chart = new QtCharts::QChart();
		chart->addSeries(series);
		chart->createDefaultAxes();
		chart->axes(Qt::Horizontal).first()->setRange(0, timer.elapsed()/100 + 10);
		chart->axes(Qt::Vertical).first()->setRange(0, 1500);
		QtCharts::QValueAxis *axisY = qobject_cast<QtCharts::QValueAxis*>(chart->axes(Qt::Vertical).first());
		Q_ASSERT(axisY);
		axisY->setLabelFormat("%.1f  ");
		//QtCharts::QChartView *chartView = new QtCharts::QChartView(chart);
		//chartView = new QtCharts::QChartView(lineChart());

		chartView->setRenderHint(QPainter::Antialiasing);
		ui->verticalLayout_3->addWidget(chartView);

}

QtCharts::QChart *lineChart(){

		//![1]
			QtCharts::QLineSeries *series = new QtCharts::QLineSeries();
		//![1]
		//![2]
			series->append(0, 8);
			series->append(0, 8);
			series->append(3, 8);
			series->append(7, 4);
			series->append(10, 5);
			*series << QPointF(11, 1) << QPointF(13, 3) << QPointF(17, 6) << QPointF(18, 3) << QPointF(20, 2);
		//![2]

		//![3]
			QtCharts::QChart *chart = new QtCharts::QChart();
			chart->legend()->hide();
			chart->addSeries(series);
			chart->createDefaultAxes();
			chart->setTitle("Simple line chart example");
		//![3]

			return chart;
		//![4]
			//QtCharts::QChartView *chartView = new QtCharts::QChartView(chart);
			//chartView->setRenderHint(QPainter::Antialiasing);
		//![4]


		//![5]

		//![5]

	}

