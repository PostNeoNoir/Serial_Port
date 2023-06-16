#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QDebug>
#include <QFile>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void slConnect(void);
    void slReadyRead(void);
    void slPing(void);
    void slJump(void);
    void slErase(void);
    void slWrite(void);
    void slVerify(void);
    void slRead(void);
    void BrowseFile(void);


private:
    Ui::MainWindow *ui;

    uint32_t Count_Wait_Byte;//how many bytes we expect from a particular command
    uint8_t Flashing_Flag = 0;
    QFile *file = nullptr;

    enum FlashMapping{
            FLASH_MAP_APP_1 = 40960,
            FLASH_MAP_APP_2 = 32768,
            FLASH_MAP_APP_USER = 40960
        };

    enum Command{
        PING = 0x01,
        JUMP = 0x02,  // for egors programm 0x03
        VERIFY = 0x03, // for egors programm not use
        WRITE = 0x04, // for egors programm 0x04
        ERASE = 0x05, // for egors 5rogramm 0x05
        READ = 0x06 // for egors programm 0x06
    };

    enum Size{
        AWAIT_PING_SIZE = 11,
        AWAIT_VERIFY_SIZE = 15,
        AWAIT_JUMP_SIZE = 7,
        AWAIT_READ_SIZE = 11,
        AWAIT_WRITE_SIZE = 11,
        AWAIT_ERASE_SIZE = 7,
        NUMBER_PROGRAMM_SIZE = 4,
        NUMBER_ID_SIZE = 4,
        CHECKSUM_VALUE = 4,
        DEFAULT_COMMAND_REQUEST_SIZE = 0x00000001,
        PACKET_FIRMWARE_SIZE_WORDS = 4
    };

    struct Message{

        uint16_t preamb;
        uint8_t cmd;
        uint32_t size;
        uint32_t number_programm;
        QVector<uint32_t>   payload;
        uint8_t progam_number[4];
        QString progam_number_str;

        Message(void){
            this->preamb = 0x5AA5;
            this->cmd = 0x00;
            this->size = 0x00;
        }

        Message(uint8_t cmds, uint32_t sizes){
            this->preamb = 0x5AA5;
            this->cmd = cmds;
            this->size = sizes;
        }

        Message(uint8_t cmds, uint32_t sizes, uint32_t number_programms){
            this->preamb = 0x5AA5;
            this->cmd = cmds;
            this->size = sizes;
            this->payload.push_back(number_programms);
        }

        Message(uint8_t cmds){
            this->preamb = 0x5AA5;
            this->cmd = cmds;
        }



        //From raw to normal
        int8_t fromRaw(QByteArray data){

            QDataStream s(&data, QIODevice::ReadOnly);
            s.setByteOrder(QDataStream::LittleEndian);

            s >> this->preamb;
            s >> this->cmd;
            s >> this->size;
            uint32_t keep;
            for (uint32_t i = 0; i < this->size; i++){
                s >> keep;
                this->payload.push_back(keep);
            }

            if((preamb != 0x5AA5)){
                return -1;
            }

            //Command::PING
            if(cmd == PING){
                if(size == 0x00000001)return 1;
                else return -2;
            }

            //Command::JUMP
            if(cmd == JUMP){
                if(size == 0x00000000)return 2;
                else return -2;
            }

            //Command::VERIFY
            if(cmd == VERIFY){
                if(size == 0x00000002)return 3;
                else return -2;

            }

            //Command::ERASE
            if(cmd == ERASE){
                if(size == 0x00000000)return 5;
                else return -2;

            }

            //Command::WRITE
            if(cmd == WRITE){
                if(size == 0x00000000)return 4;
                else return -2;
            }


            //Command::READ
            if(cmd == READ){
                return 6;
            }

            return 0;
        }


        //From normal to raw
        QByteArray toRaw(){

            QByteArray data;
            QDataStream s(&data, QIODevice::WriteOnly);
            s.setByteOrder(QDataStream::LittleEndian);

            s << this->preamb;
            s << this->cmd;
            s << this->size;
            uint32_t keep;

            foreach(auto bar, this->payload){
                keep = bar;
                s << keep;
                //qDebug()<<"Payload "<<hex<<keep;
            }

            return data;
        }

        int8_t Get_Program_ID(QByteArray data){

            QDataStream s(&data, QIODevice::ReadOnly);
            s.setByteOrder(QDataStream::LittleEndian);

            s >> this->progam_number[0];
            s >> this->progam_number[1];
            s >> this->progam_number[2];
            s >> this->progam_number[3];

            progam_number_str.push_back(progam_number[0]);
            progam_number_str.push_back(progam_number[1]);
            progam_number_str.push_back(progam_number[2]);
            progam_number_str.push_back(progam_number[3]);

            qDebug()<<"Progam ID is "<<hex<<progam_number_str;

            return 0;
        }
    };

    QSerialPort * serial = nullptr;
};
#endif // MAINWINDOW_H
