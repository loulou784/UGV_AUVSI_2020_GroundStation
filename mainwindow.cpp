#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

#ifdef Q_OS_IOS
    qDebug() << "IOS is defined";
#else
    qDebug() << "IOS is not defined";
#endif

#ifdef Q_OS_WASM
    qDebug() << "WASM is defined";
#else
    qDebug() << "WASM is not defined";
#endif


    setWindowTitle("Ground Station");

    m_widgetADI = new WidgetADI();
    m_widgetHSI = new WidgetHSI();

    ui->adiLayout->addWidget(m_widgetADI);
    ui->hsiLayout->addWidget(m_widgetHSI);

#if defined(Q_OS_IOS) || defined(Q_OS_WASM)       //Do not show if WASM or IOS
    ui->SerialGroupBox->setVisible(false);
#endif

#if !defined(Q_OS_WASM)       //Do not show if WASM
    m_controllerManager = new ControllerManager();
#endif

    enableControllerSending = false;

    m_communicationManager = new CommunicationManager();

    connect(m_communicationManager, SIGNAL(connectionChanged(bool, bool)), this, SLOT(communicationConnectionChanged(bool, bool)));
    connect(ui->firmwareSelectButton, SIGNAL(clicked()), this, SLOT(openFirmwareFilePicker()));

    connect(ui->TCPConnectButton, SIGNAL(clicked()), this, SLOT(connectTCP()));
    connect(ui->TCPDisconnectButton, SIGNAL(clicked()), m_communicationManager, SLOT(stopTCPComm()));

#if !defined(Q_OS_IOS) && !defined(Q_OS_WASM)       //Do not show if WASM or IOS
    connect(ui->SerialConnectButton, SIGNAL(clicked()), this, SLOT(connectSerial()));
    connect(ui->SerialDisconnectButton, SIGNAL(clicked()), m_communicationManager, SLOT(stopSerialComm()));
