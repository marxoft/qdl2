TEMPLATE = subdirs

maemo5 {
    config.files = qdl2-shareonline.json
    config.path = /opt/qdl2/plugins/services

    icon.files = qdl2-shareonline.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-shareonline.js
    plugin.path = /opt/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin

} else:symbian {
    config.sources = qdl2-shareonline.json
    config.path = !:/qdl2/plugins/services

    icon.sources = qdl2-shareonline.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-shareonline.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    shareonline_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Share-Online
    DEPLOYMENT += \
        shareonline_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = qdl2-shareonline.json
    config.path = /usr/share/qdl2/plugins/services

    icon.files = qdl2-shareonline.jpg
    icon.path = /usr/share/qdl2/plugins/icons

    plugin.files = qdl2-shareonline.js
    plugin.path = /usr/share/qdl2/plugins/services

    INSTALLS += \
        config \
        icon \
        plugin
}
