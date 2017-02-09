/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "categories.h"
#include "clipboardurlmodel.h"
#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "logger.h"
#include "qdl.h"
#include "recaptchapluginmanager.h"
#include "searchpluginmanager.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "transfermodel.h"
#include "urlcheckmodel.h"
#include "urlretrievalmodel.h"
#include "webserver.h"
#include <QApplication>
#include <QIcon>

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("QDL");
    app.setApplicationVersion(VERSION_NUMBER);
    app.setWindowIcon(QIcon::fromTheme("qdl2"));
    app.setQuitOnLastWindowClosed(false);

    const QStringList args = app.arguments();
    const int verbosity = args.indexOf("-v") + 1;
    
    if ((verbosity > 1) && (verbosity < args.size())) {
        Logger::setVerbosity(qMax(1, args.at(verbosity).toInt()));
    }
    else {
        Logger::setFileName(Settings::loggerFileName());
        Logger::setVerbosity(Settings::loggerVerbosity());
    }

    QScopedPointer<Categories> categories(Categories::instance());
    QScopedPointer<ClipboardUrlModel> clipboard(ClipboardUrlModel::instance());
    QScopedPointer<DecaptchaPluginManager> decaptchaManager(DecaptchaPluginManager::instance());
    QScopedPointer<Qdl> qdl(Qdl::instance());
    QScopedPointer<RecaptchaPluginManager> recaptchaManager(RecaptchaPluginManager::instance());
    QScopedPointer<SearchPluginManager> searchManager(SearchPluginManager::instance());
    QScopedPointer<ServicePluginManager> serviceManager(ServicePluginManager::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<TransferModel> transfers(TransferModel::instance());
    QScopedPointer<UrlCheckModel> checker(UrlCheckModel::instance());
    QScopedPointer<UrlRetrievalModel> retriever(UrlRetrievalModel::instance());
    QScopedPointer<WebServer> server(WebServer::instance());

    Settings::setNetworkProxy();
    
    clipboard.data()->setEnabled(Settings::clipboardMonitorEnabled());
    server.data()->setPort(Settings::webInterfacePort());
    server.data()->setUsername(Settings::webInterfaceUsername());
    server.data()->setPassword(Settings::webInterfacePassword());
    server.data()->setAuthenticationEnabled(Settings::webInterfaceAuthenticationEnabled());

    decaptchaManager.data()->load();
    recaptchaManager.data()->load();
    searchManager.data()->load();
    serviceManager.data()->load();
    clipboard.data()->restore();
    transfers.data()->restore();
    
    if (Settings::webInterfaceEnabled()) {
        server.data()->start();
    }

    if (!args.contains("--nogui")) {
        Qdl::showWindow();
    }

    QObject::connect(settings.data(), SIGNAL(clipboardMonitorEnabledChanged(bool)),
                     clipboard.data(), SLOT(setEnabled(bool)));
    QObject::connect(settings.data(), SIGNAL(webInterfaceAuthenticationEnabledChanged(bool)),
                     server.data(), SLOT(setAuthenticationEnabled(bool)));
    QObject::connect(settings.data(), SIGNAL(webInterfaceUsernameChanged(QString)),
                     server.data(), SLOT(setUsername(QString)));
    QObject::connect(settings.data(), SIGNAL(webInterfacePasswordChanged(QString)),
                     server.data(), SLOT(setPassword(QString)));
    QObject::connect(settings.data(), SIGNAL(webInterfacePortChanged(int)), server.data(), SLOT(setPort(int)));
    QObject::connect(settings.data(), SIGNAL(webInterfaceEnabledChanged(bool)),
                     server.data(), SLOT(setRunning(bool)));
    return app.exec();
}
