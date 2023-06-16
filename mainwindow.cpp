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

    //QByteArray tmp_data = this->serial->read(Count_Wait_Byte);

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
            //QByteArray tmp_data = this->serial->read(NUMBER_PROGRAMM_SIZE);
            //qDebug()<<"Jump after preamb, cmd, size " << hex << tmp_data <<"\n"<<"----------------------------"<<"\n";
            //message.Get_Program_ID(tmp_data);
            qDebug()<<"Jump done ";
        break;}

        //case VERIFY
        case Command::VERIFY:{
            //QByteArray tmp_data = this->serial->read(NUMBER_PROGRAMM_SIZE + CHECKSUM_VALUE);
            //QByteArray tmp_data = this->serial->readAll();
//            QByteArray tmp_data = this->serial->read(NUMBER_PROGRAMM_SIZE + CHECKSUM_VALUE);
//            qDebug()<<"Verify answer data" << hex << tmp_data<<"\n"<<"----------------------------"<<"\n";//"\xFF\xFF\xFF\xFF\xDD\xCC"
              qDebug()<<"Verify done";

        break;}

        //case ERASE
    case Command::ERASE:{
            qDebug()<<"Erasing done ";

//            qDebug()<<"Erasing doing ";
//            if (!(this->Flashing_Flag)){
//                       break;
//            }
//            else{
//                this->Flashing_Flag = 0;
//                this->file = new QFile(ui->lineEdit->text());
//                file->open(QFile::ReadOnly);

//            }

//            [[fallthrough]];
    }


        //case WRITE
    case Command::WRITE:{

        qDebug()<<"Write done";



        //chunk = count byte(how)
        //CHUNK_SIZE_WORDS = count word
//            char chunk[CHUNK_SIZE_WORDS*4];

//            uint32_t chunk_size = file->read(chunk, CHUNK_SIZE_WORDS*4);

//            if(chunk_size){
//                Message message(WRITE, 3 + chunk_size/4);

//                message.payload.push_back(ui->comboBox_App->currentIndex());
//                message.payload.push_back(chunk_size/4);

//                for (uint32_t iter = 0; iter < chunk_size; iter += 4){
//                    //message.payload.append(*((uint32_t *)(chunk + iter)));
//                    message.payload.append(*reinterpret_cast<uint32_t *>(chunk + iter));//answer about again
//                }

//                Count_Wait_Byte = Size::AWAIT_WRITE_SIZE;
//                //PLUG
//                //tmp_data = Message(WRITE, DEFAULT_COMMAND_REQUEST_SIZE + chunk_size/4, 0x00000001).toRaw();//+firm content
//                //this->serial->write(tmp_data);
//                this->serial->write(message.toRaw());

//            }
//            else{
//                file->close();
//                delete file;
//                file = nullptr;
//            }

        break;}

        //case READ
        case 6:{
            this->file = new QFile(ui->lineEdit->text());

                if(!(this->file->open(QFile::ReadOnly))){
                    delete file;
                    file = nullptr;
                    return;
                }





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
//CALLING
//ENQUIRY
void MainWindow::slPing(void){

    //QByteArray tmp_data;

    if(!(this->serial)) return;



    Message message(PING, 0);

    Count_Wait_Byte = Size::AWAIT_PING_SIZE;
    //qDebug()<<"Ping after tranform "<< hex << message.toRaw();
    //Message Get_Program();
    this->serial->write(message.toRaw());

    //tmp_data = Message(PING, 0).toRaw();
    //qDebug()<<"Ping after tranform "<< hex << tmp_data;
    //this->serial->write(tmp_data);
}

void MainWindow::slJump(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Message message(JUMP,1);
    message.payload.append(ui->comboBox_App->currentIndex());

    Count_Wait_Byte = Size::AWAIT_JUMP_SIZE;

    this->serial->write(message.toRaw());

    //tmp_data = Message(JUMP, DEFAULT_COMMAND_REQUEST_SIZE, 0x00000001).toRaw();//"\xA5Z\x02\x01\x00\x00\x00\x01\x00\x00\x00"
    //qDebug()<<"Jump after tranform old way "<< hex << tmp_data<<"\n";
    //this->serial->write(tmp_data);
}

void MainWindow::slVerify(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Message message(VERIFY,1);
    message.payload.append(ui->comboBox_App->currentIndex());

    Count_Wait_Byte = Size::AWAIT_VERIFY_SIZE;

    //qDebug()<<"Verify after transform new way "<< hex << message.toRaw()<<"\n";
    //this->serial->write(message.toRaw());

    tmp_data = Message(VERIFY, DEFAULT_COMMAND_REQUEST_SIZE, 0x00000001).toRaw();
    qDebug()<<"Verify after tranform old way "<< hex << tmp_data<<"\n";
    this->serial->write(tmp_data);
}

void MainWindow::slWrite(void){

//    this->file = new QFile(ui->lineEdit->text());

//    if(!(this->file->open(QFile::ReadOnly))){
//        delete file;
//        file = nullptr;
//        return;
//    }

//    if(((ui->comboBox_App->currentIndex() == 0) && (this->file->size() > FLASH_MAP_APP_1)) ||
//        ((ui->comboBox_App->currentIndex() == 1) && (this->file->size() > FLASH_MAP_APP_2)) ||
//        ((ui->comboBox_App->currentIndex() == 2) && (this->file->size() > FLASH_MAP_APP_USER))){
//        qDebug()<<"Size Error";
//        file->close();
//        delete file;
//        file = nullptr;
//        return;
//    }

//    file->close();
//    delete file;
//    file = nullptr;

//    Flashing_Flag = 1;

//    slErase();


    this->file = new QFile(ui->lineEdit->text());

    if(!(this->file->open(QFile::ReadOnly))){
            delete file;
            file = nullptr;
            return;
        }

//    QByteArray chunk;

//    chunk = file->readAll();

//    qDebug()<<chunk.size()<<"  "<<chunk;

//    if(chunk.size()){
//         Message message(WRITE, 1 + chunk.size()/4);

//         message.payload.push_back(ui->comboBox_App->currentIndex());

//         for(uint8_t i=0;i<chunk.size();i++){
//             char keep = chunk[i];
//             qDebug()<<"chunk[i]"<<keep;
//             message.payload.push_back(keep);
//             //message.payload.append(*reinterpret_cast<uint32_t *>(keep + i*4));
//         }

//         //this->serial->write(message.toRaw());
//         qDebug()<<"BBBBBBB";
//         qDebug()<<"AAAAAAAA"<<message.toRaw();
//    }

//    else{
//        qDebug()<<"File umpty";
//        file->close();
//        delete file;
//        file = nullptr;
//    }

    char packer_firmware[PACKET_FIRMWARE_SIZE_WORDS*4];

    uint32_t packer_firmware_size = file->read(packer_firmware, PACKET_FIRMWARE_SIZE_WORDS*4);

    if(packer_firmware_size){
        Message message(WRITE,(4+packer_firmware_size)/4);

        message.payload.push_back(ui->comboBox_App->currentIndex());

        qDebug()<<"packer_firmware_size"<<packer_firmware_size;

        for (uint32_t iter = 0; iter < packer_firmware_size; iter += 4){
            //message.payload.append(*((uint32_t *)(packer_firmware + iter)));
            message.payload.push_back(*reinterpret_cast<uint32_t *>(packer_firmware + iter));
        }

        Count_Wait_Byte = Size::AWAIT_WRITE_SIZE;
        //qDebug()<<this->serial->write(message.toRaw());
        this->serial->write(message.toRaw());
        //qDebug()<<hex<<message.toRaw();

    }
    else{
        qDebug()<<"Writting end!";
        file->close();
        delete file;
        file = nullptr;
    }

}


void MainWindow::slErase(void){

    QByteArray tmp_data;

    if(!(this->serial)) return;

    Message message(ERASE,1);
    message.payload.append(ui->comboBox_App->currentIndex());

    Count_Wait_Byte = Size::AWAIT_ERASE_SIZE;

    //qDebug()<<"Erase message after tranform "<< hex << message.toRaw();
    this->serial->write(message.toRaw());

    //tmp_data = Message(ERASE, DEFAULT_COMMAND_REQUEST_SIZE, 0x00000001).toRaw();
    //tmp_data = message.toRaw();
    //qDebug()<<"TMP DATA "<<tmp_data;
    //qDebug()<<"TO RAW "<<message.toRaw();
    //this->serial->write(tmp_data);
}

void MainWindow::slRead(void){

    QByteArray tmp_data;

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

