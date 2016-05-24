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

symbian {
    TARGET.UID3 = 0xE71BE51C
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/decaptchaplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    deathbycaptcha_deployment.pkg_prerules += vendorinfo

    config.sources = deathbycaptcha.json
    config.path = !:/qdl2/plugins/decaptcha

    icon.sources = deathbycaptcha.jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = deathbycaptcha.dll
    lib.path = !:/sys/bin

    stub.sources = deathbycaptcha.qtplugin
    stub.path = !:/qdl2/plugins/decaptcha

    DEPLOYMENT.display_name = QDL Plugins Death By Captcha
    DEPLOYMENT += \
        deathbycaptcha_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/decaptchaplugin.h

    config.files = deathbycaptcha.json
    config.path = /opt/qdl2/plugins/decaptcha

    icon.files = deathbycaptcha.jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/decaptcha
    
    INSTALLS += \
        target \
        config \
        icon
}
