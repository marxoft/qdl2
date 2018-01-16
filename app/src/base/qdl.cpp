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

#include "qdl.h"
#include "categories.h"
#include "clipboardurlmodel.h"
#include "decaptchapluginmanager.h"
#include "downloadrequestmodel.h"
#include "logger.h"
#include "mainwindow.h"
#include "pluginsettings.h"
#include "recaptchapluginmanager.h"
#include "searchpluginmanager.h"
#include "servicepluginmanager.h"
#include "settings.h"
#include "transfermodel.h"
#include "urlcheckmodel.h"
#include "urlretrievalmodel.h"
#include <QCoreApplication>
#include <QDBusConnection>

Qdl* Qdl::self = 0;

QPointer<MainWindow> Qdl::window = 0;

static QModelIndex getTransferModelIndexById(const QString &id) {
    const QModelIndexList indexes =
    TransferModel::instance()->match(TransferModel::instance()->index(0, 0, QModelIndex()), TransferItem::IdRole, id, 1,
            Qt::MatchExactly | Qt::MatchRecursive);

    if (indexes.isEmpty()) {
        return QModelIndex();
    }

    return indexes.first();
}

static QVariantMap configToVariantMap(const DecaptchaPluginConfig *config) {
    QVariantMap map;
    map["displayName"] = config->displayName();
    map["filePath"] = config->filePath();
    map["iconFilePath"] = config->iconFilePath();
    map["id"] = config->id();
    map["pluginFilePath"] = config->pluginFilePath();
    map["pluginType"] = config->pluginType();
    map["version"] = config->version();
    return map;
}

static QVariantMap configToVariantMap(const RecaptchaPluginConfig *config) {
    QVariantMap map;
    map["displayName"] = config->displayName();
    map["filePath"] = config->filePath();
    map["iconFilePath"] = config->iconFilePath();
    map["id"] = config->id();
    map["pluginFilePath"] = config->pluginFilePath();
    map["pluginType"] = config->pluginType();
    map["version"] = config->version();
    return map;
}

static QVariantMap configToVariantMap(const SearchPluginConfig *config) {
    QVariantMap map;
    map["displayName"] = config->displayName();
    map["filePath"] = config->filePath();
    map["iconFilePath"] = config->iconFilePath();
    map["id"] = config->id();
    map["pluginFilePath"] = config->pluginFilePath();
    map["pluginType"] = config->pluginType();
    map["version"] = config->version();
    return map;
}

static QVariantMap configToVariantMap(const ServicePluginConfig *config) {
    QVariantMap map;
    map["displayName"] = config->displayName();
    map["filePath"] = config->filePath();
    map["iconFilePath"] = config->iconFilePath();
    map["id"] = config->id();
    map["pluginFilePath"] = config->pluginFilePath();
    map["pluginType"] = config->pluginType();
    map["regExp"] = config->regExp().pattern();
    map["version"] = config->version();
    return map;
}

static void getConfigSetting(const PluginSettings &plugin, QVariantMap &setting, const QString &group = QString()) {
    QString key = setting.value("key").toString();

    if (key.isEmpty()) {
        return;
    }
    
    if (!group.isEmpty()) {
        key.prepend("/");
        key.prepend(group);
    }

    const QString type = setting.value("type").toString();

    if (type == "group") {
        QVariantList settings = setting.value("settings").toList();
        
        for (int i = 0; i < settings.size(); i++) {
            QVariantMap map = settings.at(i).toMap();
            getConfigSetting(plugin, map, key);
            settings[i] = map;
        }

        setting["settings"] = settings;
    }
    else {
        setting["value"] = plugin.value(key, setting.value("value"));
    }
}       

static QVariantList getConfigSettings(const DecaptchaPluginConfig *config) {
    QVariantList settings = config->settings();

    if (settings.isEmpty()) {
        return settings;
    }

    PluginSettings plugin(config->id());

    for (int i = 0; i < settings.size(); i++) {
        QVariantMap setting = settings.at(i).toMap();
        getConfigSetting(plugin, setting);
        settings[i] = setting;
    }

    return settings;
}

