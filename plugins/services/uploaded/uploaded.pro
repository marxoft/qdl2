QT += core network
QT -= gui
CONFIG += plugin
TARGET = uploaded
TEMPLATE = lib

HEADERS += \
    json.h \
    uploadedplugin.h

SOURCES += \
    json.cpp \
    uploadedplugin.cpp

config.files = uploaded.json
icon.files = uploaded.jpg

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
