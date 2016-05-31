TEMPLATE = subdirs

symbian {
    config.sources = qdl2-txxx.json
    config.path = !:/qdl2/plugins/services

    icon.sources = qdl2-txxx.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-txxx.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    txxx_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Txxx
    DEPLOYMENT += \
        txxx_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = qdl2-txxx.json
    config.path = /opt/qdl2/plugins/services

    icon.files = qdl2-txxx.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-txxx.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
