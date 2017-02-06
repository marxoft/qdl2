QT += network
CONFIG += plugin
TARGET = qdl2-googlerecaptcha
TEMPLATE = lib

HEADERS += googlerecaptchaplugin.h
SOURCES += googlerecaptchaplugin.cpp

maemo5 {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/recaptchaplugin.h

    config.files = "$$TARGET".json
    config.path = /opt/qdl2/plugins/recaptcha

    icon.files = "$$TARGET".jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/recaptcha

    INSTALLS += \
        config \
        icon \
        target

} else:symbian {
    TARGET.UID3 = 0xE71ADC2C
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/recaptchaplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    googlerecaptcha_deployment.pkg_prerules += vendorinfo

    config.sources = "$$TARGET".json
    config.path = !:/qdl2/plugins/recaptcha

    icon.sources = "$$TARGET".jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = "$$TARGET".dll
    lib.path = !:/sys/bin

    stub.sources = "$$TARGET".qtplugin
    stub.path = !:/qdl2/plugins/recaptcha

    DEPLOYMENT.display_name = QDL Plugins Google Recaptcha
    DEPLOYMENT += \
        googlerecaptcha_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/recaptchaplugin.h

    config.files = "$$TARGET".json
    config.path = /usr/share/qdl2/plugins/recaptcha

    icon.files = "$$TARGET".jpg
    icon.path = /usr/share/qdl2/plugins/icons

    target.path = /usr/share/qdl2/plugins/recaptcha

    INSTALLS += \
        config \
        icon \
        target
}
