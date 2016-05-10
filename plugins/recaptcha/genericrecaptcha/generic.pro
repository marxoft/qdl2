QT += network
CONFIG += plugin
TARGET = genericrecaptcha
TEMPLATE = lib

HEADERS += genericrecaptchaplugin.h
SOURCES += genericrecaptchaplugin.cpp

config.files = genericrecaptcha.json
icon.files = genericrecaptcha.jpg
    
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