static QVariantList getConfigSettings(const RecaptchaPluginConfig *config) {
    QVariantList settings = config->settings();

    if (settings.isEmpty()) {
        return settings;
    }

    PluginSettings plugin(config->id());

    for (int i = 0; i < settings.size(); i++) {
        QVariantMap setting = settings.at(i).toMap();
        getConfigSetting(plugin, setting);
        settings[i] = setting;
    }

    return settings;
}

static QVariantList getConfigSettings(const SearchPluginConfig *config) {
    QVariantList settings = config->settings();

    if (settings.isEmpty()) {
        return settings;
    }

    PluginSettings plugin(config->id());

    for (int i = 0; i < settings.size(); i++) {
        QVariantMap setting = settings.at(i).toMap();
        getConfigSetting(plugin, setting);
        settings[i] = setting;
    }

    return settings;
}

static QVariantList getConfigSettings(const ServicePluginConfig *config) {
    QVariantList settings = config->settings();

    if (settings.isEmpty()) {
        return settings;
    }

    PluginSettings plugin(config->id());

    for (int i = 0; i < settings.size(); i++) {
        QVariantMap setting = settings.at(i).toMap();
        getConfigSetting(plugin, setting);
        settings[i] = setting;
    }

    return settings;
}

Qdl::Qdl() :
    QObject()
{
    QDBusConnection connection = QDBusConnection::sessionBus();
    connection.registerService("org.marxoft.qdl2");
    connection.registerObject("/org/marxoft/qdl2", this, QDBusConnection::ExportScriptableSlots);
}

Qdl::~Qdl() {
    self = 0;
}

Qdl* Qdl::instance() {
    return self ? self : self = new Qdl;
}

void Qdl::quit() {
    closeWindow();
    ClipboardUrlModel::instance()->save();
    TransferModel::instance()->save();
    Logger::log("Qdl::quit(): Quitting the application");
    QCoreApplication::instance()->quit();
}

void Qdl::showWindow() {
    if (window.isNull()) {
        window = new MainWindow;
    }

    window->show();
}

void Qdl::closeWindow() {
    if (!window.isNull()) {
        window->close();
    }
}

QVariantMap Qdl::addTransfer(const QString &url, const QString &requestMethod, const QVariantMap &requestHeaders,
        const QString &postData, const QString &category, bool createSubfolder, int priority,
        const QString &customCommand, bool overrideGlobalCommand, bool startAutomatically) {
    if (TransferItem *transfer = TransferModel::instance()->append(url, requestMethod, requestHeaders, postData,
                category, createSubfolder, priority, customCommand, overrideGlobalCommand, startAutomatically)) {
        return transfer->itemDataWithRoleNames();
    }

    return QVariantMap();
}

QVariantList Qdl::addTransfers(const QStringList &urls, const QString &requestMethod, const QVariantMap &requestHeaders,
        const QString &postData, const QString &category, bool createSubfolder, int priority,
        const QString &customCommand, bool overrideGlobalCommand, bool startAutomatically) {
    QVariantList transfers;

    foreach (const TransferItem *transfer, TransferModel::instance()->append(urls, requestMethod, requestHeaders,
                postData, category, createSubfolder, priority, customCommand, overrideGlobalCommand,
                startAutomatically)) {
        transfers << transfer->itemDataWithRoleNames();
    }

    return transfers;
}

QVariantList Qdl::getTransfers(int offset, int limit) {
    const int count = TransferModel::instance()->rowCount();

    if ((offset < 0) || (offset >= count - 1)) {
        return QVariantList();
    }

    if (limit < 0) {
        limit = count - offset;
    }
    else {
        limit += offset;
    }

    QVariantList transfers;
    
    for (int i = offset; i < limit; i++) {
        const QModelIndex parent = TransferModel::instance()->index(i, 0, QModelIndex());
        QVariantMap package = TransferModel::instance()->itemDataWithRoleNames(parent);

        if (!package.isEmpty()) {
            const int transferCount = TransferModel::instance()->rowCount(parent);
            QVariantList children;
            
            for (int j = 0; j < transferCount; j++) {
                const QModelIndex child = TransferModel::instance()->index(j, 0, parent);
                const QVariantMap transfer = TransferModel::instance()->itemDataWithRoleNames(child);
                
                if (!transfer.isEmpty()) {
                    children << transfer;
                }
            }
            
            package["children"] = children;
            transfers << package;
        }
    }

    return transfers;
}

