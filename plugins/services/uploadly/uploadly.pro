TEMPLATE = subdirs

maemo5 {
    config.files = qdl2-uploadly.json
    config.path = /opt/qdl2/plugins/services

    icon.files = qdl2-uploadly.png
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-uploadly.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin

} else:symbian {
    config.sources = qdl2-uploadly.json
    config.path = !:/qdl2/plugins/services

    icon.sources = qdl2-uploadly.png
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-uploadly.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    uploadly_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Uploadly
    DEPLOYMENT += \
        uploadly_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = qdl2-uploadly.json
    config.path = /usr/share/qdl2/plugins/services

    icon.files = qdl2-uploadly.png
    icon.path = /usr/share/qdl2/plugins/icons

    plugin.files = qdl2-uploadly.js
    plugin.path = /usr/share/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
