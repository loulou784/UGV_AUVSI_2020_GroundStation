#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include "controllermanager.h"

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

private slots:
    void gamepadState(QVariant data);

private:
    Ui::MainWindow *ui;
    ControllerManager *m_controllerManager;
};
#endif // MAINWINDOW_H