QVariantMap Qdl::getTransfersStatus() {
    QVariantMap status;
    status["activeTransfers"] = TransferModel::instance()->activeTransfers();
    status["totalSpeed"] = TransferModel::instance()->totalSpeed();
    status["totalSpeedString"] = TransferModel::instance()->totalSpeedString();
    return status;
}

QVariantMap Qdl::getTransfer(const QString &id) {
    const QModelIndex index = getTransferModelIndexById(id);

    if (index.isValid()) {
        QVariantMap transfer = TransferModel::instance()->itemDataWithRoleNames(index);

        QVariantList children;
        const int count = TransferModel::instance()->rowCount(index);
        
        for (int i = 0; i < count; i++) {
            children << TransferModel::instance()->itemDataWithRoleNames(TransferModel::instance()->index(i, 0, index));
        }

        transfer["children"] = children;
        return transfer;
    }

    return QVariantMap();
}

QVariantList Qdl::searchTransfers(const QString &property, const QVariant &value, int offset, int limit) {
    if ((limit <= 0) || (limit > TransferModel::instance()->rowCount())) {
        limit = TransferModel::instance()->rowCount();
    }

    const int hits = qMin(offset + limit, TransferModel::instance()->rowCount());
    const int role = TransferItem::roleNames().key(property.toUtf8());
    Qt::MatchFlags flags;

    switch (value.type()) {
    case QVariant::String:
        flags = Qt::MatchFixedString | Qt::MatchContains;
        break;
    default:
        flags = Qt::MatchExactly;
        break;
    }

    QVariantList transfers;
    const QModelIndexList indexes =
    TransferModel::instance()->match(TransferModel::instance()->index(0, 0), role, value, hits, flags);
    
    for (int i = offset; i < indexes.size(); i++) {
        const QModelIndex &index = indexes.at(i);
        QVariantMap transfer = TransferModel::instance()->itemDataWithRoleNames(index);
        
        if (!transfer.isEmpty()) {
            QVariantList children;
            const int count = TransferModel::instance()->rowCount(index);
            
            for (int i = 0; i < count; i++) {
                children << TransferModel::instance()->itemDataWithRoleNames(TransferModel::instance()->index(i, 0, index));
            }
            
            transfer["children"] = children;
            transfers << transfer;
        }
    }
    
    return transfers;
}

bool Qdl::setTransferProperty(const QString &id, const QString &property, const QVariant &value) {
    const QModelIndex index = getTransferModelIndexById(id);

    if (index.isValid()) {
        return TransferModel::instance()->setData(index, value, property.toUtf8());
    }

    return false;
}

bool Qdl::setTransferProperties(const QString &id, const QVariantMap &properties) {
    const QModelIndex index = getTransferModelIndexById(id);

    if (index.isValid()) {
        return TransferModel::instance()->setItemData(index, properties);
    }

    return false;
}

bool Qdl::startTransfer(const QString &id) {
    return setTransferProperty(id, "status", TransferItem::Queued);
}

bool Qdl::pauseTransfer(const QString &id) {
    return setTransferProperty(id, "status", TransferItem::Paused);
}

bool Qdl::removeTransfer(const QString &id, bool deleteFiles) {
    return setTransferProperty(id, "status", deleteFiles ? TransferItem::CanceledAndDeleted : TransferItem::Canceled);
}

bool Qdl::moveTransfers(const QString &sourceParentId, int sourceRow, int count, const QString &destinationParentId,
        int destinationRow) {
    const QModelIndex sourceIndex = getTransferModelIndexById(sourceParentId);
    const QModelIndex destinationIndex = getTransferModelIndexById(destinationParentId);
    return TransferModel::instance()->moveRows(sourceIndex, sourceRow, count, destinationIndex, destinationRow);
}

void Qdl::startTransfers() {
    TransferModel::instance()->queue();
}

void Qdl::pauseTransfers() {
    TransferModel::instance()->pause();
}

QVariantMap Qdl::addCategory(const QString &name, const QString &path) {
    if (Categories::add(name, path)) {
        return getCategory(name);
    }

    return QVariantMap();
}

