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
    src/base/archiveextractor.h \
    src/base/archivepasswordmodel.h \
    src/base/categories.h \
    src/base/categorymodel.h \
    src/base/categoryselectionmodel.h \
    src/base/concurrenttransfersmodel.h \
    src/base/json.h \
    src/base/logger.h \
    src/base/networkproxytypemodel.h \
    src/base/package.h \
    src/base/selectionmodel.h \
    src/base/serviceselectionmodel.h \
    src/base/stringmodel.h \
    src/base/transfer.h \
    src/base/transferitem.h \
    src/base/transferitemprioritymodel.h \
    src/base/transfermodel.h \
    src/base/urlcheckmodel.h \
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
    src/base/archiveextractor.cpp \
    src/base/archivepasswordmodel.cpp \
    src/base/categories.cpp \
    src/base/categorymodel.cpp \
    src/base/json.cpp \
    src/base/logger.cpp \
    src/base/package.cpp \
    src/base/selectionmodel.cpp \
    src/base/stringmodel.cpp \
    src/base/transfer.cpp \
    src/base/transferitem.cpp \
    src/base/transfermodel.cpp \
    src/base/urlcheckmodel.cpp \
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

unix {
    DEFINES += WEB_INTERFACE
    QT += dbus
    #CONFIG += debug
    
    greaterThan(QT_MAJOR_VERSION, 4) {
        QT += widgets
    }
    
    INCLUDEPATH += src/desktop

    HEADERS += \
        src/desktop/aboutdialog.h \
        src/desktop/addurlsdialog.h \
        src/desktop/captchadialog.h \
        src/desktop/categorysettingstab.h \
        src/desktop/clipboardmonitor.h \
        src/desktop/decaptchasettingstab.h \
        src/desktop/generalsettingstab.h \
        src/desktop/interfacesettingstab.h \
        src/desktop/networksettingstab.h \
        src/desktop/mainwindow.h \
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
        src/desktop/urlcheckdialog.h

    SOURCES += \
        src/desktop/aboutdialog.cpp \
        src/desktop/addurlsdialog.cpp \
        src/desktop/captchadialog.cpp \
        src/desktop/categorysettingstab.cpp \
        src/desktop/clipboardmonitor.cpp \
        src/desktop/decaptchasettingstab.cpp \
        src/desktop/generalsettingstab.cpp \
        src/desktop/interfacesettingstab.cpp \
        src/desktop/networksettingstab.cpp \
        src/desktop/main.cpp \
        src/desktop/mainwindow.cpp \
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
        src/desktop/urlcheckdialog.cpp

    desktop.path = /usr/share/applications
    desktop.files = desktop/desktop/qdl2.desktop

    headers.files = \
        src/base/urlresult.h \
        src/plugins/decaptchaplugin.h \
        src/plugins/recaptchaplugin.h \
        src/plugins/serviceplugin.h

    headers.path = /usr/include/qdl2

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
        desktop \
        headers \
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
