QT += core network script
QT -= gui
CONFIG += plugin
TARGET = depositfiles
TEMPLATE = lib

HEADERS += depositfilesplugin.h
SOURCES += depositfilesplugin.cpp

config.files = depositfiles.json
icon.files = depositfiles.jpg

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