#endif     
    connect(ui->tabView, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    ui->tabView->removeTab(6);      //OTA Update TAB
    ui->tabView->removeTab(0);      //Camera Update TAB

    //m_video = new TCPVideoWidget();
    //ui->tab_7->layout()->addWidget(m_video);

    connect(m_communicationManager, SIGNAL(receivedHeartbeat(uint32_t, uint32_t)), this, SLOT(heartbeatChanged(uint32_t, uint32_t)));
    connect(m_communicationManager, SIGNAL(receivedRawData(oRawData_t)), this, SLOT(rawDataChanged(oRawData_t)));
    connect(m_communicationManager, SIGNAL(receivedConfigData(oConfig_t)), this, SLOT(configDataChanged(oConfig_t)));
    connect(m_communicationManager, SIGNAL(receivedVehiculeData(oVehiculeData_t)), this, SLOT(vehiculeDataChanged(oVehiculeData_t)));

    paramViewSetup();
    gamepadViewSetup();
    sensorViewSetup();
    controlViewSetup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::gamepadViewSetup() {
#if !defined(Q_OS_WASM)       //Do not show if WASM
    connect(m_controllerManager, SIGNAL(controllerStateChanged(QVariant)), this, SLOT(gamepadState(QVariant)));
#endif
}

void MainWindow::paramViewSetup() {
    connect(ui->paramWriteParamButton, SIGNAL(clicked()), this, SLOT(writeParamToVehicule()));
    connect(ui->paramReadButton, SIGNAL(clicked()), this, SLOT(readParamFromVehicule()));

}

void MainWindow::gamepadState(QVariant data) {
#if !defined(Q_OS_WASM)       //Do not show if WASM
    struct controllerState cs = data.value<struct controllerState>();

    ui->ltProgressBar->setValue((int)(cs.ltTrigger*100.0));
    ui->rtProgressBar->setValue((int)(cs.rtTrigger*100.0));
    ui->aButton->setChecked(cs.buttonA);
    ui->bButton->setChecked(cs.buttonB);
    ui->xButton->setChecked(cs.buttonX);
    ui->yButton->setChecked(cs.buttonY);
    ui->rbButton->setChecked(cs.rbButton);
    ui->lbButton->setChecked(cs.lbButton);
    ui->lcdLeftX->display(QString::number(cs.leftXAxis, 'f', 2));
    ui->lcdLeftY->display(QString::number(cs.leftYAxis, 'f', 2));
    ui->lcdRightX->display(QString::number(cs.rightXAxis, 'f', 2));
    ui->lcdRightY->display(QString::number(cs.rightYAxis, 'f', 2));

    if(cs.dpadUp && cs.dpadLeft) {
        ui->dPad->setValue(135);
        ui->dPad->setEnabled(true);
    } else if(cs.dpadUp && cs.dpadRight) {
        ui->dPad->setValue(225);
        ui->dPad->setEnabled(true);
    } else if(cs.dpadDown && cs.dpadLeft) {
        ui->dPad->setValue(45);
        ui->dPad->setEnabled(true);
    } else if(cs.dpadDown && cs.dpadRight) {
        ui->dPad->setValue(315);
        ui->dPad->setEnabled(true);
    } else if(cs.dpadUp) {
        ui->dPad->setValue(180);
        ui->dPad->setEnabled(true);
    } else if(cs.dpadDown) {
        ui->dPad->setValue(0);
        ui->dPad->setEnabled(true);

    } else if(cs.dpadLeft) {
        ui->dPad->setValue(90);
        ui->dPad->setEnabled(true);

    } else if(cs.dpadRight) {
        ui->dPad->setValue(270);
        ui->dPad->setEnabled(true);
    } else {
        ui->dPad->setValue(0);
        ui->dPad->setEnabled(false);
    }

    if(enableControllerSending) {
        oMotorSpeedData_t speedData;
        memset(&speedData, 0, sizeof(speedData));

        // 0 = forward, 1 = backward, Y axis forward = -1
        speedData.u16LeftSpeed = (uint16_t)(abs(cs.leftYAxis) * 4096);
        speedData.u8LeftDirection = (cs.leftYAxis >= 0 ? 1 : 0);
        speedData.u16RightSpeed = (uint16_t)(abs(cs.rightYAxis) * 4096);
        speedData.u8RightDirection = (cs.rightYAxis >= 0 ? 1 : 0);

        m_communicationManager->sendCommand(MOTORSPEED_CMD, (uint8_t *)&speedData, MOTORSPEED_LEN);
    }
#endif
}

void MainWindow::enableTCPUI(bool isEnable) {
    ui->TCPGroupBox->setEnabled(isEnable);
}

void MainWindow::enableSerialUI(bool isEnable) {
    ui->SerialGroupBox->setEnabled(isEnable);
}

void MainWindow::communicationConnectionChanged(bool isSerial, bool isConnected) {
    if(!isSerial) {
        if(isConnected) {
            // Disconnect serial port
            ui->TCPConnectLabel->setText("Connection established");
            ui->TCPConnectLabel->setStyleSheet("QLabel { color : green; }");
            enableSerialUI(false);
        } else {
            ui->TCPConnectLabel->setText("No connection");
            ui->TCPConnectLabel->setStyleSheet("QLabel { color : red; }");
            enableSerialUI(true);
        }
    } else {
        if(isConnected) {
            // Disconnect serial port
            ui->SerialConnectLabel->setText("Connection established");
            ui->SerialConnectLabel->setStyleSheet("QLabel { color : green; }");
            enableTCPUI(false);
        } else {
            ui->SerialConnectLabel->setText("No connection");
            ui->SerialConnectLabel->setStyleSheet("QLabel { color : red; }");
            enableTCPUI(true);
        }
    }
}

void MainWindow::tabChanged(int index) {
    qDebug() << index;
    if(index == 5) {

#if !defined(Q_OS_IOS) && !defined(Q_OS_WASM)       //Do not show if WASM or IOS
        QString selectedText = ui->serialPortComboBox->currentText();
        ui->serialPortComboBox->clear();
        int index = 0;
        Q_FOREACH(QSerialPortInfo port, m_communicationManager->listSerialPort()) {
            ui->serialPortComboBox->addItem(port.portName());
            if(port.portName() == selectedText) {
                ui->serialPortComboBox->setCurrentIndex(index);
            }
            index++;
        }
#endif
    }
}

void MainWindow::openFirmwareFilePicker() {
        QString filePath = QFileDialog::getOpenFileName(this, tr("SelectFirmware"), "/", tr("Binary files (*.bin)"));
        QString path = QFileInfo(filePath).path();

        ui->firmwarePath->setText(filePath);

        CreateUpgradeFile(filePath, path + "/UPGRADE.bin", 208*1024);

        ui->FirmwareTextEdit->setText(UpgradeFileUIData(filePath, path + "/UPGRADE.bin", 208*1024));
}

void MainWindow::connectTCP() {
    m_communicationManager->startTCPComm(ui->hostIPLineEdit->text(), ui->portSpinBox->value());
}

void MainWindow::connectSerial() {
#if !defined(Q_OS_IOS) && !defined(Q_OS_WASM)       //Do not show if WASM or IOS
    m_communicationManager->startSerialComm(ui->serialPortComboBox->currentText(), ui->baudRateComboBox->currentText().toInt());
#endif
}

void MainWindow::writeParamToVehicule() {
    oConfig_t configToSend;
    configToSend.fReleaseAltitude = float(ui->param_fReleaseAltitude->value());
    configToSend.fArmingAltitude = float(ui->param_fArmingAltitude->value());
    configToSend.fHardReleaseAltitude = float(ui->param_fHardReleaseAltitude->value());
    configToSend.fSoftReleaseAltitude = float(ui->param_fSoftReleaseAltitude->value());
    configToSend.fTargetRadius = float(ui->param_fTargetRadius->value());
    configToSend.fTargetLat = float(ui->param_fTargetLat->value());
    configToSend.fTargetLong = float(ui->param_fTargetLong->value());

    configToSend.u16HardReleaseCountdown = uint16_t(ui->param_u16HardReleaseCountdown->value());
    configToSend.u16SoftReleaseCountdown = uint16_t(ui->param_u16SoftReleaseCountdown->value());
    configToSend.u16MaxSpeed = uint16_t(ui->param_u16MaxSpeed->value());
    configToSend.u16HeartbeatTransmitInterval = uint16_t(ui->param_u16HeartbeatTransmitInterval->value());
    configToSend.u16SensorTransmitInterval = uint16_t(ui->param_u16SensorTransmitInterval->value());
    configToSend.u8VID = uint8_t(ui->param_u8VID->value());
    configToSend.bSound = bool(ui->param_bSound->isChecked());
    configToSend.bExternalTelemetry = bool(ui->param_bExternalTelemetry->isChecked());
}

void MainWindow::heartbeatChanged(uint32_t current, uint32_t previous) {
    if(ui->statusbar->currentMessage().contains("🔵")) {
        ui->statusbar->showMessage("📡 Elapsed: " + QString::number(current - previous) + " ms");
    } else {
        ui->statusbar->showMessage("📡 Elapsed: " + QString::number(current - previous) + " ms  🔵");
    }
}

void MainWindow::vehiculeDataChanged(oVehiculeData_t data) {
    this->vehiculeData = data;

    if(data.u8EStop == 1) {
        ui->modeLabel->setText("E-Stopped");
        enableControllerSending = false;
    } else {
        if(data.u8Mode == 0) {
            //In Teleop
            ui->modeLabel->setText("Teleop");
            enableControllerSending = true;
        } else if(data.u8Mode == 1) {
            //In Auto
            ui->modeLabel->setText("Auto");
            enableControllerSending = false;
        }
    }

    updateNodeForVehiculeData(data);

    ui->statAltitudeLabel->setText("Altitude " + QString::number(this->vehiculeData.fAltitude, 'f', 2) + " m");
    ui->statBearingLabel->setText("Bearing: " + QString::number(this->vehiculeData.fBearing, 'f', 2) + "°");
    ui->statDistanceLabel->setText("Distance: " + QString::number(this->vehiculeData.fTargetDistance, 'f', 2) + " m");
    ui->statBatteryLabel->setText("Battery: " + QString::number(this->vehiculeData.u32Battery) + " mV");
    ui->statGPSSpeedLabel->setText("Speed: " + QString::number(this->vehiculeData.fSpeed, 'f', 2) + " m/s");
    ui->statLeftSpeedLabel->setText("LeftSpeed: " + QString::number(this->vehiculeData.u16LeftSpeed * (this->vehiculeData.u8LeftDirection == 0 ? 1 : -1)));
    ui->statRightSpeedLabel->setText("RightSpeed: " + QString::number(this->vehiculeData.u16RightSpeed * (this->vehiculeData.u8RightDirection == 0 ? 1 : -1)));
}

void MainWindow::rawDataChanged(oRawData_t data) {
    this->rawData = data;
    ui->statPitchLabel->setText("Pitch: " + QString::number(rawData.BNO055Data.dPitch));
    ui->statRollLabel->setText("Roll: " + QString::number(rawData.BNO055Data.dRoll));
    ui->statHeadingLabel->setText("Heading: " + QString::number(rawData.BNO055Data.dYaw));
    ui->statLatLabel->setText("Lat: " + QString::number(rawData.CAMM8QData.fLatitude, 'f', 6));
    ui->statLongLabel->setText("Long: " + QString::number(rawData.CAMM8QData.fLongitude, 'f', 6));
    this->m_widgetHSI->setHeading(rawData.BNO055Data.dYaw);
    this->m_widgetADI->setRoll(rawData.BNO055Data.dRoll);
    this->m_widgetADI->setPitch(rawData.BNO055Data.dPitch);

    this->m_widgetHSI->update();
    this->m_widgetADI->update();

    updateNodeForBNO055Data(data.BNO055Data);
    updateNodeForCAMM8QData(data.CAMM8QData);
    updateNodeForBME280Data(data.BME280Data);

}

void MainWindow::configDataChanged(oConfig_t data) {
    this->configData = data;
    ui->param_fReleaseAltitude->setValue(configData.fReleaseAltitude);
    ui->param_fArmingAltitude->setValue(configData.fArmingAltitude);
    ui->param_fHardReleaseAltitude->setValue(configData.fHardReleaseAltitude);
    ui->param_fSoftReleaseAltitude->setValue(configData.fSoftReleaseAltitude);
    ui->param_fTargetRadius->setValue(configData.fTargetRadius);
    ui->param_fTargetLat->setValue(configData.fTargetLat);
    ui->param_fTargetLong->setValue(configData.fTargetLong);

    ui->param_u16HardReleaseCountdown->setValue(configData.u16HardReleaseCountdown);
    ui->param_u16SoftReleaseCountdown->setValue(configData.u16SoftReleaseCountdown);
    ui->param_u16MaxSpeed->setValue(configData.u16MaxSpeed);
    ui->param_u16HeartbeatTransmitInterval->setValue(configData.u16HeartbeatTransmitInterval);
    ui->param_u16SensorTransmitInterval->setValue(configData.u16SensorTransmitInterval);
    ui->param_u8VID->setValue(configData.u8VID);
    ui->param_bSound->setChecked(configData.bSound == 1);
    ui->param_bExternalTelemetry->setChecked(configData.bExternalTelemetry == 1);

    updateNodeForConfigData(data);
}

void MainWindow::readParamFromVehicule() {
    //Send read param command to vehicule
    m_communicationManager->sendReadParamCommand();
}

void MainWindow::sensorViewSetup() {
    treeViewList.setColumnCount(2);
    treeViewList.setHeaderData(0, Qt::Horizontal, "Struct");
    treeViewList.setHeaderData(1, Qt::Horizontal, "Value");

    oBNO055Data_t BNO055Data;
    memset(&BNO055Data, 0, sizeof(oBNO055Data_t));

    oCAMM8QData_t CAMM8QData;
    memset(&CAMM8QData, 0, sizeof(oCAMM8QData_t));

    oBME280Data_t BME280Data;
    memset(&BME280Data, 0, sizeof(oBME280Data_t));

    oConfig_t configData;
    memset(&configData, 0, sizeof(oConfig_t));

    oVehiculeData_t vehiculeData;
    memset(&vehiculeData, 0, sizeof(oVehiculeData_t));

    treeViewList.setItem(0, generateNodeForBNO055Data(BNO055Data));
    treeViewList.setItem(1, generateNodeForCAMM8QData(CAMM8QData));
    treeViewList.setItem(2, generateNodeForBME280Data(BME280Data));
    treeViewList.setItem(3, generateNodeForVehiculeData(vehiculeData));
    treeViewList.setItem(4, generateNodeForConfigData(configData));

    ui->sensorTreeView->setModel(&treeViewList);
    ui->sensorTreeView->setColumnWidth(0, 150);
    ui->sensorTreeView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->sensorTreeView->setFocusPolicy(Qt::NoFocus);
    ui->sensorTreeView->setDragEnabled(false);
}

QStandardItem* MainWindow::generateNodeForBNO055Data(oBNO055Data_t data) {
    QStandardItem *root = new QStandardItem("oBNO055Data_t");
    root->setColumnCount(2);
    root->setChild(0, 0, new QStandardItem("dPitch"));
    root->setChild(0, 1, new QStandardItem(QString::number(data.dPitch, 'f', 2) + "°"));

    root->setChild(1, 0, new QStandardItem("dYaw"));
    root->setChild(1, 1, new QStandardItem(QString::number(data.dYaw, 'f', 2) + "°"));

    root->setChild(2, 0, new QStandardItem("dRoll"));
    root->setChild(2, 1, new QStandardItem(QString::number(data.dRoll, 'f', 2) + "°"));

    root->setChild(3, 0, new QStandardItem("u8SysCal"));
    root->setChild(3, 1, new QStandardItem(QString::number(data.u8SysCal)));

    root->setChild(4, 0, new QStandardItem("u8MagCal"));
    root->setChild(4, 1, new QStandardItem(QString::number(data.u8MagCal)));

    root->setChild(5, 0, new QStandardItem("u8GyrCal"));
    root->setChild(5, 1, new QStandardItem(QString::number(data.u8GyrCal)));

    root->setChild(6, 0, new QStandardItem("u8AccCal"));
    root->setChild(6, 1, new QStandardItem(QString::number(data.u8AccCal)));
    return root;
}

QStandardItem* MainWindow::generateNodeForCAMM8QData(oCAMM8QData_t data) {
    QStandardItem *root = new QStandardItem("oCAMM8QData_t");
    root->setColumnCount(2);
    root->setChild(0, 0, new QStandardItem("fLongitude"));
    root->setChild(0, 1, new QStandardItem(QString::number(data.fLongitude, 'f', 2) + ""));

    root->setChild(1, 0, new QStandardItem("fLatitude"));
    root->setChild(1, 1, new QStandardItem(QString::number(data.fLatitude, 'f', 2) + ""));

    root->setChild(2, 0, new QStandardItem("fMSLAlt"));
    root->setChild(2, 1, new QStandardItem(QString::number(data.fMSLAlt, 'f', 2) + " m"));

    root->setChild(3, 0, new QStandardItem("u8Fix"));
    root->setChild(3, 1, new QStandardItem(QString::number(data.u8Fix) + ""));

    root->setChild(4, 0, new QStandardItem("u8NumSat"));
    root->setChild(4, 1, new QStandardItem(QString::number(data.u8NumSat) + ""));

    return root;
}

QStandardItem* MainWindow::generateNodeForBME280Data(oBME280Data_t data) {
    QStandardItem *root = new QStandardItem("oBME280Data_t");
    root->setColumnCount(2);
    root->setChild(0, 0, new QStandardItem("fTemperature"));
    root->setChild(0, 1, new QStandardItem(QString::number(data.fTemperature, 'f', 2) + " °C"));

    root->setChild(1, 0, new QStandardItem("fHumidity"));
    root->setChild(1, 1, new QStandardItem(QString::number(data.fHumidity, 'f', 2) + " %"));

    root->setChild(2, 0, new QStandardItem("fPressure"));
    root->setChild(2, 1, new QStandardItem(QString::number(data.fPressure, 'f', 2) + " hPa"));

    return root;
}

QStandardItem* MainWindow::generateNodeForVehiculeData(oVehiculeData_t data) {
    QStandardItem *root = new QStandardItem("oVehiculeData_t");
    root->setColumnCount(2);
    root->setChild(0, 0, new QStandardItem("fBearing"));
    root->setChild(0, 1, new QStandardItem(QString::number(data.fBearing, 'f', 2) + "°"));

    root->setChild(1, 0, new QStandardItem("fSpeed"));
    root->setChild(1, 1, new QStandardItem(QString::number(data.fSpeed, 'f', 2) + " m/s"));

    root->setChild(2, 0, new QStandardItem("fTargetDistance"));
    root->setChild(2, 1, new QStandardItem(QString::number(data.fTargetDistance, 'f', 2) + " m"));

    root->setChild(3, 0, new QStandardItem("fAltitude"));
    root->setChild(3, 1, new QStandardItem(QString::number(data.fAltitude, 'f', 2) + " m"));

    root->setChild(4, 0, new QStandardItem("fStartPressure"));
    root->setChild(4, 1, new QStandardItem(QString::number(data.fStartPressure, 'f', 2) + " hpa"));

    root->setChild(5, 0, new QStandardItem("u16LeftSpeed"));
    root->setChild(5, 1, new QStandardItem(QString::number(data.u16LeftSpeed) + ""));

    root->setChild(6, 0, new QStandardItem("u8LeftDirection"));
    root->setChild(6, 1, new QStandardItem(QString::number(data.u8LeftDirection) + ""));

    root->setChild(7, 0, new QStandardItem("u16RightSpeed"));
    root->setChild(7, 1, new QStandardItem(QString::number(data.u16RightSpeed) + ""));

    root->setChild(8, 0, new QStandardItem("u8RightDirection"));
    root->setChild(8, 1, new QStandardItem(QString::number(data.u8RightDirection) + ""));

    root->setChild(9, 0, new QStandardItem("u32Time"));
    root->setChild(9, 1, new QStandardItem(QString::number(data.u32Time) + " ms"));

    root->setChild(10, 0, new QStandardItem("u8Battery"));
    root->setChild(10, 1, new QStandardItem(QString::number(data.u32Battery) + " mv"));

    root->setChild(11, 0, new QStandardItem("u8Mode"));
    root->setChild(11, 1, new QStandardItem(QString::number(data.u8Mode) + ""));

    root->setChild(12, 0, new QStandardItem("u8EStop"));
    root->setChild(12, 1, new QStandardItem(QString::number(data.u8EStop) + ""));

    return root;
}

QStandardItem* MainWindow::generateNodeForConfigData(oConfig_t data) {
    QStandardItem *root = new QStandardItem("oConfig_t");
    root->setColumnCount(2);
    root->setChild(0, 0, new QStandardItem("fReleaseAltitude"));
    root->setChild(0, 1, new QStandardItem(QString::number(data.fReleaseAltitude, 'f', 2) + " m"));

    root->setChild(1, 0, new QStandardItem("fArmingAltitude"));
    root->setChild(1, 1, new QStandardItem(QString::number(data.fArmingAltitude, 'f', 2) + " m"));

    root->setChild(2, 0, new QStandardItem("fHardReleaseAltitude"));
    root->setChild(2, 1, new QStandardItem(QString::number(data.fHardReleaseAltitude, 'f', 2) + " m"));

    root->setChild(3, 0, new QStandardItem("fSoftReleaseAltitude"));
    root->setChild(3, 1, new QStandardItem(QString::number(data.fSoftReleaseAltitude, 'f', 2) + " m"));

    root->setChild(4, 0, new QStandardItem("fTargetRadius"));
    root->setChild(4, 1, new QStandardItem(QString::number(data.fTargetRadius, 'f', 2) + " m"));

    root->setChild(5, 0, new QStandardItem("fTargetLat"));
    root->setChild(5, 1, new QStandardItem(QString::number(data.fTargetLat, 'f', 2) + ""));

    root->setChild(6, 0, new QStandardItem("fTargetLong"));
    root->setChild(6, 1, new QStandardItem(QString::number(data.fTargetLong, 'f', 2) + ""));

    root->setChild(7, 0, new QStandardItem("u16HardReleaseCountdown"));
    root->setChild(7, 1, new QStandardItem(QString::number(data.u16HardReleaseCountdown) + " ms"));

    root->setChild(8, 0, new QStandardItem("u16SoftReleaseCountdown"));
    root->setChild(8, 1, new QStandardItem(QString::number(data.u16SoftReleaseCountdown) + " ms"));

    root->setChild(9, 0, new QStandardItem("u16MaxSpeed"));
    root->setChild(9, 1, new QStandardItem(QString::number(data.u16MaxSpeed) + ""));

    root->setChild(10, 0, new QStandardItem("u16HeartbeatTransmitInterval"));
    root->setChild(10, 1, new QStandardItem(QString::number(data.u16HeartbeatTransmitInterval) + " ms"));

    root->setChild(11, 0, new QStandardItem("u16SensorTransmitInterval"));
    root->setChild(11, 1, new QStandardItem(QString::number(data.u16SensorTransmitInterval) + " ms"));

    root->setChild(12, 0, new QStandardItem("u8VID"));
    root->setChild(12, 1, new QStandardItem(QString::number(data.u8VID) + ""));

    root->setChild(13, 0, new QStandardItem("bSound"));
    root->setChild(13, 1, new QStandardItem(QString::number(data.bSound) + ""));

    root->setChild(14, 0, new QStandardItem("bExternalTelemetry"));
    root->setChild(14, 1, new QStandardItem(QString::number(data.bExternalTelemetry) + ""));

    return root;
}

void MainWindow::updateNodeForBNO055Data(oBNO055Data_t data) {
    QModelIndex rootIdx = ui->sensorTreeView->model()->index(0, 0);         //BNO055Root
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(0, 1, rootIdx), QString::number(data.dPitch, 'f', 2) + "°");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(1, 1, rootIdx), QString::number(data.dYaw, 'f', 2) + "°");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(2, 1, rootIdx), QString::number(data.dRoll, 'f', 2) + "°");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(3, 1, rootIdx), QString::number(data.u8SysCal));
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(4, 1, rootIdx), QString::number(data.u8MagCal));
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(5, 1, rootIdx), QString::number(data.u8GyrCal));
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(6, 1, rootIdx), QString::number(data.u8AccCal));
}

