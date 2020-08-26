#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    m_widgetADI = new WidgetADI();
    m_widgetHSI = new WidgetHSI();

    ui->adiLayout->addWidget(m_widgetADI);
    ui->hsiLayout->addWidget(m_widgetHSI);

#ifdef Q_OS_IOS
    ui->SerialGroupBox->setVisible(false);
#endif

    m_controllerManager = new ControllerManager();
    m_communicationManager = new CommunicationManager();


    connect(m_communicationManager, SIGNAL(connectionChanged(bool, bool)), this, SLOT(communicationConnectionChanged(bool, bool)));
    connect(ui->firmwareSelectButton, SIGNAL(clicked()), this, SLOT(openFirmwareFilePicker()));

    connect(ui->TCPConnectButton, SIGNAL(clicked()), this, SLOT(connectTCP()));
    connect(ui->TCPDisconnectButton, SIGNAL(clicked()), m_communicationManager, SLOT(stopTCPComm()));

#ifndef Q_OS_IOS
    connect(ui->SerialConnectButton, SIGNAL(clicked()), this, SLOT(connectSerial()));
    connect(ui->SerialDisconnectButton, SIGNAL(clicked()), m_communicationManager, SLOT(stopSerialComm()));
#endif

    tabChanged(5);
    connect(ui->tabView, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    connect(m_communicationManager, SIGNAL(receivedHeartbeat(uint32_t, uint32_t)), this, SLOT(heartbeatChanged(uint32_t, uint32_t)));
    connect(m_communicationManager, SIGNAL(receivedRawData(oRawData_t)), this, SLOT(rawDataChanged(oRawData_t)));
    connect(m_communicationManager, SIGNAL(receivedConfigData(oConfig_t)), this, SLOT(configDataChanged(oConfig_t)));

    paramViewSetup();
    gamepadViewSetup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::gamepadViewSetup() {
    connect(m_controllerManager, SIGNAL(controllerStateChanged(QVariant)), this, SLOT(gamepadState(QVariant)));
}

void MainWindow::paramViewSetup() {
    connect(ui->paramWriteParamButton, SIGNAL(clicked()), this, SLOT(writeParamToVehicule()));
    connect(ui->paramReadButton, SIGNAL(clicked()), this, SLOT(readParamFromVehicule()));

}

void MainWindow::gamepadState(QVariant data) {
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
    /*
        uint8_t controllerData[8];
        uint8_t len = 0;
        m_controllerManager->convertToArray(controllerData, &len);
        m_communicationManager->sendControllerData(controllerData, len);
    */
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
    if(index == 5) {

#ifndef Q_OS_IOS
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
        QString filename = QFileDialog::getOpenFileName(this, tr("SelectFirmware"), "/", tr("Binary files (*.bin)"));
        ui->firmwarePath->setText(filename);
}

void MainWindow::connectTCP() {
    m_communicationManager->startTCPComm(ui->hostIPLineEdit->text(), ui->portSpinBox->value());
}

void MainWindow::connectSerial() {
#ifndef Q_OS_IOS
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

void MainWindow::rawDataChanged(oRawData_t data) {
    this->rawData = data;
    ui->statPitchLabel->setText("Pitch: " + QString::number(rawData.BNO055Data.dPitch));
    ui->statRollLabel->setText("Roll: " + QString::number(rawData.BNO055Data.dRoll));
    ui->statHeadingLabel->setText("Heading: " + QString::number(rawData.BNO055Data.dYaw));
    ui->statLatLabel->setText("Lat: " + QString::number(rawData.CAMM8QData.fLatitude));
    ui->statLongLabel->setText("Long: " + QString::number(rawData.CAMM8QData.fLongitude));
    this->m_widgetHSI->setHeading(rawData.BNO055Data.dYaw);
    this->m_widgetADI->setRoll(rawData.BNO055Data.dRoll);
    this->m_widgetADI->setPitch(rawData.BNO055Data.dPitch);


    this->m_widgetHSI->update();
    this->m_widgetADI->update();
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
}

void MainWindow::readParamFromVehicule() {
    //Send read param command to vehicule
    m_communicationManager->sendReadParamCommand();
}
