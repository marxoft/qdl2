QT += network
CONFIG += plugin
TARGET = solvemediarecaptcha
TEMPLATE = lib

HEADERS += \
    json.h \
    solvemediarecaptchaplugin.h

SOURCES += \
    json.cpp \
    solvemediarecaptchaplugin.cpp

config.files = solvemediarecaptcha.json
icon.files = solvemediarecaptcha.jpg
    
unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/recaptchaplugin.h
    
    config.path = /opt/qdl2/plugins/recaptcha
    icon.path = /opt/qdl2/plugins/icons
    target.path = /opt/qdl2/plugins/recaptcha
    
    INSTALLS += \
        config \
        icon \
        target
}
