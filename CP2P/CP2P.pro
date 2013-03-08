TEMPLATE = app
CONFIG += console
CONFIG -= qt

SOURCES += \
    client.cpp \
    server.cpp \
    receivehandler.cpp \
    trame.cpp \
    socketmanager.cpp \
    serverreceivehandler.cpp

HEADERS += \
    receivehandler.h \
    trame.h \
    common.h \
    socketmanager.h \
    serverreceivehandler.h

