#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <QFlightInstruments/WidgetADI.h>
#include <QFlightInstruments/WidgetHSI.h>
#include <QMessageBox>
#include <QFileDialog>
#include <QDate>
#include <QTime>
#include <QStandardItemModel>
#include <math.h>

#include "DataTypes.h"
#include "communicationmanager.h"

#if !defined(Q_OS_IOS) && !defined(Q_OS_WASM)
#include <QSerialPort>
#include <QSerialPortInfo>
#endif

#if !defined(Q_OS_WASM)       //Do not show if WASM
#include "controllermanager.h"
//#include "tcpvideowidget.h"
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void gamepadViewSetup();
    void paramViewSetup();
    void sensorViewSetup();
    void controlViewSetup();

private slots:
    void gamepadState(QVariant data);
    void enableTCPUI(bool isEnable);
    void enableSerialUI(bool isEnable);
    void communicationConnectionChanged(bool isSerial, bool isConnected);
    void tabChanged(int index);
    void openFirmwareFilePicker();
    void connectTCP();
    void connectSerial();
    void writeParamToVehicule();
    void readParamFromVehicule();
    void heartbeatChanged(uint32_t current, uint32_t previous);
    void rawDataChanged(oRawData_t data);
    void configDataChanged(oConfig_t data);
    void vehiculeDataChanged(oVehiculeData_t data);
    void switchModeButtonClicked();
    void estopButtonClicked();
    void resetAltitudeClicked();
    void releaseButtonClicked();

private:
    Ui::MainWindow *ui;

#if !defined(Q_OS_WASM)       //Do not show if WASM
    ControllerManager *m_controllerManager;
    //TCPVideoWidget *m_video;
#endif

    CommunicationManager *m_communicationManager;
    oRawData_t rawData;
    oConfig_t configData;
    oVehiculeData_t vehiculeData;
    bool enableControllerSending;
    WidgetADI *m_widgetADI;
    WidgetHSI *m_widgetHSI;
    QStandardItemModel treeViewList;

    QStandardItem* generateNodeForBNO055Data(oBNO055Data_t data);
    QStandardItem* generateNodeForCAMM8QData(oCAMM8QData_t data);
    QStandardItem* generateNodeForBME280Data(oBME280Data_t data);
    QStandardItem* generateNodeForVehiculeData(oVehiculeData_t data);
    QStandardItem* generateNodeForConfigData(oConfig_t data);

    void updateNodeForBNO055Data(oBNO055Data_t data);
    void updateNodeForCAMM8QData(oCAMM8QData_t data);
    void updateNodeForBME280Data(oBME280Data_t data);
    void updateNodeForVehiculeData(oVehiculeData_t data);
    void updateNodeForConfigData(oConfig_t data);

    bool CreateUpgradeFile(QString inFilename, QString outFilename, uint32_t totalSizeInBytes);
    QString UpgradeFileUIData(QString inFilename, QString outFilename, uint32_t totalSizeInBytes);
    uint32_t calculateCRC32(QByteArray *ba);
};
#endif // MAINWINDOW_H
