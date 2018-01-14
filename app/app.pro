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
    src/base/captchatype.h \
    src/base/categories.h \
    src/base/categorymodel.h \
    src/base/categoryselectionmodel.h \
    src/base/clipboardurlmodel.h \
    src/base/concurrenttransfersmodel.h \
    src/base/downloadrequester.h \
    src/base/downloadrequestmodel.h \
    src/base/json.h \
    src/base/logger.h \
    src/base/loggerverbositymodel.h \
    src/base/networkproxytypemodel.h \
    src/base/package.h \
    src/base/qdl.h \
    src/base/searchmodel.h \
    src/base/searchresult.h \
    src/base/searchselectionmodel.h \
    src/base/selectionmodel.h \
    src/base/serviceselectionmodel.h \
    src/base/stringmodel.h \
    src/base/transfer.h \
    src/base/transferitem.h \
    src/base/transferitemprioritymodel.h \
    src/base/transfermodel.h \
    src/base/urlchecker.h \
    src/base/urlcheckmodel.h \
    src/base/urlresult.h \
    src/base/urlretrievalmodel.h \
    src/base/urlretriever.h \
    src/base/utils.h \
    src/plugins/decaptchaplugin.h \
    src/plugins/decaptchapluginconfig.h \
    src/plugins/decaptchapluginconfigmodel.h \
    src/plugins/decaptchapluginmanager.h \
    src/plugins/javascriptcaptchatype.h \
    src/plugins/javascriptdecaptchaplugin.h \
    src/plugins/javascriptnetworkrequest.h \
    src/plugins/javascriptpluginengine.h \
    src/plugins/javascriptpluginglobalobject.h \
    src/plugins/javascriptrecaptchaplugin.h \
    src/plugins/javascriptsearchplugin.h \
    src/plugins/javascriptsearchresult.h \
    src/plugins/javascriptserviceplugin.h \
    src/plugins/javascripturlresult.h \
    src/plugins/pluginsettings.h \
    src/plugins/recaptchaplugin.h \
    src/plugins/recaptchapluginconfig.h \
    src/plugins/recaptchapluginconfigmodel.h \
    src/plugins/recaptchapluginmanager.h \
    src/plugins/searchplugin.h \
    src/plugins/searchpluginconfig.h \
    src/plugins/searchpluginconfigmodel.h \
    src/plugins/searchpluginmanager.h \
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
    src/base/clipboardurlmodel.cpp \
    src/base/downloadrequester.cpp \
    src/base/downloadrequestmodel.cpp \
    src/base/json.cpp \
    src/base/logger.cpp \
    src/base/package.cpp \
    src/base/qdl.cpp \
    src/base/searchmodel.cpp \
    src/base/selectionmodel.cpp \
    src/base/stringmodel.cpp \
    src/base/transfer.cpp \
    src/base/transferitem.cpp \
    src/base/transfermodel.cpp \
    src/base/urlchecker.cpp \
    src/base/urlcheckmodel.cpp \
    src/base/urlretrievalmodel.cpp \
    src/base/urlretriever.cpp \
    src/base/utils.cpp \
    src/plugins/decaptchapluginconfig.cpp \
    src/plugins/decaptchapluginconfigmodel.cpp \
    src/plugins/decaptchapluginmanager.cpp \
    src/plugins/javascriptdecaptchaplugin.cpp \
    src/plugins/javascriptnetworkrequest.cpp \
    src/plugins/javascriptpluginengine.cpp \
    src/plugins/javascriptpluginglobalobject.cpp \
    src/plugins/javascriptrecaptchaplugin.cpp \
    src/plugins/javascriptsearchplugin.cpp \
    src/plugins/javascriptsearchresult.cpp \
    src/plugins/javascriptserviceplugin.cpp \
    src/plugins/javascripturlresult.cpp \
    src/plugins/pluginsettings.cpp \
    src/plugins/recaptchapluginconfig.cpp \
    src/plugins/recaptchapluginconfigmodel.cpp \
    src/plugins/recaptchapluginmanager.cpp \
    src/plugins/searchpluginconfig.cpp \
    src/plugins/searchpluginconfigmodel.cpp \
    src/plugins/searchpluginmanager.cpp \
    src/plugins/servicepluginconfig.cpp \
    src/plugins/servicepluginconfigmodel.cpp \
    src/plugins/servicepluginmanager.cpp \
    src/plugins/xmlhttprequest.cpp

