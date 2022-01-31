#ifndef COMMUNICATIONMANAGER_H
#define COMMUNICATIONMANAGER_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include <stdint.h>
#include <math.h>
#include "tcpcommunicationmanager.h"
#include "DataTypes.h"

#if !defined(Q_OS_IOS) && !defined(Q_OS_WASM)
#include "serialcommunicationmanager.h"
#endif

//CommManager available command
#define BNO055DATA_CMD 0x01
#define BNO055DATA_LEN (sizeof(oHeartbeatData_t))

#define CAMM8QDATA_CMD 0x02
#define CAMM8QDATA_LEN (sizeof(oCAMM8QData_t))

#define BME280DATA_CMD 0x03
#define BME280DATA_LEN (sizeof(oBME280Data_t))

#define RAWDATA_CMD 0x04
#define RAWDATA_LEN (sizeof(oRawData_t))

#define VEHICULEDATA_CMD 0x05
#define VEHICULEDATA_LEN (sizeof(oVehiculeData_t))

#define CONFIGDATA_CMD 0x06
#define CONFIGDATA_LEN (sizeof(oConfig_t))

#define HEARTBEAT_CMD 0x07
#define HEARTBEAT_LEN (sizeof(oHeartbeatData_t))

#define MOTORSPEED_CMD 0x07
#define MOTORSPEED_LEN (sizeof(oMotorSpeedData_t))

#define READPARAM_CMD 0x08
#define READPARAM_LEN (0x00)

#define SETMODE_CMD 0x09
#define SETMODE_LEN (0x01)

#define SETESTOP_CMD 0x0A
#define SETESTOP_LEN (0x01)

#define SETRESETALT_CMD 0x0B
#define SETRESETALT_LEN (0x01)

#define SETRELEASE_CMD 0x0C
#define SETRELEASE_LEN (0x01)

// CommManager parameter
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
    void receivedBNO055Data(oBNO055Data_t BNO055Data);
    void receivedCAQMM8QData(oCAMM8QData_t CAMM8QData);
    void receivedBME280Data(oBME280Data_t BME280Data);
    void receivedVehiculeData(oVehiculeData_t VehiculeData);

public slots:
    void startTCPComm(QString host, quint16 port);
    void stopTCPComm();
    void sendControllerData(uint8_t *data, uint8_t len);
    void sendReadParamCommand();
    void sendCommand(uint8_t cmd, uint8_t payload[], uint8_t len);


#if !defined(Q_OS_IOS) && !defined(Q_OS_WASM)
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
#if !defined(Q_OS_IOS) && !defined(Q_OS_WASM)
    SerialCommunicationManager *m_serialCommunicationManager;
#endif
};

#endif // COMMUNICATIONMANAGER_H
