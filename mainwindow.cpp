#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    m_controllerManager = new ControllerManager();

    ui->statusbar->showMessage("Hello World!");

    gamepadViewSetup();
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
}

void MainWindow::gamepadViewSetup() {
    connect(m_controllerManager, SIGNAL(controllerStateChanged(QVariant)), this, SLOT(gamepadState(QVariant)));
}

MainWindow::~MainWindow()
{
    delete ui;
}

