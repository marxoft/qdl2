TEMPLATE = subdirs

symbian {
    config.sources = qdl2-pornhub.json
    config.path = !:/qdl2/plugins/services

    icon.sources = qdl2-pornhub.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-pornhub.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    pornhub_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Pornhub
    DEPLOYMENT += \
        pornhub_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = qdl2-pornhub.json
    config.path = /opt/qdl2/plugins/services

    icon.files = qdl2-pornhub.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-pornhub.js
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
