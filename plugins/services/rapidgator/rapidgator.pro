QT += core network
CONFIG += plugin
TARGET = rapidgator
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += \
    json.h \
    rapidgatorplugin.h

SOURCES += \
    json.cpp \
    rapidgatorplugin.cpp

config.files = rapidgator.json
icon.files = rapidgator.jpg

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
