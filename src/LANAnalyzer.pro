# -------------------------------------------------
# Project created by QtCreator 2009-03-06T06:17:05
# -------------------------------------------------
TARGET = LANAnalyzer
TEMPLATE = app
CONFIG += debug \
    console
QT += network \
    svg
RC_FILE = LANAnalyzer.rc
RESOURCES += images.qrc
INCLUDEPATH += WpdPack/Include
LIBS += -lws2_32 \
    -LWpdPack/Lib \
    -lwpcap
CODECFORTR = UTF-8
SOURCES += main.cpp \
    mainwindow.cpp \
    aboutdialog.cpp \
    devicesdialog.cpp \
    filtersdialog.cpp \
    settingsdialog.cpp \
    transparencydialog.cpp \
    capturethread.cpp \
    startcapturedialog.cpp \
    editordialog.cpp \
    packetsmainwindow.cpp \
    eventsviewermainwindow.cpp \
    netpacketsdialog.cpp \
    portnumbersdialog.cpp \
    graphwidget.cpp \
    nettransferdialog.cpp \
    netpacketsgraphdialog.cpp \
    doublegraphwidget.cpp \
    nettransfergraphdialog.cpp \
    usertransfersgraphdialog.cpp \
    topactivedialog.cpp \
    exportdatadialog.cpp \
    receivercore.cpp \
    summarydialog.cpp \
    settings.cpp \
    myoutputdialog.cpp \
    licensedialog.cpp
HEADERS += mainwindow.h \
    aboutdialog.h \
    devicesdialog.h \
    filtersdialog.h \
    settingsdialog.h \
    transparencydialog.h \
    protocols.h \
    capturethread.h \
    startcapturedialog.h \
    editordialog.h \
    packetsmainwindow.h \
    eventsviewermainwindow.h \
    netpacketsdialog.h \
    portnumbersdialog.h \
    graphwidget.h \
    nettransferdialog.h \
    netpacketsgraphdialog.h \
    doublegraphwidget.h \
    nettransfergraphdialog.h \
    usertransfersgraphdialog.h \
    topactivedialog.h \
    exportdatadialog.h \
    receivercore.h \
    summarydialog.h \
    settings.h \
    myoutputdialog.h \
    licensedialog.h
FORMS += mainwindow.ui \
    aboutdialog.ui \
    devicesdialog.ui \
    filtersdialog.ui \
    settingsdialog.ui \
    transparencydialog.ui \
    startcapturedialog.ui \
    editordialog.ui \
    packetsmainwindow.ui \
    eventsviewermainwindow.ui \
    netpacketsdialog.ui \
    portnumbersdialog.ui \
    nettransferdialog.ui \
    netpacketsgraphdialog.ui \
    nettransfergraphdialog.ui \
    usertransfersgraphdialog.ui \
    topactivedialog.ui \
    exportdatadialog.ui \
    summarydialog.ui \
    myoutputdialog.ui \
    licensedialog.ui
TRANSLATIONS += translations/polish.ts \
    translations/german.ts
