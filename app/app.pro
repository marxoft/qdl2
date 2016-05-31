TEMPLATE = app
TARGET = qdl2

QT += \
    network \
    script

INCLUDEPATH += \
    src/base \
    src/plugins

HEADERS += \
    src/base/actionmodel.h \
    src/base/categories.h \
    src/base/categorymodel.h \
    src/base/categoryselectionmodel.h \
    src/base/concurrenttransfersmodel.h \
    src/base/json.h \
    src/base/logger.h \
    src/base/networkproxytypemodel.h \
    src/base/selectionmodel.h \
    src/base/serviceselectionmodel.h \
    src/base/stringmodel.h \
    src/base/transfer.h \
    src/base/transferitem.h \
    src/base/transferitemprioritymodel.h \
    src/base/transfermodel.h \
    src/base/urlactionmodel.h \
    src/base/urlresult.h \
    src/base/urlretrievalmodel.h \
    src/base/urlretriever.h \
    src/base/utils.h \
    src/plugins/decaptchaplugin.h \
    src/plugins/decaptchapluginconfig.h \
    src/plugins/decaptchapluginconfigmodel.h \
    src/plugins/decaptchapluginmanager.h \
    src/plugins/javascriptdecaptchaplugin.h \
    src/plugins/javascriptpluginglobalobject.h \
    src/plugins/javascriptrecaptchaplugin.h \
    src/plugins/javascriptserviceplugin.h \
    src/plugins/pluginsettings.h \
    src/plugins/recaptchaplugin.h \
    src/plugins/recaptchapluginconfig.h \
    src/plugins/recaptchapluginconfigmodel.h \
    src/plugins/recaptchapluginmanager.h \
    src/plugins/serviceplugin.h \
    src/plugins/servicepluginconfig.h \
    src/plugins/servicepluginconfigmodel.h \
    src/plugins/servicepluginmanager.h \
    src/plugins/xmlhttprequest.h

SOURCES += \
    src/base/categories.cpp \
    src/base/categorymodel.cpp \
    src/base/json.cpp \
    src/base/logger.cpp \
    src/base/selectionmodel.cpp \
    src/base/stringmodel.cpp \
    src/base/transfer.cpp \
    src/base/transferitem.cpp \
    src/base/transfermodel.cpp \
    src/base/urlretrievalmodel.cpp \
    src/base/urlretriever.cpp \
    src/base/utils.cpp \
    src/plugins/decaptchapluginconfig.cpp \
    src/plugins/decaptchapluginconfigmodel.cpp \
    src/plugins/decaptchapluginmanager.cpp \
    src/plugins/javascriptdecaptchaplugin.cpp \
    src/plugins/javascriptpluginglobalobject.cpp \
    src/plugins/javascriptrecaptchaplugin.cpp \
    src/plugins/javascriptserviceplugin.cpp \
    src/plugins/pluginsettings.cpp \
    src/plugins/recaptchapluginconfig.cpp \
    src/plugins/recaptchapluginconfigmodel.cpp \
    src/plugins/recaptchapluginmanager.cpp \
    src/plugins/servicepluginconfig.cpp \
    src/plugins/servicepluginconfigmodel.cpp \
    src/plugins/servicepluginmanager.cpp \
    src/plugins/xmlhttprequest.cpp

