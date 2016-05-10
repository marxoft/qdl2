QT += core network
QT -= gui
CONFIG += plugin
TARGET = keeptoshare
TEMPLATE = lib

HEADERS += keeptoshareplugin.h
SOURCES += keeptoshareplugin.cpp

config.files = keeptoshare.json
icon.files = keeptoshare.jpg

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
