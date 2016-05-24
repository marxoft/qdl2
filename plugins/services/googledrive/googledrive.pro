QT += core network
CONFIG += plugin
TARGET = googledrive
TEMPLATE = lib

greaterThan(QT_MAJOR_VERSION, 4) {
    QT -= gui
}

HEADERS += googledriveplugin.h
SOURCES += googledriveplugin.cpp

symbian {
    TARGET.UID3 = 0xE71AEB1C
    TARGET.CAPABILITY += NetworkServices ReadUserData WriteUserData
    TARGET.EPOCALLOWDLLDATA = 1
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000

    INCLUDEPATH += ../../src
    HEADERS += ../../src/serviceplugin.h

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    googledrive_deployment.pkg_prerules += vendorinfo

    config.sources = googledrive.json
    config.path = !:/qdl2/plugins/services

    icon.sources = googledrive.jpg
    icon.path = !:/qdl2/plugins/icons

    lib.sources = googledrive.dll
    lib.path = !:/sys/bin

    stub.sources = googledrive.qtplugin
    stub.path = !:/qdl2/plugins/services

    DEPLOYMENT.display_name = QDL Plugins Google Drive
    DEPLOYMENT += \
        googledrive_deployment \
        config \
        icon \
        lib \
        stub

} else:unix {
    INCLUDEPATH += /usr/include/qdl2
    HEADERS += /usr/include/qdl2/serviceplugin.h
    
    config.files = googledrive.json
    config.path = /opt/qdl2/plugins/services

    icon.files = googledrive.jpg
    icon.path = /opt/qdl2/plugins/icons

    target.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        target \
        config \
        icon
}
