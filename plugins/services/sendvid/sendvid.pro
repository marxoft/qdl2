TEMPLATE = subdirs

config.files = sendvid.json
icon.files = sendvid.jpg
plugin.files = sendvid.js

unix {
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
