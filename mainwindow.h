#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QDebug>

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
    void slWrite(void);

private:
    Ui::MainWindow *ui;

    uint32_t Count_Wait_Byte;//how many bytes we expect from a particular command

    enum Command{
        PING = 0x01
    };

    enum Size{
        AWAIT_PING = 11
    };

    struct Message{

        uint16_t preamb;
        uint8_t cmd;
        uint32_t size;
        uint8_t id[4];
        QString id_str;

        Message(void){
            this->preamb = 0x5AA5;//swap 5aa5 to a55a
            this->cmd = 0x00;
            this->size = 0x00;
        }

        Message(uint8_t cmds, uint32_t sizes){
            this->preamb = 0x5AA5;
            this->cmd = cmds;
            this->size = sizes;
        }

        //Из сырого в норм
        //Command::PING
        int8_t fromRaw(QByteArray data){

            QDataStream s(&data, QIODevice::ReadOnly);
            s.setByteOrder(QDataStream::LittleEndian);

            s >> this->preamb;
            s >> this->cmd;
            s >> this->size;

            if((preamb != 0x5AA5)){
                return -1;
            }

            if(cmd != 1){
                return -2;
            }

            return 0;
        }


        //Из норм в сырой
        QByteArray toRaw(void){

            QByteArray data;
            QDataStream s(&data, QIODevice::WriteOnly);
            s.setByteOrder(QDataStream::LittleEndian);

            s << this->preamb;
            s << this->cmd;
            s << this->size;

            return data;
        }

        int8_t GetId(QByteArray data){

            QDataStream s(&data, QIODevice::ReadOnly);
            s.setByteOrder(QDataStream::LittleEndian);

            s >> this->id[0];
            s >> this->id[1];
            s >> this->id[2];
            s >> this->id[3];

            id_str.push_back(id[0]);
            id_str.push_back(id[1]);
            id_str.push_back(id[2]);
            id_str.push_back(id[3]);

            qDebug()<<"ID is "<<id_str;

            return 0;
        }
    };

    QSerialPort * serial = nullptr;
};
#endif // MAINWINDOW_H