void MainWindow::updateNodeForCAMM8QData(oCAMM8QData_t data) {
    QModelIndex rootIdx = ui->sensorTreeView->model()->index(1, 0);         //CAMM8QRoot
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(0, 1, rootIdx), QString::number(data.fLongitude, 'f', 2) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(1, 1, rootIdx), QString::number(data.fLatitude, 'f', 2) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(2, 1, rootIdx), QString::number(data.fMSLAlt, 'f', 2) + " m");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(3, 1, rootIdx), QString::number(data.u8Fix) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(4, 1, rootIdx), QString::number(data.u8NumSat) + "");
}

void MainWindow::updateNodeForBME280Data(oBME280Data_t data) {
    QModelIndex rootIdx = ui->sensorTreeView->model()->index(2, 0);         //BME280Root
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(0, 1, rootIdx), QString::number(data.fTemperature, 'f', 2) + " °C");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(1, 1, rootIdx), QString::number(data.fHumidity, 'f', 2) + " %");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(2, 1, rootIdx), QString::number(data.fPressure, 'f', 2) + " hPa");
}

void MainWindow::updateNodeForVehiculeData(oVehiculeData_t data) {
    QModelIndex rootIdx = ui->sensorTreeView->model()->index(3, 0);         //VehiculeRoot
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(0, 1, rootIdx), QString::number(data.fBearing, 'f', 2) + "°");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(1, 1, rootIdx), QString::number(data.fSpeed, 'f', 2) + " m/s");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(2, 1, rootIdx), QString::number(data.fTargetDistance, 'f', 2) + " m");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(3, 1, rootIdx), QString::number(data.fAltitude, 'f', 2) + " m");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(4, 1, rootIdx), QString::number(data.fStartPressure, 'f', 2) + " hPa");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(5, 1, rootIdx), QString::number(data.u16LeftSpeed) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(6, 1, rootIdx), QString::number(data.u8LeftDirection) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(7, 1, rootIdx), QString::number(data.u16RightSpeed) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(8, 1, rootIdx), QString::number(data.u8RightDirection) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(9, 1, rootIdx), QString::number(data.u32Time) + " ms");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(10, 1, rootIdx), QString::number(data.u32Battery) + " mv");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(11, 1, rootIdx), QString::number(data.u8Mode) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(12, 1, rootIdx), QString::number(data.u8EStop) + "");
}