QVariantList Qdl::getCategories() {
    QVariantList list;
    const CategoryList categories = Categories::get();

    for (int i = 0; i < categories.size(); i++) {
        QVariantMap category;
        category["name"] = categories.at(i).name;
        category["path"] = categories.at(i).path;
        list << category;
    }

    return list;
}

QVariantMap Qdl::getCategory(const QString &name) {
    const Category category = Categories::get(name);
    QVariantMap map;
    map["name"] = category.name;
    map["path"] = category.path;
    return map;
}

bool Qdl::removeCategory(const QString &name) {
    return Categories::remove(name);
}

void Qdl::clearClipboardUrls() {
    ClipboardUrlModel::instance()->clear();
}

QStringList Qdl::getClipboardUrls() {
    return ClipboardUrlModel::instance()->strings();
}

bool Qdl::removeClipboardUrl(const QString &url) {
    return ClipboardUrlModel::instance()->remove(url);
}

QVariantList Qdl::getDecaptchaPlugins() {
    QVariantList list;

    foreach (const DecaptchaPluginPair &pair, DecaptchaPluginManager::instance()->plugins()) {
        list << configToVariantMap(pair.config);
    }

    return list;
}

QVariantMap Qdl::getDecaptchaPlugin(const QString &id) {
    if (const DecaptchaPluginConfig *config = DecaptchaPluginManager::instance()->getConfigById(id)) {
        return configToVariantMap(config);
    }

    return QVariantMap();
}

QVariantList Qdl::getDecaptchaPluginSettings(const QString &id) {
    if (const DecaptchaPluginConfig *config = DecaptchaPluginManager::instance()->getConfigById(id)) {
        return getConfigSettings(config);
    }

    return QVariantList();
}

bool Qdl::setDecaptchaPluginSettings(const QString &id, const QVariantMap &properties) {
    if (properties.isEmpty()) {
        return false;
    }
    
    if (const DecaptchaPluginConfig *config = DecaptchaPluginManager::instance()->getConfigById(id)) {
        PluginSettings settings(config->id());
        QMapIterator<QString, QVariant> iterator(properties);

        while (iterator.hasNext()) {
            iterator.next();
            settings.setValue(iterator.key(), iterator.value());
        }

        return true;
    }

    return false;
}

QVariantList Qdl::getRecaptchaPlugins() {
    QVariantList list;

    foreach (const RecaptchaPluginPair &pair, RecaptchaPluginManager::instance()->plugins()) {
        list << configToVariantMap(pair.config);
    }

    return list;
}

QVariantMap Qdl::getRecaptchaPlugin(const QString &id) {
    if (const RecaptchaPluginConfig *config = RecaptchaPluginManager::instance()->getConfigById(id)) {
        return configToVariantMap(config);
    }

    return QVariantMap();
}

QVariantList Qdl::getRecaptchaPluginSettings(const QString &id) {
    if (const RecaptchaPluginConfig *config = RecaptchaPluginManager::instance()->getConfigById(id)) {
        return getConfigSettings(config);
    }

    return QVariantList();
}

bool Qdl::setRecaptchaPluginSettings(const QString &id, const QVariantMap &properties) {
    if (properties.isEmpty()) {
        return false;
    }
    
    if (const RecaptchaPluginConfig *config = RecaptchaPluginManager::instance()->getConfigById(id)) {
        PluginSettings settings(config->id());
        QMapIterator<QString, QVariant> iterator(properties);

        while (iterator.hasNext()) {
            iterator.next();
            settings.setValue(iterator.key(), iterator.value());
        }

        return true;
    }

    return false;
}

QVariantList Qdl::getSearchPlugins() {
    QVariantList list;

    foreach (const SearchPluginPair &pair, SearchPluginManager::instance()->plugins()) {
        list << configToVariantMap(pair.config);
    }

    return list;
}

QVariantMap Qdl::getSearchPlugin(const QString &id) {
    if (const SearchPluginConfig *config = SearchPluginManager::instance()->getConfigById(id)) {
        return configToVariantMap(config);
    }

    return QVariantMap();
}

QVariantList Qdl::getSearchPluginSettings(const QString &id) {
    if (const SearchPluginConfig *config = SearchPluginManager::instance()->getConfigById(id)) {
        return getConfigSettings(config);
    }

    return QVariantList();
}

