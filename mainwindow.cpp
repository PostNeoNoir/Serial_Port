#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtSerialPort/QSerialPort>
#include <QtSerialPort/QSerialPortInfo>
#include <QList>
#include <QDebug>
#include <QByteArray>
#include <QDataStream>
#include <QString>

#include <QPair>
#include <QIODevice>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->pushButton, &QPushButton::released, this, &MainWindow::slConnect);
    connect(ui->Button_Write, &QPushButton::released, this, &MainWindow::slWrite);

    QList<QSerialPortInfo> avail_port;
    avail_port = QSerialPortInfo::availablePorts();
    for (auto iter = avail_port.begin(); iter != avail_port.end(); iter++){
        qDebug() << iter->description() << " " << iter->portName();
        ui->comboBox->addItem(iter->portName());
    }
}

MainWindow::~MainWindow()
{
    delete ui;
}

struct Message{

    uint16_t preamb_5a, preamb_a5;
    uint8_t ping;
    uint32_t size;

    Message(void){}

    Message(uint8_t pings, uint32_t sizes){
        this->preamb_5a = 0x5a;
        this->preamb_a5 = 0xa5;
        this->ping = pings;
        this->size = sizes;
    }

    //EGOR, PLEASE CHECK THIS
    //EGOR, PLEASE CHECK THIS
    //EGOR, PLEASE CHECK THIS
    //EGOR, PLEASE CHECK THIS
    //EGOR, PLEASE CHECK THIS

    //Из сырого в норм
    int8_t toRaw(QByteArray data){
        this->preamb_5a = data.at(0);
        this->preamb_a5 = data.at(1);
        this->ping = data.at(2);
        this->size = data.at(3);

        return 0;
    }


    //Из норм в сырой
    QByteArray fromRaw(void){
        QByteArray data;
        QDataStream s(&data, QIODevice::WriteOnly);
        s.setByteOrder(QDataStream::LittleEndian);

        s << this->preamb_5a;
        s << this->preamb_a5;
        s << this->ping;
        s << this->size;


        return data;
    }
};

void MainWindow::slConnect(void){
    if (this->serial){
        this->serial->close();
        delete this->serial;
    }
    this->serial = new QSerialPort(ui->comboBox->currentText());
    this->serial->setBaudRate(57600);
    if (this->serial->open(QIODevice::ReadWrite)){
        qDebug() << "Active port is at " << this->serial->portName();
        connect(this->serial, &QSerialPort::readyRead, this, &MainWindow::slReadyRead);
    }
    else{
        qDebug() << "Port is not active";
    }
}

void MainWindow::slReadyRead(void){

    if (this->serial->bytesAvailable() < 7) return;
    //qDebug() << "READY READ";

    QByteArray tmp_data = this->serial->readAll();
    //qDebug() << "SIZE IS " << tmp_data.size();

    QDataStream s(&tmp_data, QIODevice::ReadOnly);
    s.setByteOrder(QDataStream::LittleEndian);


    //EGOR, PLEASE CHECK THIS
    //EGOR, PLEASE CHECK THIS
    //EGOR, PLEASE CHECK THIS
    //EGOR, PLEASE CHECK THIS
    //EGOR, PLEASE CHECK THIS
    uint16_t preamb;
    uint8_t cmd;
    uint32_t size;
    s >> preamb;
    s >> cmd;
    s >> size;

    qDebug()<<"Values received from the port " << tmp_data;

    if(preamb == 0x5aa5){
        qDebug()<< "ID: "<<cmd;
    }


// Через строку
//    QString value_str_cc_10 = QString::number(value,10);
//    int value_change = value_str_cc_10.toInt();
//    QString value_str_cc_16 = QString::number( value_change, 16);

//    QString hex_value = QString("%1").arg(value, value_str_cc_16.size(), 16, QLatin1Char( '0' ));


//    if(hex_value[0] == '5' and hex_value[1] == 'a' and hex_value[2] == 'a' and hex_value[3] == '5'){
//        QChar cmd_1,cmd_2;
//        cmd_1 = hex_value[4];
//        cmd_2 = hex_value[5];

//        qDebug()<<"Cmd "<<cmd_1<<cmd_2;

//        qDebug()<<"Value "<<value;
//        qDebug() << "Value hex "<< hex << value;
//        qDebug()<<"Value String hex "<< hex_value;

//    }



}

void MainWindow::slWrite(void){

    if(!(this->serial)) return;

    QByteArray tmp_data;

    QDataStream s(&tmp_data, QIODevice::WriteOnly);

    s.setByteOrder(QDataStream::LittleEndian);
    uint16_t preamb = 0x5AA5;
    s << preamb;

    s.setByteOrder(QDataStream::LittleEndian);
    uint8_t ping = 0x01;
    s << ping;

    s.setByteOrder(QDataStream::LittleEndian);
    uint32_t size = 0x00000000;
    s << size;

    this->serial->write(tmp_data);

}










