QT += core network
CONFIG += plugin
TARGET = keeptoshare
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += keeptoshareplugin.h
SOURCES += keeptoshareplugin.cpp

symbian {
    TARGET.UID3 = 0xE74BA3AC
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/serviceplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    keeptoshare_deployment.pkg_prerules += vendorinfo

    config.sources = keeptoshare.json
    config.path = !:/qdl2/plugins/services

    icon.sources = keeptoshare.jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = keeptoshare.dll
    lib.path = !:/sys/bin

    stub.sources = keeptoshare.qtplugin
    stub.path = !:/qdl2/plugins/services

    DEPLOYMENT.display_name = QDL Plugins Keep2Share
    DEPLOYMENT += \
        keeptoshare_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h

    config.files = keeptoshare.json
    config.path = /opt/qdl2/plugins/services

    icon.files = keeptoshare.jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/services

    INSTALLS += \
        target \
        config \
        icon
}
