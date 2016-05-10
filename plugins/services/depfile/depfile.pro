QT += core network
QT -= gui
CONFIG += plugin
TARGET = depfile
TEMPLATE = lib

HEADERS += depfileplugin.h
SOURCES += depfileplugin.cpp

config.files = depfile.json
icon.files = depfile.jpg

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
