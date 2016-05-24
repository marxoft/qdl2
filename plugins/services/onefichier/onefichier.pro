QT += core network
CONFIG += plugin
TARGET = onefichier
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += onefichierplugin.h
SOURCES += onefichierplugin.cpp

symbian {
    TARGET.UID3 = 0xE72FA1B4
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/serviceplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    onefichier_deployment.pkg_prerules += vendorinfo

    config.sources = onefichier.json
    config.path = !:/qdl2/plugins/services

    icon.sources = onefichier.jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = onefichier.dll
    lib.path = !:/sys/bin

    stub.sources = onefichier.qtplugin
    stub.path = !:/qdl2/plugins/services

    DEPLOYMENT.display_name = QDL Plugins 1Fichier
    DEPLOYMENT += \
        onefichier_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h

    config.files = onefichier.json
    config.path = /opt/qdl2/plugins/services

    icon.files = onefichier.jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/services

    INSTALLS += \
        target \
        config \
        icon
}
