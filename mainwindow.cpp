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

    ui->statusbar->showMessage("Hello World!");

    tabChanged(5);
    connect(ui->tabView, SIGNAL(currentChanged(int)), this, SLOT(tabChanged(int)));

    gamepadViewSetup();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::gamepadViewSetup() {
    connect(m_controllerManager, SIGNAL(controllerStateChanged(QVariant)), this, SLOT(gamepadState(QVariant)));
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
