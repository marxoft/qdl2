TEMPLATE = subdirs

symbian {
    config.sources = sendvid.json
    config.path = !:/qdl2/plugins/services

    icon.sources = sendvid.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = sendvid.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    sendvid_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Sendvid
    DEPLOYMENT += \
        sendvid_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = sendvid.json
    config.path = /opt/qdl2/plugins/services

    icon.files = sendvid.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = sendvid.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
