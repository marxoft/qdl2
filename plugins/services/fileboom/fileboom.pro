QT += core network
QT -= gui
CONFIG += plugin
TARGET = fileboom
TEMPLATE = lib

HEADERS += fileboomplugin.h
SOURCES += fileboomplugin.cpp

config.files = fileboom.json
icon.files = fileboom.jpg

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
