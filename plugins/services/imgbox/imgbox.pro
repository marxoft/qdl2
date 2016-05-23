TEMPLATE = subdirs

symbian {
    config.sources = imgbox.json
    config.path = !:/qdl2/plugins/services

    icon.sources = imgbox.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = imgbox.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    imgbox_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins imgbox
    DEPLOYMENT += \
        imgbox_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = imgbox.json
    config.path = /opt/qdl2/plugins/services

    icon.files = imgbox.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = imgbox.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
