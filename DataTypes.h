#ifndef DATATYPES_H
#define DATATYPES_H

#include "stdint.h"
#include "stdbool.h"

typedef struct __attribute__((__packed__)) {
    double dPitch;
    double dYaw;
    double dRoll;
    uint8_t u8SysCal;
    uint8_t u8MagCal;
    uint8_t u8GyrCal;
    uint8_t u8AccCal;
} oBNO055Data_t, *poBNO055Data_t;

typedef struct __attribute__((__packed__)) {
    float fLongitude;
    float fLatitude;
    float fMSLAlt;
    uint8_t u8Fix;
    uint8_t u8NumSat;
} oCAMM8QData_t, *poCAMM8QData_t;

typedef struct __attribute__((__packed__)) {
    float fTemperature;
    float fHumidity;
    float fPressure;
} oBME280Data_t, *poBME280Data_t;

typedef struct __attribute__((__packed__)) {
    oBNO055Data_t BNO055Data;
    oCAMM8QData_t CAMM8QData;
    oBME280Data_t BME280Data;
} oRawData_t, *poRawData_t;

typedef struct __attribute__((__packed__)) {
    float fBearing;
    float fSpeed;
    float fTargetDistance;
    float fAltitude;
    float fStartPressure;
    uint16_t u16LeftSpeed;
    uint8_t u8LeftDirection;
    uint16_t u16RightSpeed;
    uint8_t u8RightDirection;
    uint32_t u32Time;
    uint32_t u32Battery;
    uint8_t u8Mode;
    uint8_t u8EStop;
} oVehiculeData_t, *poVehiculeData_t;

typedef struct __attribute__((__packed__)) {
    float fReleaseAltitude;                     //Release altitude in m
    float fArmingAltitude;                      //Arming altitude in m
    float fHardReleaseAltitude;                 //First drop altitude in m in case of issue
    float fSoftReleaseAltitude;                 //Second drop altitude in m, for ground offset error
    float fTargetRadius;                        //Max distance to be considered on target in m
    float fTargetLat;                           //Target latitude
    float fTargetLong;                          //Target longitude

    uint16_t u16HardReleaseCountdown;           //Time before executing a hard realease in ms after reaching fHardReleaseAltitude
    uint16_t u16SoftReleaseCountdown;           //Time before executing a soft realease in ms after reaching fSoftReleaseAltitude
    uint16_t u16MaxSpeed;                       //Max allowed motor speed between 0 - 4096
    uint16_t u16HeartbeatTransmitInterval;      //Interval between hearbeat transmission in ms
    uint16_t u16SensorTransmitInterval;         //Interval between sensor transmission in ms
    uint8_t u8VID;                              //Vehicule ID between 0 - 255
    bool bSound;                                //Enables or disables sound, true = enabled
    bool bExternalTelemetry;                    //Uses external telemetry, true = external
} oConfig_t, *poConfig_t;

typedef struct __attribute__((__packed__)) {
    uint32_t u32Timestamp;
} oHeartbeatData_t, *poHeartbeatData_t;

typedef struct __attribute__((__packed__)) {
    uint16_t u16LeftSpeed;
    uint8_t u8LeftDirection;
    uint16_t u16RightSpeed;
    uint8_t u8RightDirection;
} oMotorSpeedData_t, *poMotorSpeedData_t;

#endif // DATATYPES_H
