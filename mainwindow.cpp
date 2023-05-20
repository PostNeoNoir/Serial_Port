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
#include <QFile>


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
            message.Get_Program_ID(tmp_data);
        break;}

        //case JUMP
        case 2:{
            QByteArray tmp_data = this->serial->read(ID_SIZE);
        break;}

        //case VERIFY
        case 3:{
            QByteArray tmp_data = this->serial->read(ID_SIZE + NUMBER_PROGRAMM_SIZE);
        break;}

        //case ERASE
        case 5:{
            if (!(this->Flashing_Flag)){
                       break;
            }
            else{
                this->Flashing_Flag = 0;
                QFile file("C:/QtProject/Serial_Port_Egor/file.txt");
                file.open(QFile::ReadOnly);
            }

            [[fallthrough]];
              }

        //case WRITE
        case 4:{
            char chunk[CHUNK_SIZE_WORDS*4];
            //fix file ->
            uint32_t chunk_size = file.read(chunk, CHUNK_SIZE_WORDS*4);

            if(chunk_size){
                Message(WRITE, 3 + chunk_size/4);

                //payload

                for (uint32_t iter = 0; iter < chunk_size; iter += 4){
                    //payload
                    //Message.payload.append(*((uint32_t *)(chunk + iter)));
                }
                Count_Wait_Byte = Size::AWAIT_WRITE_SIZE;

                tmp_data = Message(WRITE, DEFAULT_COMMAND_REQUEST_SIZE + chunk_size/4, 0x00000001).toRaw();//+firm content
                this->serial->write(tmp_data);

            }
            else{
                //fix file ->
                file.close();
                file.remove();
            }

        break;}

        //case READ
        case 6:{
            //QByteArray tmp_data = this->serial->read(ID_SIZE + NUMBER_PROGRAMM_SIZE + 512/4);
            //message.Get_Program_ID(tmp_data);
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

    QFile file("C:/QtProject/Serial_Port_Egor/file.txt");

    if(!(file.open(QFile::ReadOnly))){
        file.remove();
        return;
    }

    if((file.size() > FLASH_MAP_APP_1) || (file.size() > FLASH_MAP_APP_2) || (file.size() > FLASH_MAP_APP_USER)){
        qDebug()<<"Size Error";
        file.close();
        file.remove();
        return;

    }

    file.close();
    file.remove();

    Flashing_Flag = 1;

    slErase();

}

void MainWindow::slErase(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Count_Wait_Byte = Size::AWAIT_ERASE_SIZE;

    tmp_data = Message(ERASE, DEFAULT_COMMAND_REQUEST_SIZE, 0x00000001).toRaw();

    //Message.payload.append();

    this->serial->write(tmp_data);
}

void MainWindow::slRead(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Count_Wait_Byte = Size::AWAIT_READ_SIZE;

    tmp_data = Message(READ, DEFAULT_COMMAND_REQUEST_SIZE, 0x00000001).toRaw();

    this->serial->write(tmp_data);
}
