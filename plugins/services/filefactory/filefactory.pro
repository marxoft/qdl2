QT += core network
CONFIG += plugin
TARGET = filefactory
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += \
    filefactoryplugin.h \
    json.h

SOURCES += \
    filefactoryplugin.cpp \
    json.cpp

config.files = filefactory.json
icon.files = filefactory.jpg

unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h
    
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    target.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        target \
        config \
        icon
}
