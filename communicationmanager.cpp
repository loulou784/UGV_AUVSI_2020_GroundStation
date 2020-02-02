#include "communicationmanager.h"

CommunicationManager::CommunicationManager(QObject *parent) : QObject(parent)
{
    reset();
    connect(this, SIGNAL(newMessageToProcess(uint8_t*, uint8_t)), this, SLOT(processMessage(uint8_t*, uint8_t)));


    // Start Serial communication if not on IOS
#ifndef Q_OS_IOS
    m_serialCommunicationManager = new SerialCommunicationManager();
    connect(m_serialCommunicationManager, SIGNAL(connectionChanged(bool)), this, SLOT(serialStateChanged(bool)));
    connect(m_serialCommunicationManager, SIGNAL(bytesAvailable(uint8_t*, uint8_t)), this, SLOT(processMultipleBytes(uint8_t*, uint8_t)));
#endif

    m_tcpCommunicationManager = new TCPCommunicationManager();
    // Start TCP Communication
    connect(m_tcpCommunicationManager, SIGNAL(connectionChanged(bool)), this, SLOT(TCPStateChanged(bool)));
    connect(m_tcpCommunicationManager, SIGNAL(bytesAvailable(uint8_t*, uint8_t)), this, SLOT(processMultipleBytes(uint8_t*, uint8_t)));
}

void CommunicationManager::reset() {
    memset(dataBuffer, 0, sizeof(dataBuffer));
    crcCounter = 2;
    payloadLength = 0;
    currentLength = 0;
}

uint16_t CommunicationManager::calculateCRC16(uint8_t *pu8Data, uint8_t u16Length) {
    uint16_t i;
    uint16_t u16crc = 0xffff;

    if(pu8Data == nullptr) {
        return 0;
    }

    while (u16Length--) {
        u16crc ^= *(unsigned char *)pu8Data++ << 8;
        for (i=0; i < 8; i++)
            u16crc = (u16crc & (uint16_t)0x8000) ? (u16crc << 1) ^ (uint16_t)0x1021 : u16crc << 1;
    }
    uint16_t c = (u16crc & 0xffff);
    c = (c << 8) + (c >> 8);
    return c;
}

void CommunicationManager::processChar(uint8_t ch) {
    switch (currentState) {
    case commSM::START:
        if(ch == START_BYTE) {
            reset();
            dataBuffer[currentLength] = ch;
            currentState = commSM::TARGET;
            currentLength++;
        }
        break;

    case commSM::TARGET:
        // Directed to ME
        if(ch == MY_ID) {
            dataBuffer[currentLength] = ch;
            currentState = commSM::SOURCE;
            currentLength++;
        } else {
            currentState = commSM::START;
        }
        break;

    case commSM::SOURCE:
        // From someone I trust
        if(ch == TARGET_ID) {
            dataBuffer[currentLength] = ch;
            currentState = commSM::COMMAND;
            currentLength++;
        } else {
            currentState = commSM::START;
        }
        break;

    case commSM::COMMAND:
        dataBuffer[currentLength] = ch;
        currentState = commSM::LENGTH;
        currentLength++;
        break;

    case commSM::LENGTH:
        dataBuffer[currentLength] = ch;
        payloadLength = ch;
        currentLength++;
        if(payloadLength > 0) {
            currentState = commSM::PAYLOAD;
        } else {
            currentState = commSM::CRC16;
        }
        break;

    case commSM::PAYLOAD:
        dataBuffer[currentLength] = ch;
        currentLength++;
        payloadLength--;
        if(payloadLength == 0x00) {
            currentState = commSM::CRC16;
        }
        break;

    case commSM::CRC16:
        dataBuffer[currentLength] = ch;
        currentLength++;
        crcCounter--;
        if(crcCounter == 0x00) {
            currentState = commSM::START;
            if(calculateCRC16(dataBuffer, currentLength) == 0x00) {
                uint8_t buf[256];
                memcpy(buf, dataBuffer, currentLength);
                emit newMessageToProcess(buf, currentLength);
            }
        }
        break;
    }
}

void CommunicationManager::processMultipleBytes(uint8_t *data, uint8_t len) {
    for(int i = 0; i < len; i++) {
        processChar(data[i]);
    }
}

void CommunicationManager::serialStateChanged(bool isConnected) {
    isSerialConnected = isConnected;
    emit connectionChanged(true, isConnected);
}

void CommunicationManager::TCPStateChanged(bool isConnected) {
    isTCPConnected = isConnected;
    emit connectionChanged(false, isConnected);
}

void CommunicationManager::sendBytes(uint8_t* data, uint8_t len) {
#ifndef Q_OS_IOS
    if(isSerialConnected) {
        m_serialCommunicationManager->sendData(data, len);
    }
#endif
    if(isTCPConnected) {
        m_tcpCommunicationManager->sendData(data, len);
    }
}

void CommunicationManager::processMessage(uint8_t *arr, uint8_t len) {
    // Dispatch received message
    qDebug() << "New Message: ";
    for(int i = 0; i < len; i++) {
        qDebug() << "0x" << hex << arr[i];
    }
}

void CommunicationManager::startTCPComm(QString host, quint16 port) {
    qDebug() << "Connecting to: " << host << ":" << port;
    m_tcpCommunicationManager->connectSocket(host, port);
}

void CommunicationManager::stopTCPComm() {
    m_tcpCommunicationManager->disconnectSocket();
}

void CommunicationManager::sendControllerData(uint8_t *data, uint8_t len) {

    uint8_t toSend[7 + len];
    toSend[0] = START_BYTE;
    toSend[1] = TARGET_ID;
    toSend[2] = MY_ID;
    toSend[3] = CMD::CONTROLLER;
    toSend[4] = len;
    memcpy(&toSend[5], data, len);

    uint16_t crc = calculateCRC16(toSend, sizeof(toSend) - 2);
    memcpy(&toSend[5 + len], &crc, sizeof(crc));
    sendBytes(toSend, sizeof(toSend));

    QDebug dbg(QtDebugMsg);
    for(int i = 0; i < sizeof(toSend); i++) {
        dbg << hex << toSend[i];
    }
    dbg << "  CRC16:" << calculateCRC16(toSend, sizeof(toSend));
}

#ifndef Q_OS_IOS
void CommunicationManager::startSerialComm(QString port, int baud) {
    qDebug() << "Connecting to: " << port << " at " << baud << " baud";
    m_serialCommunicationManager->connectSerial(port, baud);
}

void CommunicationManager::stopSerialComm() {
    m_serialCommunicationManager->disconnectSerial();
}

QList<QSerialPortInfo> CommunicationManager::listSerialPort() {
    return m_serialCommunicationManager->listSerialPort();
}
#endif