bool Qdl::setSearchPluginSettings(const QString &id, const QVariantMap &properties) {
    if (properties.isEmpty()) {
        return false;
    }
    
    if (const SearchPluginConfig *config = SearchPluginManager::instance()->getConfigById(id)) {
        PluginSettings settings(config->id());
        QMapIterator<QString, QVariant> iterator(properties);

        while (iterator.hasNext()) {
            iterator.next();
            settings.setValue(iterator.key(), iterator.value());
        }

        return true;
    }

    return false;
}

QVariantList Qdl::getServicePlugins() {
    QVariantList list;

    foreach (const ServicePluginPair &pair, ServicePluginManager::instance()->plugins()) {
        list << configToVariantMap(pair.config);
    }

    return list;
}

QVariantMap Qdl::getServicePlugin(const QString &id) {
    if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigById(id)) {
        return configToVariantMap(config);
    }

    return QVariantMap();
}

QVariantList Qdl::getServicePluginSettings(const QString &id) {
    if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigById(id)) {
        return getConfigSettings(config);
    }

    return QVariantList();
}

bool Qdl::setServicePluginSettings(const QString &id, const QVariantMap &properties) {
    if (properties.isEmpty()) {
        return false;
    }
    
    if (const ServicePluginConfig *config = ServicePluginManager::instance()->getConfigById(id)) {
        PluginSettings settings(config->id());
        QMapIterator<QString, QVariant> iterator(properties);

        while (iterator.hasNext()) {
            iterator.next();
            settings.setValue(iterator.key(), iterator.value());
        }

        return true;
    }

    return false;
}

QVariantMap Qdl::getSettings() {
    QVariantMap map;
    map["clipboardMonitorEnabled"] = Settings::clipboardMonitorEnabled();
    map["customCommand"] = Settings::customCommand();
    map["customCommandEnabled"] = Settings::customCommandEnabled();
    map["decaptchaPlugin"] = Settings::decaptchaPlugin();
    map["defaultCategory"] = Settings::defaultCategory();
    map["defaultSearchPlugin"] = Settings::defaultSearchPlugin();
    map["defaultServicePlugin"] = Settings::defaultServicePlugin();
    map["usePlugins"] = Settings::usePlugins();
    map["downloadPath"] = Settings::downloadPath();
    map["createSubfolders"] = Settings::createSubfolders();
    map["extractArchives"] = Settings::extractArchives();
    map["deleteExtractedArchives"] = Settings::deleteExtractedArchives();
    map["archivePasswords"] = Settings::archivePasswords();
    map["maximumConcurrentTransfers"] = Settings::maximumConcurrentTransfers();
    map["startTransfersAutomatically"] = Settings::startTransfersAutomatically();
    map["nextAction"] = Settings::nextAction();
    map["networkProxyEnabled"] = Settings::networkProxyEnabled();
    map["networkProxyType"] = Settings::networkProxyType();
    map["networkProxyHost"] = Settings::networkProxyHost();
    map["networkProxyPort"] = Settings::networkProxyPort();
    map["networkProxyAuthenticationEnabled"] = Settings::networkProxyAuthenticationEnabled();
    map["networkProxyUsername"] = Settings::networkProxyUsername();
    map["networkProxyPassword"] = Settings::networkProxyPassword();
    return map;
}

