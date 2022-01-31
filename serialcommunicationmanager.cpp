#include "serialcommunicationmanager.h"
#ifndef Q_OS_IOS
SerialCommunicationManager::SerialCommunicationManager(QObject *parent) : QObject(parent)
{
    m_serialPort = nullptr;
}

QList<QSerialPortInfo> SerialCommunicationManager::listSerialPort() {
    return QSerialPortInfo::availablePorts();
}

void SerialCommunicationManager::connectSerial(QString portName, int baud) {
    if(m_serialPort != nullptr) {
        disconnectSerial();
    }

    m_serialPort = new QSerialPort();
    m_serialPort->setPortName(portName);
    m_serialPort->setBaudRate(baud);
    m_serialPort->setDataBits(QSerialPort::Data8);
    m_serialPort->setParity(QSerialPort::NoParity);
    m_serialPort->setStopBits(QSerialPort::OneStop);
    m_serialPort->setFlowControl(QSerialPort::NoFlowControl);
    if(m_serialPort->open(QIODevice::ReadWrite) == true) {
        serialConnected();
    }

    connect(m_serialPort, SIGNAL(readyRead()),this, SLOT(serialReadyRead()));
}

void SerialCommunicationManager::serialConnected() {
    isSerialConnected = true;
    emit connectionChanged(isSerialConnected);
}
void SerialCommunicationManager::serialDisconnected() {
    isSerialConnected = false;
    emit connectionChanged(isSerialConnected);
}
void SerialCommunicationManager::serialReadyRead() {
    QByteArray ba = m_serialPort->readAll();
    uint8_t buf[ba.count()];
    for(int i = 0; i < ba.count(); i++) {
        buf[i] = (uint8_t)ba.at(i);
    }

    emit bytesAvailable(buf, sizeof(buf));
}

bool SerialCommunicationManager::sendData(uint8_t *data, uint8_t len) {
    if(m_serialPort != nullptr) {
        if(m_serialPort->isOpen()) {
            m_serialPort->write((const char*)data, len);
            return true;
        } else {
            disconnectSerial();
            return false;
        }
    }
    return false;
}

void SerialCommunicationManager::disconnectSerial() {
    if(m_serialPort != nullptr) {
        m_serialPort->close();
        delete m_serialPort;
        m_serialPort = nullptr;
        isSerialConnected = false;
        emit connectionChanged(isSerialConnected);
    }
}
#endif
