QT       += core gui network svg serialport gamepad
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11 sdk_no_version_check

TARGET = "UGV_GroundStation"

# The following define makes your compiler emit warnings if you use
# any Qt feature that has been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    communicationmanager.cpp \
    controllermanager.cpp \
    main.cpp \
    mainwindow.cpp \
    qflightinstruments/LayoutSquare.cpp \
    qflightinstruments/WidgetADI.cpp \
    qflightinstruments/WidgetALT.cpp \
    qflightinstruments/WidgetASI.cpp \
    qflightinstruments/WidgetHSI.cpp \
    qflightinstruments/WidgetNAV.cpp \
    qflightinstruments/WidgetPFD.cpp \
    qflightinstruments/WidgetSix.cpp \
    qflightinstruments/WidgetTC.cpp \
    qflightinstruments/WidgetVSI.cpp \
    qflightinstruments/qfi_ADI.cpp \
    qflightinstruments/qfi_ALT.cpp \
    qflightinstruments/qfi_ASI.cpp \
    qflightinstruments/qfi_HSI.cpp \
    qflightinstruments/qfi_NAV.cpp \
    qflightinstruments/qfi_PFD.cpp \
    qflightinstruments/qfi_TC.cpp \
    qflightinstruments/qfi_VSI.cpp \
    serialcommunicationmanager.cpp \
    tcpcommunicationmanager.cpp \
    tcpvideowidget.cpp \
    websocketvideowidget.cpp

HEADERS += \
    DataTypes.h \
    communicationmanager.h \
    controllermanager.h \
    mainwindow.h \
    qflightinstruments/LayoutSquare.h \
    qflightinstruments/WidgetADI.h \
    qflightinstruments/WidgetALT.h \
    qflightinstruments/WidgetASI.h \
    qflightinstruments/WidgetHSI.h \
    qflightinstruments/WidgetNAV.h \
    qflightinstruments/WidgetPFD.h \
    qflightinstruments/WidgetSix.h \
    qflightinstruments/WidgetTC.h \
    qflightinstruments/WidgetVSI.h \
    qflightinstruments/qfi_ADI.h \
    qflightinstruments/qfi_ALT.h \
    qflightinstruments/qfi_ASI.h \
    qflightinstruments/qfi_HSI.h \
    qflightinstruments/qfi_NAV.h \
    qflightinstruments/qfi_PFD.h \
    qflightinstruments/qfi_TC.h \
    qflightinstruments/qfi_VSI.h \
    serialcommunicationmanager.h \
    tcpcommunicationmanager.h \
    tcpvideowidget.h \
    websocketvideowidget.h

FORMS += \
    mainwindow.ui \
    qflightinstruments/WidgetADI.ui \
    qflightinstruments/WidgetALT.ui \
    qflightinstruments/WidgetASI.ui \
    qflightinstruments/WidgetHSI.ui \
    qflightinstruments/WidgetNAV.ui \
    qflightinstruments/WidgetPFD.ui \
    qflightinstruments/WidgetSix.ui \
    qflightinstruments/WidgetTC.ui \
    qflightinstruments/WidgetVSI.ui


# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

ios {
    QMAKE_INFO_PLIST = Info.plist
    QT -= serialport
    SOURCES -= serialcommunicationmanager.cpp
    HEADERS -= serialcommunicationmanager.h
}

wasm {
    message(Building WASM)
    QT -= serialport \
        gamepad

    SOURCES -= serialcommunicationmanager.cpp \
            controllermanager.cpp \

    HEADERS -= serialcommunicationmanager.h
            controllermanager.h \
}

RESOURCES += qdarkstyle/style.qrc

RESOURCES += \
    qflightinstruments/qfi.qrc

macx: ICON = SmallRoundedGroundStation.icns
