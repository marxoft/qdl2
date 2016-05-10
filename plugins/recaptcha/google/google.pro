QT += network
CONFIG += plugin
TARGET = googlerecaptcha
TEMPLATE = lib

HEADERS += googlerecaptchaplugin.h
SOURCES +=  googlerecaptchaplugin.cpp

config.files = googlerecaptcha.json
icon.files = googlerecaptcha.jpg
    
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
