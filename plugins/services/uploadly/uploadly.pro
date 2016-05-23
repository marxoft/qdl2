TEMPLATE = subdirs

symbian {
    config.sources = uploadly.json
    config.path = !:/qdl2/plugins/services

    icon.sources = uploadly.png
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = uploadly.js
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
    config.files = uploadly.json
    config.path = /opt/qdl2/plugins/services

    icon.files = uploadly.png
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = uploadly.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