void MainWindow::updateNodeForConfigData(oConfig_t data) {
    QModelIndex rootIdx = ui->sensorTreeView->model()->index(4, 0);         //ConfigRoot
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(0, 1, rootIdx), QString::number(data.fReleaseAltitude, 'f', 2) + " m");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(1, 1, rootIdx), QString::number(data.fArmingAltitude, 'f', 2) + " m");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(2, 1, rootIdx), QString::number(data.fHardReleaseAltitude, 'f', 2) + " m");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(3, 1, rootIdx), QString::number(data.fSoftReleaseAltitude, 'f', 2) + " m");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(4, 1, rootIdx), QString::number(data.fTargetRadius, 'f', 2) + " m");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(5, 1, rootIdx), QString::number(data.fTargetLat, 'f', 2) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(6, 1, rootIdx), QString::number(data.fTargetLong, 'f', 2) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(7, 1, rootIdx), QString::number(data.u16HardReleaseCountdown) + " ms");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(8, 1, rootIdx), QString::number(data.u16SoftReleaseCountdown) + " ms");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(9, 1, rootIdx), QString::number(data.u16MaxSpeed) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(10, 1, rootIdx), QString::number(data.u16HeartbeatTransmitInterval) + " ms");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(11, 1, rootIdx), QString::number(data.u16SensorTransmitInterval) + " ms");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(12, 1, rootIdx), QString::number(data.u8VID) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(13, 1, rootIdx), QString::number(data.bSound) + "");
    ui->sensorTreeView->model()->setData(ui->sensorTreeView->model()->index(14, 1, rootIdx), QString::number(data.bExternalTelemetry) + "");
}

