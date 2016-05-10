QT += core network
CONFIG += plugin
TARGET = deathbycaptcha
TEMPLATE = lib

HEADERS += \
    deathbycaptchaplugin.h \
    json.h

SOURCES += \
    json.cpp \
    deathbycaptchaplugin.cpp

config.files = deathbycaptcha.json
icon.files = deathbycaptcha.jpg

unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/decaptchaplugin.h

    config.path = /opt/qdl2/plugins/decaptcha
    icon.path = /opt/qdl2/plugins/icons
    target.path = /opt/qdl2/plugins/decaptcha
    
    INSTALLS += \
        target \
        config \
        icon
}
