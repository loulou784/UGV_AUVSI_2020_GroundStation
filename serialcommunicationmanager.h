#ifndef SERIALCOMMUNICATIONMANAGER_H
#define SERIALCOMMUNICATIONMANAGER_H

#include <QObject>
#include <QDebug>

#ifndef Q_OS_IOS
#include <QSerialPort>
#include <QSerialPortInfo>

class SerialCommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit SerialCommunicationManager(QObject *parent = nullptr);
    void connectSerial(QString portName, int baud);
    void disconnectSerial();

signals:
    void bytesAvailable(uint8_t *data, uint8_t len);
    void connectionChanged(bool isConnected);

private slots:
    void serialConnected();
    void serialDisconnected();
    void serialReadyRead();

public slots:
    QList<QSerialPortInfo> listSerialPort();
    bool sendData(uint8_t *data, uint8_t len);
private:
    QSerialPort *m_serialPort;
    bool isSerialConnected;

};

#endif

#endif // SERIALCOMMUNICATIONMANAGER_H