bool Qdl::setSettings(const QVariantMap &settings) {
    QMapIterator<QString, QVariant> iterator(settings);
    bool setNetworkProxy = false;

    while (iterator.hasNext()) {
        iterator.next();

        if (iterator.key() == "clipboardMonitorEnabled") {
            Settings::setClipboardMonitorEnabled(iterator.value().toBool());
        }
        else if (iterator.key() == "customCommand") {
            Settings::setCustomCommand(iterator.value().toString());
        }
        else if (iterator.key() == "customCommandEnabled") {
            Settings::setCustomCommandEnabled(iterator.value().toBool());
        }
        else if (iterator.key() == "decaptchaPlugin") {
            Settings::setDecaptchaPlugin(iterator.value().toString());
        }
        else if (iterator.key() == "defaultCategory") {
            Settings::setDefaultCategory(iterator.value().toString());
        }
        else if (iterator.key() == "defaultSearchPlugin") {
            Settings::setDefaultSearchPlugin(iterator.value().toString());
        }
        else if (iterator.key() == "defaultServicePlugin") {
            Settings::setDefaultServicePlugin(iterator.value().toString());
        }
        else if (iterator.key() == "usePlugins") {
            Settings::setUsePlugins(iterator.value().toBool());
        }
        else if (iterator.key() == "downloadPath") {
            Settings::setDownloadPath(iterator.value().toString());
        }
        else if (iterator.key() == "createSubfolders") {
            Settings::setCreateSubfolders(iterator.value().toBool());
        }
        else if (iterator.key() == "extractArchives") {
            Settings::setExtractArchives(iterator.value().toBool());
        }
        else if (iterator.key() == "deleteExtractedArchives") {
            Settings::setDeleteExtractedArchives(iterator.value().toBool());
        }
        else if (iterator.key() == "archivePasswords") {
            Settings::setArchivePasswords(iterator.value().toStringList());
        }
        else if (iterator.key() == "maximumConcurrentTransfers") {
            Settings::setMaximumConcurrentTransfers(iterator.value().toInt());
        }
        else if (iterator.key() == "startTransfersAutomatically") {
            Settings::setStartTransfersAutomatically(iterator.value().toBool());
        }
        else if (iterator.key() == "nextAction") {
            Settings::setNextAction(iterator.value().toInt());
        }
        else if (iterator.key() == "networkProxyEnabled") {
            Settings::setNetworkProxyEnabled(iterator.value().toBool());
        }
        else if (iterator.key() == "networkProxyType") {
            Settings::setNetworkProxyType(iterator.value().toInt());
            setNetworkProxy = true;
        }
        else if (iterator.key() == "networkProxyHost") {
            Settings::setNetworkProxyHost(iterator.value().toString());
            setNetworkProxy = true;
        }
        else if (iterator.key() == "networkProxyPort") {
            Settings::setNetworkProxyPort(iterator.value().toInt());
            setNetworkProxy = true;
        }
        else if (iterator.key() == "networkProxyAuthenticationEnabled") {
            Settings::setNetworkProxyAuthenticationEnabled(iterator.value().toBool());
            setNetworkProxy = true;
        }
        else if (iterator.key() == "networkProxyUsername") {
            Settings::setNetworkProxyUsername(iterator.value().toString());
            setNetworkProxy = true;
        }
        else if (iterator.key() == "networkProxyPassword") {
            Settings::setNetworkProxyPassword(iterator.value().toString());
            setNetworkProxy = true;
        }
        else {
            return false;
        }
    }

    if (setNetworkProxy) {
        Settings::setNetworkProxy();
    }

    return true;
}

QVariantList Qdl::addUrlChecks(const QStringList &urls, const QString &category, bool createSubfolder, int priority,
        const QString &customCommand, bool overrideGlobalCommand, bool startAutomatically) {
    UrlCheckModel::instance()->append(urls, category, createSubfolder, priority, customCommand, overrideGlobalCommand,
            startAutomatically);
    return getUrlChecks();
}

void Qdl::clearUrlChecks() {
    UrlCheckModel::instance()->clear();
}

QVariantList Qdl::getUrlChecks() {
    QVariantList checks;

    for (int i = 0; i < UrlCheckModel::instance()->rowCount(); i++) {
        checks << UrlCheckModel::instance()->itemData(i);
    }

    return checks;
}

QVariantMap Qdl::getUrlChecksStatus() {
    QVariantMap map;
    map["captchaType"] = UrlCheckModel::instance()->captchaType();
    map["captchaTypeString"] = UrlCheckModel::instance()->captchaTypeString();
    map["captchaData"] = UrlCheckModel::instance()->captchaData();
    map["captchaTimeout"] = UrlCheckModel::instance()->captchaTimeout();
    map["captchaTimeoutString"] = UrlCheckModel::instance()->captchaTimeoutString();
    map["count"] = UrlCheckModel::instance()->rowCount();
    map["progress"] = UrlCheckModel::instance()->progress();
    map["requestedSettings"] = UrlCheckModel::instance()->requestedSettings();
    map["requestedSettingsTimeout"] = UrlCheckModel::instance()->requestedSettingsTimeout();
    map["requestedSettingsTimeoutString"] = UrlCheckModel::instance()->requestedSettingsTimeoutString();
    map["requestedSettingsTitle"] = UrlCheckModel::instance()->requestedSettingsTitle();
    map["status"] = UrlCheckModel::instance()->status();
    map["statusString"] = UrlCheckModel::instance()->statusString();
    map["waitTime"] = UrlCheckModel::instance()->waitTime();
    map["waitTimeString"] = UrlCheckModel::instance()->waitTimeString();
    return map;
}

