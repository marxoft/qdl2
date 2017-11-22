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
#include "javascriptpluginengine.h"
#include "logger.h"
#include "qdl.h"
#include "recaptchapluginmanager.h"
#include "searchpluginmanager.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "transfermodel.h"
#include "urlcheckmodel.h"
#include "urlretrievalmodel.h"
#include <QApplication>
#include <QSsl>
#include <QSslConfiguration>

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("QDL");
    app.setApplicationVersion(VERSION_NUMBER);
    app.setQuitOnLastWindowClosed(false);
    
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

    QScopedPointer<Categories> categories(Categories::instance());
    QScopedPointer<ClipboardUrlModel> clipboard(ClipboardUrlModel::instance());
    QScopedPointer<DecaptchaPluginManager> decaptchaManager(DecaptchaPluginManager::instance());
    QScopedPointer<JavaScriptPluginEngine> engine(JavaScriptPluginEngine::instance());
    QScopedPointer<Qdl> qdl(Qdl::instance());
    QScopedPointer<RecaptchaPluginManager> recaptchaManager(RecaptchaPluginManager::instance());
    QScopedPointer<SearchPluginManager> searchManager(SearchPluginManager::instance());
    QScopedPointer<ServicePluginManager> serviceManager(ServicePluginManager::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<TransferModel> transfers(TransferModel::instance());
    QScopedPointer<UrlCheckModel> checker(UrlCheckModel::instance());
    QScopedPointer<UrlRetrievalModel> retriever(UrlRetrievalModel::instance());

    Settings::setNetworkProxy();

    clipboard.data()->setEnabled(Settings::clipboardMonitorEnabled());

    decaptchaManager.data()->load();
    recaptchaManager.data()->load();
    searchManager.data()->load();
    serviceManager.data()->load();
    clipboard.data()->restore();
    transfers.data()->restore();
    
    if (!args.contains("--nogui")) {
        Qdl::showWindow();
    }
    
    QObject::connect(settings.data(), SIGNAL(clipboardMonitorEnabledChanged(bool)),
                     clipboard.data(), SLOT(setEnabled(bool)));
    
    return app.exec();
}
