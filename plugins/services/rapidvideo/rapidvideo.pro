TEMPLATE = subdirs

maemo5 {
    config.files = qdl2-rapidvideo.json
    config.path = /opt/qdl2/plugins/services

    icon.files = qdl2-rapidvideo.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-rapidvideo.js
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin

} else:symbian {
    config.sources = qdl2-rapidvideo.json
    config.path = !:/qdl2/plugins/services

    icon.sources = qdl2-rapidvideo.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-rapidvideo.js
    plugin.path = !:/qdl2/plugins/services

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    rapidvideo_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Rapidvideo
    DEPLOYMENT += \
        rapidvideo_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = qdl2-rapidvideo.json
    config.path = /usr/share/qdl2/plugins/services

    icon.files = qdl2-rapidvideo.jpg
    icon.path = /usr/share/qdl2/plugins/icons

    plugin.files = qdl2-rapidvideo.js
    plugin.path = /usr/share/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}