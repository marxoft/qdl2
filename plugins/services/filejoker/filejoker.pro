TEMPLATE = subdirs

maemo5 {
    config.files = qdl2-filejoker.json
    config.path = /opt/qdl2/plugins/services

    icon.files = qdl2-filejoker.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-filejoker.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin

} else:symbian {
    config.sources = qdl2-filejoker.json
    config.path = !:/qdl2/plugins/services

    icon.sources = qdl2-filejoker.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-filejoker.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    filejoker_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins FileJoker
    DEPLOYMENT += \
        filejoker_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = qdl2-filejoker.json
    config.path = /usr/share/qdl2/plugins/services

    icon.files = qdl2-filejoker.jpg
    icon.path = /usr/share/qdl2/plugins/icons

    plugin.files = qdl2-filejoker.js
    plugin.path = /usr/share/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