void MainWindow::controlViewSetup() {
    connect(ui->resetAltitudeButton, SIGNAL(clicked()), this, SLOT(resetAltitudeClicked()));
    connect(ui->estopButton, SIGNAL(clicked()), this, SLOT(estopButtonClicked()));
    connect(ui->switchModeButton, SIGNAL(clicked()), this, SLOT(switchModeButtonClicked()));
    connect(ui->releaseButton, SIGNAL(clicked()), this, SLOT(resetAltitudeClicked()));
}

void MainWindow::switchModeButtonClicked() {
#ifndef Q_OS_IOS
    if(this->vehiculeData.u8Mode == 0) {
        // Teleop switching to Auto
        QMessageBox msgBox;
        msgBox.setText("Switch to Auto mode?");
        msgBox.setInformativeText("Are you sure you want to enable Auto mode?");
        msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
        msgBox.setDefaultButton(QMessageBox::No);
        msgBox.setIcon(QMessageBox::Warning);
        if (msgBox.exec() == QMessageBox::Yes) {
            uint8_t mode = 1;
            m_communicationManager->sendCommand(SETMODE_CMD, &mode, SETMODE_LEN);
            enableControllerSending = true;
        }

    } else {
        // Auto switiching to Teleop
        uint8_t mode = 0;
        m_communicationManager->sendCommand(SETMODE_CMD, &mode, SETMODE_LEN);
        enableControllerSending = false;
    }
#else
    if(this->vehiculeData.u8Mode == 0) {
        // Teleop switching to Auto
        uint8_t mode = 1;
        m_communicationManager->sendCommand(SETMODE_CMD, &mode, SETMODE_LEN);
        enableControllerSending = true;
    } else {
        // Auto switiching to Teleop
        uint8_t mode = 0;
        m_communicationManager->sendCommand(SETMODE_CMD, &mode, SETMODE_LEN);
        enableControllerSending = false;
    }
#endif
}

