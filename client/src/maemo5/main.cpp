/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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
#include "downloadrequestmodel.h"
#include "logger.h"
#include "mainwindow.h"
#include "pluginsettings.h"
#include "recaptchapluginmanager.h"
#include "request.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "transfermodel.h"
#include "urlcheckmodel.h"
#include "urlretrievalmodel.h"
#include <QApplication>
#include <QNetworkAccessManager>
#include <QSsl>
#include <QSslConfiguration>

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("QDL-Client");
    app.setApplicationVersion(VERSION_NUMBER);
    
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);

    const QStringList args = app.arguments();
    const int verbosity = args.indexOf("-v") + 1;
    
    if ((verbosity > 1) && (verbosity < args.size())) {
        Logger::setVerbosity(qMax(1, args.at(verbosity).toInt()));
    }
    else {
        Logger::setFileName(Settings::loggerFileName());
        Logger::setVerbosity(Settings::loggerVerbosity());
    }

    QNetworkAccessManager manager;
    Request::setNetworkAccessManager(&manager);

    QScopedPointer<Categories> categories(Categories::instance());
    QScopedPointer<ClipboardUrlModel> clipboard(ClipboardUrlModel::instance());
    QScopedPointer<DecaptchaPluginManager> decaptchaManager(DecaptchaPluginManager::instance());
    QScopedPointer<DownloadRequestModel> requester(DownloadRequestModel::instance());
    QScopedPointer<PluginSettings> pluginSettings(PluginSettings::instance());
    QScopedPointer<RecaptchaPluginManager> recaptchaManager(RecaptchaPluginManager::instance());
    QScopedPointer<ServicePluginManager> serviceManager(ServicePluginManager::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<TransferModel> transfers(TransferModel::instance());
    QScopedPointer<UrlCheckModel> checker(UrlCheckModel::instance());
    QScopedPointer<UrlRetrievalModel> retriever(UrlRetrievalModel::instance());

    clipboard.data()->setEnabled(Settings::clipboardMonitorEnabled());
    clipboard.data()->restore();

    transfers.data()->setAutoReloadEnabled(Settings::autoReloadEnabled());
    
    MainWindow window;
    window.show();
    
    QObject::connect(settings.data(), SIGNAL(clipboardMonitorEnabledChanged(bool)),
            clipboard.data(), SLOT(setEnabled(bool)));
    QObject::connect(settings.data(), SIGNAL(autoReloadEnabledChanged(bool)),
            transfers.data(), SLOT(setAutoReloadEnabled(bool)));
    return app.exec();
}
