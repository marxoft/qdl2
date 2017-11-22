TEMPLATE = subdirs

maemo5 {
    config.files = qdl2-xopenloadsearch.json
    config.path = /opt/qdl2/plugins/search

    icon.files = qdl2-xopenloadsearch.jpg
    icon.path = /opt/qdl2/plugins/icons

    plugin.files = qdl2-xopenloadsearch.js
    plugin.path = /opt/qdl2/plugins/search

    INSTALLS += \
        config \
        icon \
        plugin

} else:symbian {
    config.sources = qdl2-xopenloadsearch.json
    config.path = !:/qdl2/plugins/search

    icon.sources = qdl2-xopenloadsearch.jpg
    icon.path = !:/qdl2/plugins/icons

    plugin.sources = qdl2-xopenloadsearch.js
    plugin.path = !:/qdl2/plugins/search

    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    xopenloadsearch_deployment.pkg_prerules += vendorinfo

    DEPLOYMENT.display_name = QDL Plugins Xopenload Search
    DEPLOYMENT += \
        xopenloadsearch_deployment \
        config \
        icon \
        plugin

} else:unix {
    config.files = qdl2-xopenloadsearch.json
    config.path = /usr/share/qdl2/plugins/search

    icon.files = qdl2-xopenloadsearch.jpg
    icon.path = /usr/share/qdl2/plugins/icons

    plugin.files = qdl2-xopenloadsearch.js
    plugin.path = /usr/share/qdl2/plugins/search

    INSTALLS += \
        config \
        icon \
        plugin
}