void MainWindow::estopButtonClicked() {
    uint8_t estop = 1;
    m_communicationManager->sendCommand(SETESTOP_CMD, &estop, SETESTOP_LEN);
}

void MainWindow::resetAltitudeClicked() {
    uint8_t resetAlt = 1;
    m_communicationManager->sendCommand(SETRESETALT_CMD, &resetAlt, SETRESETALT_LEN);
}

void MainWindow::releaseButtonClicked() {
    uint8_t release = 1;
    m_communicationManager->sendCommand(SETRELEASE_CMD, &release, SETRELEASE_LEN);
}

uint32_t MainWindow::calculateCRC32(QByteArray *ba) {
    static uint32_t u32Table[256];
    static uint8_t u8HaveTable = 0;
    uint32_t u32Rem;
    uint32_t u32CRC = 0;
    uint8_t u8Octet;
    uint32_t i, j;

    /* This check is not thread safe; there is no mutex. */
    if (u8HaveTable == 0) {
        /* Calculate CRC table. */
        for (i = 0; i < 256; i++) {
            u32Rem = i;  /* remainder from polynomial division */
            for (j = 0; j < 8; j++) {
                if (u32Rem & 1) {
                    u32Rem >>= 1;
                    u32Rem ^= 0xedb88320;
                } else
                    u32Rem >>= 1;
            }
            u32Table[i] = u32Rem;
        }
        u8HaveTable = 1;
    }

    u32CRC = ~u32CRC;

    for(i = 0; i < ba->size(); i++) {
        u8Octet = (uint8_t) ba->at(i);
        u32CRC = (u32CRC >> 8) ^ u32Table[(u32CRC & 0xff) ^ u8Octet];
    }

    return ~u32CRC;
}