maemo5 {
    QT += \
        dbus \
        maemo5

    INCLUDEPATH += src/maemo5

    HEADERS += \
        src/maemo5/aboutdialog.h \
        src/maemo5/aboutpluginsdialog.h \
        src/maemo5/addurlsdialog.h \
        src/maemo5/archiveextractor.h \
        src/maemo5/archivepasswordmodel.h \
        src/maemo5/archivepasswordsdialog.h \
        src/maemo5/captchadialog.h \
        src/maemo5/categoriesdialog.h \
        src/maemo5/clipboardurlmodel.h \
        src/maemo5/clipboardurlsdialog.h \
        src/maemo5/decaptchadialog.h \
        src/maemo5/definitions.h \
        src/maemo5/mainwindow.h \
        src/maemo5/networkproxydialog.h \
        src/maemo5/package.h \
        src/maemo5/packagepropertiesdialog.h \
        src/maemo5/plugindialog.h \
        src/maemo5/pluginsettingsdialog.h \
        src/maemo5/qdl.h \
        src/maemo5/recaptchadialog.h \
        src/maemo5/retrieveurlsdialog.h \
        src/maemo5/servicesdialog.h \
        src/maemo5/settings.h \
        src/maemo5/settingsdialog.h \
        src/maemo5/textinputdialog.h \
        src/maemo5/transferpropertiesdialog.h \
        src/maemo5/urlcheckdialog.h \
        src/maemo5/urlcheckmodel.h \
        src/maemo5/valueselector.h \
        src/maemo5/valueselectoraction.h

    SOURCES += \
        src/maemo5/aboutdialog.cpp \
        src/maemo5/aboutpluginsdialog.cpp \
        src/maemo5/addurlsdialog.cpp \
        src/maemo5/archiveextractor.cpp \
        src/maemo5/archivepasswordmodel.cpp \
        src/maemo5/archivepasswordsdialog.cpp \
        src/maemo5/captchadialog.cpp \
        src/maemo5/categoriesdialog.cpp \
        src/maemo5/clipboardurlmodel.cpp \
        src/maemo5/clipboardurlsdialog.cpp \
        src/maemo5/decaptchadialog.cpp \
        src/maemo5/main.cpp \
        src/maemo5/mainwindow.cpp \
        src/maemo5/networkproxydialog.cpp \
        src/maemo5/package.cpp \
        src/maemo5/packagepropertiesdialog.cpp \
        src/maemo5/plugindialog.cpp \
        src/maemo5/pluginsettingsdialog.cpp \
        src/maemo5/qdl.cpp \
        src/maemo5/recaptchadialog.cpp \
        src/maemo5/retrieveurlsdialog.cpp \
        src/maemo5/servicesdialog.cpp \
        src/maemo5/settings.cpp \
        src/maemo5/settingsdialog.cpp \
        src/maemo5/textinputdialog.cpp \
        src/maemo5/transferpropertiesdialog.cpp \
        src/maemo5/urlcheckdialog.cpp \
        src/maemo5/urlcheckmodel.cpp \
        src/maemo5/valueselector.cpp \
        src/maemo5/valueselectoraction.cpp
        
    dbus.path = /usr/share/dbus-1/services
    dbus.files = dbus/org.marxoft.qdl2.service

    desktop.path = /usr/share/applications/hildon
    desktop.files = desktop/maemo5/qdl2.desktop

    icon.files = desktop/maemo5/64x64/qdl2.png
    icon.path = /opt/qdl2/icons

    icon64.files = desktop/desktop/64x64/qdl2.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps
    
    target.path = /opt/qdl2/bin
    
    INSTALLS += \
        dbus \
        desktop \
        icon \
        icon64 \
        target

} else:symbian|simulator {
    TARGET = qdl2
    TARGET.UID3 = 0xE71CBF5C
    TARGET.CAPABILITY += NetworkServices ReadUserData
    TARGET.EPOCHEAPSIZE = 0x20000 0x8000000
    TARGET.EPOCSTACKSIZE = 0x14000
    
    VERSION = 2.1.0
    ICON = desktop/symbian/qdl2.svg
    
    MMP_RULES += "DEBUGGABLE_UDEBONLY"

    CONFIG += qtcomponents
    
    QT += declarative
    
    INCLUDEPATH += src/symbian
    
    HEADERS += \
        src/symbian/definitions.h \
        src/symbian/graphicsview.h \
        src/symbian/maskeditem.h \
        src/symbian/maskeffect.h \
        src/symbian/package.h \
        src/symbian/qdl.h \
        src/symbian/screenorientationmodel.h \
        src/symbian/settings.h \
        src/symbian/transferlistmodel.h \
        src/symbian/urlcheckmodel.h
    
    SOURCES += \
        src/symbian/main.cpp \
        src/symbian/maskeditem.cpp \
        src/symbian/maskeffect.cpp \
        src/symbian/package.cpp \
        src/symbian/qdl.cpp \
        src/symbian/settings.cpp \
        src/symbian/transferlistmodel.cpp \
        src/symbian/urlcheckmodel.cpp
    
    qml.sources = $$files(src/symbian/qml/*.qml)
    qml.path = !:/Private/e71cbf5c/qml

    images.sources = $$files(src/symbian/qml/images/*.png)
    images.path = !:/Private/e71cbf5c/qml/images

    icon.sources = desktop/symbian/qdl2.png
    icon.path = C:/qdl2/icons
    
    vendorinfo += "%{\"Stuart Howarth\"}" ":\"Stuart Howarth\""
    qtcomponentsdep = "; Default dependency to Qt Quick Components for Symbian library" \
        "(0x200346DE), 1, 1, 0, {\"Qt Quick components for Symbian\"}"

    qdl2_deployment.pkg_prerules += \
        vendorinfo \
        qtcomponentsdep

    DEPLOYMENT.display_name = QDL
    DEPLOYMENT += \
        qdl2_deployment \
        qml \
        images \
        icon

} else:unix {
    DEFINES += WEB_INTERFACE
    QT += dbus
    CONFIG += debug
    
    greaterThan(QT_MAJOR_VERSION, 4) {
        QT += widgets
    }
    
    INCLUDEPATH += src/desktop

    HEADERS += \
        src/desktop/aboutdialog.h \
        src/desktop/addurlsdialog.h \
        src/desktop/archiveextractor.h \
        src/desktop/archivepasswordmodel.h \
        src/desktop/captchadialog.h \
        src/desktop/categorysettingstab.h \
        src/desktop/clipboardurlmodel.h \
        src/desktop/clipboardurlsdialog.h \
        src/desktop/definitions.h \
        src/desktop/decaptchasettingstab.h \
        src/desktop/generalsettingstab.h \
        src/desktop/interfacesettingstab.h \
        src/desktop/networksettingstab.h \
        src/desktop/mainwindow.h \
        src/desktop/package.h \
        src/desktop/packagepropertiesdialog.h \
        src/desktop/pluginsettingsdialog.h \
        src/desktop/pluginsettingstab.h \
        src/desktop/qdl.h \
        src/desktop/recaptchasettingstab.h \
        src/desktop/retrieveurlsdialog.h \
        src/desktop/servicesettingstab.h \
        src/desktop/settings.h \
        src/desktop/settingsdialog.h \
        src/desktop/settingstab.h \
        src/desktop/transferdelegate.h \
        src/desktop/transferpropertiesdialog.h \
        src/desktop/urlcheckdialog.h \
        src/desktop/urlcheckmodel.h

    SOURCES += \
        src/desktop/aboutdialog.cpp \
        src/desktop/addurlsdialog.cpp \
        src/desktop/archiveextractor.cpp \
        src/desktop/archivepasswordmodel.cpp \
        src/desktop/captchadialog.cpp \
        src/desktop/categorysettingstab.cpp \
        src/desktop/clipboardurlmodel.cpp \
        src/desktop/clipboardurlsdialog.cpp \
        src/desktop/decaptchasettingstab.cpp \
        src/desktop/generalsettingstab.cpp \
        src/desktop/interfacesettingstab.cpp \
        src/desktop/networksettingstab.cpp \
        src/desktop/main.cpp \
        src/desktop/mainwindow.cpp \
        src/desktop/package.cpp \
        src/desktop/packagepropertiesdialog.cpp \
        src/desktop/pluginsettingsdialog.cpp \
        src/desktop/pluginsettingstab.cpp \
        src/desktop/qdl.cpp \
        src/desktop/recaptchasettingstab.cpp \
        src/desktop/retrieveurlsdialog.cpp \
        src/desktop/servicesettingstab.cpp \
        src/desktop/settings.cpp \
        src/desktop/settingsdialog.cpp \
        src/desktop/settingstab.cpp \
        src/desktop/transferdelegate.cpp \
        src/desktop/transferpropertiesdialog.cpp \
        src/desktop/urlcheckdialog.cpp \
        src/desktop/urlcheckmodel.cpp
        
    dbus.path = /usr/share/dbus-1/services
    dbus.files = dbus/org.marxoft.qdl2.service

    desktop.path = /usr/share/applications
    desktop.files = desktop/desktop/qdl2.desktop

    icon.files = desktop/desktop/64x64/qdl2.png
    icon.path = /opt/qdl2/icons

    icon64.files = desktop/desktop/64x64/qdl2.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps

    icon48.files = desktop/desktop/48x48/qdl2.png
    icon48.path = /usr/share/icons/hicolor/48x48/apps

    icon22.files = desktop/desktop/22x22/qdl2.png
    icon22.path = /usr/share/icons/hicolor/22x22/apps

    icon16.files = desktop/desktop/16x16/qdl2.png
    icon16.path = /usr/share/icons/hicolor/16x16/apps
    
    target.path = /opt/qdl2/bin
    
    INSTALLS += \
        dbus \
        desktop \
        icon \
        icon16 \
        icon22 \
        icon48 \
        icon64 \
        target
}

contains(DEFINES, WEB_INTERFACE) {
    INCLUDEPATH += \
        src/qhttpserver \
        src/webif
    
    HEADERS += \
        src/qhttpserver/http_parser.h \
        src/qhttpserver/qhttpconnection.h \
        src/qhttpserver/qhttprequest.h \
        src/qhttpserver/qhttpresponse.h \
        src/qhttpserver/qhttpserver.h \
        src/qhttpserver/qhttpserverapi.h \
        src/qhttpserver/qhttpserverfwd.h \
        src/webif/applicationserver.h \
        src/webif/categoryserver.h \
        src/webif/decaptchapluginconfigserver.h \
        src/webif/fileserver.h \
        src/webif/recaptchapluginconfigserver.h \
        src/webif/serverresponse.h \
        src/webif/servicepluginconfigserver.h \
        src/webif/settingsserver.h \
        src/webif/transferserver.h \
        src/webif/urlcheckserver.h \
        src/webif/urlretrievalserver.h \
        src/webif/webserver.h
    
    SOURCES += \
        src/qhttpserver/http_parser.c \
        src/qhttpserver/qhttpconnection.cpp \
        src/qhttpserver/qhttprequest.cpp \
        src/qhttpserver/qhttpresponse.cpp \
        src/qhttpserver/qhttpserver.cpp \
        src/webif/applicationserver.cpp \
        src/webif/categoryserver.cpp \
        src/webif/decaptchapluginconfigserver.cpp \
        src/webif/fileserver.cpp \
        src/webif/recaptchapluginconfigserver.cpp \
        src/webif/servicepluginconfigserver.cpp \
        src/webif/settingsserver.cpp \
        src/webif/transferserver.cpp \
        src/webif/urlcheckserver.cpp \
        src/webif/urlretrievalserver.cpp \
        src/webif/webserver.cpp
}
