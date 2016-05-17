TEMPLATE = subdirs

config.files = xhamster.json
icon.files = xhamster.jpg
plugin.files = xhamster.js

unix {
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