bool Qdl::submitUrlCheckCaptchaResponse(const QString &response) {
    return UrlCheckModel::instance()->submitCaptchaResponse(response);
}

bool Qdl::submitUrlCheckSettingsResponse(const QVariantMap &settings) {
    return UrlCheckModel::instance()->submitSettingsResponse(settings);
}

QVariantList Qdl::addUrlRetrievals(const QStringList &urls, const QString &pluginId) {
    UrlRetrievalModel::instance()->append(urls, pluginId);
    return getUrlRetrievals();
}

void Qdl::clearUrlRetrievals() {
    UrlRetrievalModel::instance()->clear();
}

QVariantList Qdl::getUrlRetrievals() {
    QVariantList retrievals;

    for (int i = 0; i < UrlRetrievalModel::instance()->rowCount(); i++) {
        retrievals << UrlRetrievalModel::instance()->itemData(i);
    }

    return retrievals;
}

QVariantMap Qdl::getUrlRetrievalsStatus() {
    QVariantMap map;
    map["count"] = UrlRetrievalModel::instance()->rowCount();
    map["progress"] = UrlRetrievalModel::instance()->progress();
    map["status"] = UrlRetrievalModel::instance()->status();
    map["statusString"] = UrlRetrievalModel::instance()->statusString();
    return map;
}

QVariantList Qdl::addDownloadRequests(const QStringList &urls) {
    DownloadRequestModel::instance()->append(urls);
    return getDownloadRequests();
}

void Qdl::clearDownloadRequests() {
    DownloadRequestModel::instance()->clear();
}

QVariantList Qdl::getDownloadRequests() {
    QVariantList requests;

    for (int i = 0; i < DownloadRequestModel::instance()->rowCount(); i++) {
        requests << DownloadRequestModel::instance()->itemData(i);
    }

    return requests;
}

QVariantMap Qdl::getDownloadRequestsStatus() {
    QVariantMap map;
    map["captchaType"] = DownloadRequestModel::instance()->captchaType();
    map["captchaTypeString"] = DownloadRequestModel::instance()->captchaTypeString();
    map["captchaData"] = DownloadRequestModel::instance()->captchaData();
    map["captchaTimeout"] = DownloadRequestModel::instance()->captchaTimeout();
    map["captchaTimeoutString"] = DownloadRequestModel::instance()->captchaTimeoutString();
    map["count"] = DownloadRequestModel::instance()->rowCount();
    map["progress"] = DownloadRequestModel::instance()->progress();
    map["requestedSettings"] = DownloadRequestModel::instance()->requestedSettings();
    map["requestedSettingsTimeout"] = DownloadRequestModel::instance()->requestedSettingsTimeout();
    map["requestedSettingsTimeoutString"] = DownloadRequestModel::instance()->requestedSettingsTimeoutString();
    map["requestedSettingsTitle"] = DownloadRequestModel::instance()->requestedSettingsTitle();
    map["status"] = DownloadRequestModel::instance()->status();
    map["statusString"] = DownloadRequestModel::instance()->statusString();
    map["waitTime"] = DownloadRequestModel::instance()->waitTime();
    map["waitTimeString"] = DownloadRequestModel::instance()->waitTimeString();
    return map;
}

bool Qdl::submitDownloadRequestCaptchaResponse(const QString &response) {
    return DownloadRequestModel::instance()->submitCaptchaResponse(response);
}

bool Qdl::submitDownloadRequestSettingsResponse(const QVariantMap &settings) {
    return DownloadRequestModel::instance()->submitSettingsResponse(settings);
}
