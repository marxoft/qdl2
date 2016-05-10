QT += core network
QT -= gui
CONFIG += plugin
TARGET = publishtome
TEMPLATE = lib

HEADERS += publishtomeplugin.h
SOURCES += publishtomeplugin.cpp

config.files = publishtome.json
icon.files = publishtome.jpg

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
