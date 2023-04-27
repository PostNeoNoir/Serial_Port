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

    connect(ui->Connect_Button, &QPushButton::released, this, &MainWindow::slConnect);
    connect(ui->Ping_Button, &QPushButton::released, this, &MainWindow::slPing);
    connect(ui->Jump_Button, &QPushButton::released, this, &MainWindow::slJump);
    connect(ui->Verify_Button, &QPushButton::released, this, &MainWindow::slVerify);
    connect(ui->Write_Button, &QPushButton::released, this, &MainWindow::slWrite);
    connect(ui->Write_Button, &QPushButton::released, this, &MainWindow::slErase);
    connect(ui->Write_Button, &QPushButton::released, this, &MainWindow::slRead);

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

    if (this->serial->bytesAvailable() < Count_Wait_Byte)return;

    QByteArray tmp_data = this->serial->read(Count_Wait_Byte - ID_SIZE);

    QDataStream s(&tmp_data, QIODevice::ReadOnly);
    s.setByteOrder(QDataStream::LittleEndian);

    Message message;

    switch(message.fromRaw(tmp_data)){

        //case PING (case PING:)
        case 1:{
            QByteArray tmp_data = this->serial->read(ID_SIZE);
            message.GetId(tmp_data);
        break;}

        //case JUMP
        case 2:{
            QByteArray tmp_data = this->serial->read(ID_SIZE);
        break;}

        //case VERIFY
        case 3:{
            QByteArray tmp_data = this->serial->read(ID_SIZE + NUMBER_PROGRAMM_SIZE);
        break;}

        //case WRITE
        case 4:{
            QByteArray tmp_data = this->serial->read(ID_SIZE);
            //message.Get_Checksum_Value();       message.GetId(tmp_data);

        break;}

        //case ERASE
        case 5:{
            QByteArray tmp_data = this->serial->read(ID_SIZE);
            message.GetId(tmp_data);
        break;}

        //case READ
        case 6:{
            QByteArray tmp_data = this->serial->read(ID_SIZE + NUMBER_PROGRAMM_SIZE + 512/4);
            message.GetId(tmp_data);
        break;}

//ERROR BLOCK
        case 0:{
            qDebug()<<"Command not found ";
        break;}

        case -1:{
            qDebug()<<"Incorrect preambul ";
        break;}

        case -2:{
            qDebug()<<"Incorrect size ";
        break;}

    }
}

//BUTTON PRESS PROCESSING
void MainWindow::slPing(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Count_Wait_Byte = Size::AWAIT_PING_SIZE;

    tmp_data = Message(PING, DEFAULT_COMMAND_REQUEST_SIZE).toRaw();

    this->serial->write(tmp_data);
}

void MainWindow::slJump(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Count_Wait_Byte = Size::AWAIT_JUMP_SIZE;

    tmp_data = Message(JUMP, DEFAULT_COMMAND_REQUEST_SIZE, 0x00000001).toRaw();

    this->serial->write(tmp_data);
}

void MainWindow::slVerify(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Count_Wait_Byte = Size::AWAIT_VERIFY_SIZE;

    tmp_data = Message(VERIFY, DEFAULT_COMMAND_REQUEST_SIZE, 0x00000001).toRaw();

    this->serial->write(tmp_data);
}

void MainWindow::slWrite(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Count_Wait_Byte = Size::AWAIT_WRITE_SIZE;

    //tmp_data = Message(WRITE, 1+0, 0x00000001, 0, array?????).toRaw();

    this->serial->write(tmp_data);
}

void MainWindow::slErase(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Count_Wait_Byte = Size::AWAIT_ERASE_SIZE;

    tmp_data = Message(ERASE, DEFAULT_COMMAND_REQUEST_SIZE, 0x00000001).toRaw();

    this->serial->write(tmp_data);
}

void MainWindow::slRead(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Count_Wait_Byte = Size::AWAIT_READ_SIZE;

    tmp_data = Message(READ, DEFAULT_COMMAND_REQUEST_SIZE, 0x00000001).toRaw();

    this->serial->write(tmp_data);
}
