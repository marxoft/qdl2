TEMPLATE = subdirs

config.files = imgbox.json
icon.files = imgbox.jpg
plugin.files = imgbox.js

unix {
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
