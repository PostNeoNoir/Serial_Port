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
//Egor check pls
        //Egor check pls
            //Egor check pls
                //Egor check pls
    if (this->serial->bytesAvailable() < Count_Wait_Byte)return;

    QByteArray tmp_data = this->serial->read(Count_Wait_Byte - 4);

    QDataStream s(&tmp_data, QIODevice::ReadOnly);
    s.setByteOrder(QDataStream::LittleEndian);

    Message message;

    switch(message.fromRaw(tmp_data)){
        case 0:{
            QByteArray tmp_data = this->serial->read(4);
            message.GetId(tmp_data);
        break;}

        case -1:{
            qDebug()<<"Incorrect preambul ";
        break;}

        case -2:{
            qDebug()<<"Incorrect command ";
        break;}

    }
}

void MainWindow::slWrite(void){

    //Command PING    ->    Command::PING
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

//Egor check pls
        //Egor check pls
            //Egor check pls
                //Egor check pls

    Count_Wait_Byte = Size::AWAIT_PING;

    this->serial->write(tmp_data);


}
