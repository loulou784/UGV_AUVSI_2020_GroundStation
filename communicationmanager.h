#ifndef COMMUNICATIONMANAGER_H
#define COMMUNICATIONMANAGER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <stdint.h>
#include <math.h>
#include "tcpcommunicationmanager.h"
#include "DataTypes.h"

#ifndef Q_OS_IOS
#include "serialcommunicationmanager.h"
#endif

#define MY_VID 0x01
#define START_BYTE  0x16

enum commSM {
    START = 0,
    VID,
    COMMAND,
    LENGTH,
    PAYLOAD,
    CRC16
};

// START_BYTE           --  1 byte
// VID_ID               --  1 byte
// COMMAND              --  1 byte
// PAYLOAD_LENGTH       --  1 byte
// PAYLOAD              --  [0 to 248] bytes
// CRC16                --  2 bytes
//               Max Total: 256 bytes

class CommunicationManager : public QObject
{
    Q_OBJECT
public:
    explicit CommunicationManager(QObject *parent = nullptr);
    void reset();
    uint16_t calculateCRC16(uint8_t *arr, uint8_t len);

private slots:
    void processChar(uint8_t ch);
    void processMultipleBytes(uint8_t *data, uint8_t len);
    void processMessage(uint8_t *arr, uint8_t len);
    void serialStateChanged(bool isConnected);
    void TCPStateChanged(bool isConnected);
    void sendBytes(uint8_t* data, uint8_t len);

signals:
    void newMessageToProcess(uint8_t *arr, uint8_t len);
    void connectionChanged(bool isSerial, bool isConnected);
    void receivedHeartbeat(uint32_t current, uint32_t previous);
    void receivedRawData(oRawData_t rawData);
    void receivedConfigData(oConfig_t configData);

public slots:
    void startTCPComm(QString host, quint16 port);
    void stopTCPComm();
    void sendControllerData(uint8_t *data, uint8_t len);
    void sendReadParamCommand();


#ifndef Q_OS_IOS
    void startSerialComm(QString port, int baud);
    void stopSerialComm();
    QList<QSerialPortInfo> listSerialPort();
#endif

private:
    commSM currentState = commSM::START;            // Our current state
    uint8_t dataBuffer[256];                        // Our incoming data buffer
    uint8_t crcCounter    = 2;
    uint8_t payloadLength = 0;
    uint8_t currentLength = 0;
    bool isSerialConnected = false;
    bool isTCPConnected = false;
    uint32_t lastHeartbeat;
    uint32_t currentHeartbeat;

    TCPCommunicationManager *m_tcpCommunicationManager;
#ifndef Q_OS_IOS
    SerialCommunicationManager *m_serialCommunicationManager;
#endif
};

#endif // COMMUNICATIONMANAGER_H
