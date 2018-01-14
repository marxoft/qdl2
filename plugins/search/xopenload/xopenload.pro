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
