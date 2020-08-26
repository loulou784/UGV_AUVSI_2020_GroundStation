#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QFileDialog>
#include <qflightinstruments/WidgetADI.h>
#include <qflightinstruments/WidgetHSI.h>
#include <QFileDialog>
#include "controllermanager.h"
#include "DataTypes.h"
#include "communicationmanager.h"

#ifndef Q_OS_IOS
#include <QSerialPort>
#include <QSerialPortInfo>
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

private:
    Ui::MainWindow *ui;
    ControllerManager *m_controllerManager;
    CommunicationManager *m_communicationManager;
    oRawData_t rawData;
    oConfig_t configData;
    WidgetADI *m_widgetADI;
    WidgetHSI *m_widgetHSI;
};
#endif // MAINWINDOW_H