maemo5 {
    QT += \
        dbus \
        maemo5 \
        webkit

    INCLUDEPATH += src/maemo5

    HEADERS += \
        src/maemo5/aboutdialog.h \
        src/maemo5/aboutpluginsdialog.h \
        src/maemo5/addurlsdialog.h \
        src/maemo5/archivepasswordsdialog.h \
        src/maemo5/captchadialog.h \
        src/maemo5/categoriesdialog.h \
        src/maemo5/clipboardurlsdialog.h \
        src/maemo5/decaptchapluginsdialog.h \
        src/maemo5/definitions.h \
        src/maemo5/downloadrequestdialog.h \
        src/maemo5/itemdelegate.h \
        src/maemo5/mainwindow.h \
        src/maemo5/multivalueselector.h \
        src/maemo5/networkproxydialog.h \
        src/maemo5/nocaptchadialog.h \
        src/maemo5/packagepropertiesdialog.h \
        src/maemo5/page.h \
        src/maemo5/plugindialog.h \
        src/maemo5/pluginsettingsdialog.h \
        src/maemo5/qmaemo5multilistpickselector.h \
        src/maemo5/recaptchapluginsdialog.h \
        src/maemo5/retrieveurlsdialog.h \
        src/maemo5/searchdialog.h \
        src/maemo5/searchpage.h \
        src/maemo5/searchpluginsdialog.h \
        src/maemo5/servicepluginsdialog.h \
        src/maemo5/settings.h \
        src/maemo5/settingsdialog.h \
        src/maemo5/texteditdialog.h \
        src/maemo5/textinputdialog.h \
        src/maemo5/transferpropertiesdialog.h \
        src/maemo5/urlcheckdialog.h \
        src/maemo5/valueselector.h \
        src/maemo5/valueselectoraction.h

    SOURCES += \
        src/maemo5/aboutdialog.cpp \
        src/maemo5/aboutpluginsdialog.cpp \
        src/maemo5/addurlsdialog.cpp \
        src/maemo5/archivepasswordsdialog.cpp \
        src/maemo5/captchadialog.cpp \
        src/maemo5/categoriesdialog.cpp \
        src/maemo5/clipboardurlsdialog.cpp \
        src/maemo5/decaptchapluginsdialog.cpp \
        src/maemo5/downloadrequestdialog.cpp \
        src/maemo5/itemdelegate.cpp \
        src/maemo5/main.cpp \
        src/maemo5/mainwindow.cpp \
        src/maemo5/multivalueselector.cpp \
        src/maemo5/networkproxydialog.cpp \
        src/maemo5/nocaptchadialog.cpp \
        src/maemo5/packagepropertiesdialog.cpp \
        src/maemo5/page.cpp \
        src/maemo5/plugindialog.cpp \
        src/maemo5/pluginsettingsdialog.cpp \
        src/maemo5/qmaemo5multilistpickselector.cpp \
        src/maemo5/recaptchapluginsdialog.cpp \
        src/maemo5/retrieveurlsdialog.cpp \
        src/maemo5/searchdialog.cpp \
        src/maemo5/searchpage.cpp \
        src/maemo5/searchpluginsdialog.cpp \
        src/maemo5/servicepluginsdialog.cpp \
        src/maemo5/settings.cpp \
        src/maemo5/settingsdialog.cpp \
        src/maemo5/texteditdialog.cpp \
        src/maemo5/textinputdialog.cpp \
        src/maemo5/transferpropertiesdialog.cpp \
        src/maemo5/urlcheckdialog.cpp \
        src/maemo5/valueselector.cpp \
        src/maemo5/valueselectoraction.cpp
        
    dbus.path = /usr/share/dbus-1/services
    dbus.files = dbus/maemo5/org.marxoft.qdl2.service

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

}  else:unix {
    DEFINES += WEB_INTERFACE
    QT += dbus
    
    greaterThan(QT_MAJOR_VERSION, 4) {
        QT += \
            webkitwidgets \
            widgets
    }
    else {
        QT += webkit
    }

    INCLUDEPATH += src/desktop

    HEADERS += \
        src/desktop/aboutdialog.h \
        src/desktop/addurlsdialog.h \
        src/desktop/captchadialog.h \
        src/desktop/categorysettingspage.h \
        src/desktop/clipboardurlsdialog.h \
        src/desktop/definitions.h \
        src/desktop/decaptchasettingspage.h \
        src/desktop/downloadrequestdialog.h \
        src/desktop/generalsettingspage.h \
        src/desktop/interfacesettingspage.h \
        src/desktop/networksettingspage.h \
        src/desktop/nocaptchadialog.h \
        src/desktop/mainwindow.h \
        src/desktop/packagepropertiesdialog.h \
        src/desktop/page.h \
        src/desktop/pluginsettingsdialog.h \
        src/desktop/pluginsettingspage.h \
        src/desktop/recaptchasettingspage.h \
        src/desktop/retrieveurlsdialog.h \
        src/desktop/searchdialog.h \
        src/desktop/searchpage.h \
        src/desktop/searchsettingspage.h \
        src/desktop/servicesettingspage.h \
        src/desktop/settings.h \
        src/desktop/settingsdialog.h \
        src/desktop/settingspage.h \
        src/desktop/texteditdialog.h \
        src/desktop/transferdelegate.h \
        src/desktop/transferpropertiesdialog.h \
        src/desktop/urlcheckdialog.h

    SOURCES += \
        src/desktop/aboutdialog.cpp \
        src/desktop/addurlsdialog.cpp \
        src/desktop/captchadialog.cpp \
        src/desktop/categorysettingspage.cpp \
        src/desktop/clipboardurlsdialog.cpp \
        src/desktop/decaptchasettingspage.cpp \
        src/desktop/downloadrequestdialog.cpp \
        src/desktop/generalsettingspage.cpp \
        src/desktop/interfacesettingspage.cpp \
        src/desktop/networksettingspage.cpp \
        src/desktop/nocaptchadialog.cpp \
        src/desktop/main.cpp \
        src/desktop/mainwindow.cpp \
        src/desktop/packagepropertiesdialog.cpp \
        src/desktop/page.cpp \
        src/desktop/pluginsettingsdialog.cpp \
        src/desktop/pluginsettingspage.cpp \
        src/desktop/recaptchasettingspage.cpp \
        src/desktop/retrieveurlsdialog.cpp \
        src/desktop/searchdialog.cpp \
        src/desktop/searchpage.cpp \
        src/desktop/searchsettingspage.cpp \
        src/desktop/servicesettingspage.cpp \
        src/desktop/settings.cpp \
        src/desktop/settingsdialog.cpp \
        src/desktop/settingspage.cpp \
        src/desktop/texteditdialog.cpp \
        src/desktop/transferdelegate.cpp \
        src/desktop/transferpropertiesdialog.cpp \
        src/desktop/urlcheckdialog.cpp
        
    dbus.path = /usr/share/dbus-1/services
    dbus.files = dbus/desktop/org.marxoft.qdl2.service

    desktop.path = /usr/share/applications
    desktop.files = desktop/desktop/qdl2.desktop

    icon.files = desktop/desktop/64x64/qdl2.png
    icon.path = /usr/share/qdl2/icons

    icon64.files = desktop/desktop/64x64/qdl2.png
    icon64.path = /usr/share/icons/hicolor/64x64/apps

    icon48.files = desktop/desktop/48x48/qdl2.png
    icon48.path = /usr/share/icons/hicolor/48x48/apps

    icon22.files = desktop/desktop/22x22/qdl2.png
    icon22.path = /usr/share/icons/hicolor/22x22/apps

    icon16.files = desktop/desktop/16x16/qdl2.png
    icon16.path = /usr/share/icons/hicolor/16x16/apps
    
    target.path = /usr/bin
    
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
        src/webif/categoryserver.h \
        src/webif/clipboardserver.h \
        src/webif/decaptchaserver.h \
        src/webif/downloadrequestserver.h \
        src/webif/fileserver.h \
        src/webif/recaptchaserver.h \
        src/webif/searchserver.h \
        src/webif/serviceserver.h \
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
        src/webif/categoryserver.cpp \
        src/webif/clipboardserver.cpp \
        src/webif/decaptchaserver.cpp \
        src/webif/downloadrequestserver.cpp \
        src/webif/fileserver.cpp \
        src/webif/recaptchaserver.cpp \
        src/webif/searchserver.cpp \
        src/webif/serviceserver.cpp \
        src/webif/settingsserver.cpp \
        src/webif/transferserver.cpp \
        src/webif/urlcheckserver.cpp \
        src/webif/urlretrievalserver.cpp \
        src/webif/webserver.cpp
}