bool MainWindow::CreateUpgradeFile(QString inFilename, QString outFilename, uint32_t totalSizeInBytes) {
    QFile inFile(inFilename);
    QFile outFile(outFilename);
    QByteArray inFileData;
    uint32_t u32PaddingLength = 0;
    if(inFile.open(QIODevice::ReadOnly)) {
        inFileData = inFile.readAll();
        u32PaddingLength = (totalSizeInBytes - inFileData.size()) - 4;
        if(outFile.open(QIODevice::ReadWrite)) {
            uint8_t blankData[256*1024] = {0};
            QByteArray resultingData;
            resultingData.append(inFileData);
            resultingData.append((const char *)blankData, u32PaddingLength);
            uint32_t calculatedCRC32 = calculateCRC32(&resultingData);

            resultingData.append((uint8_t)(calculatedCRC32 >> 0 & 0xFF));
            resultingData.append((uint8_t)(calculatedCRC32 >> 8 & 0xFF));
            resultingData.append((uint8_t)(calculatedCRC32 >> 16 & 0xFF));
            resultingData.append((uint8_t)(calculatedCRC32 >> 24 & 0xFF));

            outFile.write(resultingData);
        }
        inFile.close();
        outFile.close();
    }
    return false;
}

QString MainWindow::UpgradeFileUIData(QString inFilename, QString outFilename, uint32_t totalSizeInBytes) {
    QString returnText = "";

    QFile inFile(inFilename);
    QFile outFile(outFilename);
    inFile.open(QIODevice::ReadOnly);
    outFile.open(QIODevice::ReadOnly);

    QByteArray ba = outFile.readAll();

    returnText += "Starting at " + QDate::currentDate().toString() + " " + QTime::currentTime().toString() + "\n";
    returnText += "Input File location: " + inFilename + "\n";
    returnText += "Input File Size: " + QString::number(inFile.size()) + " bytes\n";
    returnText += "Padding length: " + QString::number((totalSizeInBytes - inFile.size()) - 4) + " bytes\n\n";


    returnText += "Output File location: " + outFilename + "\n";
    returnText += "Output File Size: " + QString::number(outFile.size()) + " bytes\n";
    returnText += "Output File CRC32: 0x";
    returnText += QString::number((uint8_t)(ba.at(outFile.size() - 4)), 16).toUpper();
    returnText += QString::number((uint8_t)(ba.at(outFile.size() - 3)), 16).toUpper();
    returnText += QString::number((uint8_t)(ba.at(outFile.size() - 2)), 16).toUpper();
    returnText += QString::number((uint8_t)(ba.at(outFile.size() - 1)), 16).toUpper();
    returnText += "\n\n";

    if(totalSizeInBytes == outFile.size()) {
        returnText += "Success\n";
    } else {
        returnText += "Error\n";
    }

    inFile.close();
    outFile.close();

    return returnText;
}



