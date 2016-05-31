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

#include "actionmodel.h"
#include "categories.h"
#include "categorymodel.h"
#include "categoryselectionmodel.h"
#include "concurrenttransfersmodel.h"
#include "decaptchapluginconfigmodel.h"
#include "definitions.h"
#include "logger.h"
#include "maskeditem.h"
#include "networkproxytypemodel.h"
#include "package.h"
#include "pluginsettings.h"
#include "qdl.h"
#include "recaptchapluginconfigmodel.h"
#include "screenorientationmodel.h"
#include "servicepluginconfigmodel.h"
#include "serviceselectionmodel.h"
#include "settings.h"
#include "transfer.h"
#include "transferitemprioritymodel.h"
#include "transfermodel.h"
#include "transferlistmodel.h"
#include "urlactionmodel.h"
#include "urlcheckmodel.h"
#include "urlretrievalmodel.h"
#include "utils.h"
#include <QApplication>
#include <QDeclarativeContext>
#include <QDeclarativeView>
#include <qdeclarative.h>
#include <QSsl>
#include <QSslConfiguration>

void registerTypes() {
    qmlRegisterType<ActionModel>("Qdl", 2, 0, "ActionModel");
    qmlRegisterType<CategoryModel>("Qdl", 2, 0, "CategoryModel");
    qmlRegisterType<CategorySelectionModel>("Qdl", 2, 0, "CategorySelectionModel");
    qmlRegisterType<ConcurrentTransfersModel>("Qdl", 2, 0, "ConcurrentTransfersModel");
    qmlRegisterType<DecaptchaPluginConfigModel>("Qdl", 2, 0, "DecaptchaPluginConfigModel");
    qmlRegisterType<MaskedItem>("Qdl", 2, 0, "MaskedItem");
    qmlRegisterType<NetworkProxyTypeModel>("Qdl", 2, 0, "NetworkProxyTypeModel");
    qmlRegisterType<PluginSettings>("Qdl", 2, 0, "PluginSettings");
    qmlRegisterType<RecaptchaPluginConfigModel>("Qdl", 2, 0, "RecaptchaPluginConfigModel");
    qmlRegisterType<ScreenOrientationModel>("Qdl", 2, 0, "ScreenOrientationModel");
    qmlRegisterType<SelectionModel>("Qdl", 2, 0, "SelectionModel");
    qmlRegisterType<ServicePluginConfigModel>("Qdl", 2, 0, "ServicePluginConfigModel");
    qmlRegisterType<ServiceSelectionModel>("Qdl", 2, 0, "ServiceSelectionModel");
    qmlRegisterType<TransferItemPriorityModel>("Qdl", 2, 0, "TransferItemPriorityModel");
    qmlRegisterType<TransferListModel>("Qdl", 2, 0, "TransferListModel");
    qmlRegisterType<UrlActionModel>("Qdl", 2, 0, "UrlActionModel");
    
    qmlRegisterUncreatableType<Package>("Qdl", 2, 0, "Package", "");
    qmlRegisterUncreatableType<Qdl>("Qdl", 2, 0, "Qdl", "");
    qmlRegisterUncreatableType<Transfer>("Qdl", 2, 0, "Transfer", "");
    qmlRegisterUncreatableType<TransferItem>("Qdl", 2, 0, "TransferItem", "");
    qmlRegisterUncreatableType<UrlCheckModel>("Qdl", 2, 0, "UrlCheckModel", "");
    qmlRegisterUncreatableType<UrlRetrievalModel>("Qdl", 2, 0, "UrlRetrievalModel", "");
}

Q_DECL_EXPORT int main(int argc, char *argv[]) {
    QApplication app(argc, argv);
    app.setApplicationName("QDL");
    app.setApplicationVersion(VERSION_NUMBER);
    
    QSslConfiguration config = QSslConfiguration::defaultConfiguration();
    config.setProtocol(QSsl::TlsV1);
    QSslConfiguration::setDefaultConfiguration(config);

    Logger::setVerbosity(10);
    
    QScopedPointer<Categories> categories(Categories::instance());
    QScopedPointer<DecaptchaPluginManager> decaptchaManager(DecaptchaPluginManager::instance());
    QScopedPointer<Qdl> qdl(Qdl::instance());
    QScopedPointer<RecaptchaPluginManager> recaptchaManager(RecaptchaPluginManager::instance());
    QScopedPointer<ServicePluginManager> serviceManager(ServicePluginManager::instance());
    QScopedPointer<Settings> settings(Settings::instance());
    QScopedPointer<TransferModel> transfers(TransferModel::instance());
    QScopedPointer<UrlCheckModel> checker(UrlCheckModel::instance());
    QScopedPointer<UrlRetrievalModel> retriever(UrlRetrievalModel::instance());
    
    Utils utils;
    
    registerTypes();
    
    QDeclarativeView view;
    QDeclarativeContext *context = view.rootContext();
    context->setContextProperty("categories", categories.data());
    context->setContextProperty("decaptchaPluginManager", decaptchaManager.data());
    context->setContextProperty("qdl", qdl.data());
    context->setContextProperty("recaptchaPluginManager", recaptchaManager.data());
    context->setContextProperty("servicePluginManager", serviceManager.data());
    context->setContextProperty("settings", settings.data());
    context->setContextProperty("transferModel", transfers.data());
    context->setContextProperty("urlCheckModel", checker.data());
    context->setContextProperty("urlRetrievalModel", retriever.data());
    context->setContextProperty("utils", &utils);
    context->setContextProperty("ACTIVE_COLOR", ACTIVE_COLOR);
    context->setContextProperty("CAPTCHA_TIMEOUT", CAPTCHA_TIMEOUT);
    context->setContextProperty("VERSION_NUMBER", VERSION_NUMBER);
    view.setSource(QUrl::fromLocalFile(QApplication::applicationDirPath() + "/qml/main.qml"));
    view.showFullScreen();

    QObject::connect(&app, SIGNAL(aboutToQuit()), transfers.data(), SLOT(save()));

    return app.exec();
}
