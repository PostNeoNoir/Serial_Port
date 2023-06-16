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
#include <QFileDialog>
#include <iostream>


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
    connect(ui->Erase_Button, &QPushButton::released, this, &MainWindow::slErase);
    connect(ui->Read_Button, &QPushButton::released, this, &MainWindow::slRead);

    connect(ui->BrowseFile_Button, &QPushButton::released, this, &MainWindow::BrowseFile);

    ui->comboBox_App->addItem("App_0");
    ui->comboBox_App->addItem("App_1");
    ui->comboBox_App->addItem("App_2");

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

    QByteArray tmp_data = this->serial->read(7);//only preamb, cmd, size

    QDataStream s(&tmp_data, QIODevice::ReadOnly);
    s.setByteOrder(QDataStream::LittleEndian);

    Message message;

    //Debug
    //qDebug()<<"What came (Answer) "<< hex << tmp_data;

    switch(message.fromRaw(tmp_data)){

        case Command::PING:{
            qDebug()<<"Ping done";
            QByteArray tmp_data = this->serial->read(NUMBER_ID_SIZE);
            message.Get_Program_ID(tmp_data);
        break;}

        case Command::JUMP:{
            qDebug()<<"Jump done ";
        break;}

        case Command::VERIFY:{
            qDebug()<<"Verify done";
        break;}

        case Command::ERASE:{
            qDebug()<<"Erasing done ";
        break;}

        case Command::WRITE:{
            qDebug()<<"Write done";
        break;}

        case Command::READ:{

            QByteArray tmp_data = this->serial->read(NUMBER_ID_SIZE);
            //message.Get_Program_ID(tmp_data);

            QByteArray tmp_data_firmware = this->serial->readAll();
            //qDebug()<<"What came "<<hex<<tmp_data_firmware;

            this->file = new QFile("C:/QtProject/Serial_Port_Egor/File.txt");
            if(!(this->file->open(QFile::WriteOnly))){
                        delete file;
                        file = nullptr;
                        return;
                    }
            //qDebug()<<"Tmp data"<<hex<<tmp_data_firmware;
            file->write(tmp_data_firmware);
            file->close();

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
//CALLING
//ENQUIRY
void MainWindow::slPing(void){

    if(!(this->serial)) return;

    Message message(PING, 0);

    Count_Wait_Byte = Size::AWAIT_PING_SIZE;

    this->serial->write(message.toRaw());
}

void MainWindow::slJump(void){

    if(!(this->serial)) return;

    Message message(JUMP,1);
    message.payload.append(ui->comboBox_App->currentIndex());

    Count_Wait_Byte = Size::AWAIT_JUMP_SIZE;

    this->serial->write(message.toRaw());
}

void MainWindow::slVerify(void){

    if(!(this->serial)) return;

    Message message(VERIFY,1);
    message.payload.append(ui->comboBox_App->currentIndex());

    Count_Wait_Byte = Size::AWAIT_VERIFY_SIZE;

    this->serial->write(message.toRaw());
}

void MainWindow::slWrite(void){

    this->file = new QFile(ui->lineEdit->text());

    if(!(this->file->open(QFile::ReadOnly))){
            delete file;
            file = nullptr;
            return;
        }

    char packer_firmware[PACKET_FIRMWARE_SIZE_WORDS*4];

    uint32_t packer_firmware_size = file->read(packer_firmware, PACKET_FIRMWARE_SIZE_WORDS*4);

    if(packer_firmware_size){
        Message message(WRITE,(4+packer_firmware_size)/4);

        message.payload.push_back(ui->comboBox_App->currentIndex());

        //qDebug()<<"packer_firmware_size"<<packer_firmware_size;

        for (uint32_t iter = 0; iter < packer_firmware_size; iter += 4){
            message.payload.push_back(*reinterpret_cast<uint32_t *>(packer_firmware + iter));
        }

        Count_Wait_Byte = Size::AWAIT_WRITE_SIZE;

        this->serial->write(message.toRaw());
    }

    else{
        qDebug()<<"Writting end!";
        file->close();
        delete file;
        file = nullptr;
    }

}


void MainWindow::slErase(void){

    if(!(this->serial)) return;

    Message message(ERASE,1);
    message.payload.append(ui->comboBox_App->currentIndex());

    Count_Wait_Byte = Size::AWAIT_ERASE_SIZE;

    this->serial->write(message.toRaw());
}

void MainWindow::slRead(void){

    if(!(this->serial)) return;

    Message message(READ,1);
    message.payload.append(ui->comboBox_App->currentIndex());

    Count_Wait_Byte = Size::AWAIT_READ_SIZE;

    this->serial->write(message.toRaw());
}

void MainWindow::BrowseFile(void){
    QString fname = QFileDialog::getOpenFileName(this,
                                                 tr("Open Firmware File"),
                                                 "C:/QtProject/Serial_Port_Egor",
                                                 tr("Binary Files (*.bin)"));
    ui->lineEdit->setText(fname);
}

