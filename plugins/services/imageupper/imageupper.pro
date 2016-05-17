TEMPLATE = subdirs

config.files = imageupper.json
icon.files = imageupper.jpg
plugin.files = imageupper.js

unix {
    config.path = /opt/qdl2/plugins/services
    icon.path = /opt/qdl2/plugins/icons
    plugin.path = /opt/qdl2/plugins/services
    
    INSTALLS += \
        config \
        icon \
        plugin
}
