/*
 * Copyright (C) 2016 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms and conditions of the GNU General Public License,
 * version 3, as published by the Free Software Foundation.
 *
 * This program is distributed in the hope it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St - Fifth Floor, Boston, MA 02110-1301 USA.
 */

#include "servicepluginmanager.h"
#include "definitions.h"
#include "javascriptserviceplugin.h"
#include "logger.h"
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QPluginLoader>

static bool displayNameLessThan(const ServicePluginPair &pair, const ServicePluginPair &other) {
    return QString::localeAwareCompare(pair.config->displayName(), other.config->displayName()) < 0;
}

ServicePluginManager* ServicePluginManager::self = 0;

ServicePluginManager::ServicePluginManager() :
    QObject(),
    m_nam(0)
{
}

ServicePluginManager::~ServicePluginManager() {
    self = 0;
}

ServicePluginManager* ServicePluginManager::instance() {
    return self ? self : self = new ServicePluginManager;
}

ServicePluginList ServicePluginManager::plugins() const {
    return m_plugins;
}

ServicePluginConfig* ServicePluginManager::getConfigById(const QString &id) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("ServicePluginManager::getConfigById(). PluginFound: " + id);
            return pair.config;
        }
    }
    
    Logger::log("ServicePluginManager::getConfigById(). No Plugin found");
    return 0;
}

ServicePluginConfig* ServicePluginManager::getConfigByFilePath(const QString &filePath) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->filePath() == filePath) {
            Logger::log("ServicePluginManager::getConfigByFilePath(). PluginFound: " + pair.config->id());
            return pair.config;
        }
    }
    
    Logger::log("ServicePluginManager::getConfigByFilePath(). No Plugin found");
    return 0;
}

ServicePluginConfig* ServicePluginManager::getConfigByUrl(const QString &url) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->urlIsSupported(url)) {
            Logger::log("ServicePluginManager::getConfigByUrl(). PluginFound: " + pair.config->id());
            return pair.config;
        }
    }
    
    Logger::log("ServicePluginManager::getConfigByUrl(). No Plugin found");
    return 0;
}

ServicePlugin* ServicePluginManager::getPluginById(const QString &id) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("ServicePluginManager::getPluginById(). PluginFound: " + id);
            return pair.plugin;
        }
    }
    
    Logger::log("ServicePluginManager::getPluginById(). No Plugin found");
    return 0;
}

ServicePlugin* ServicePluginManager::getPluginByUrl(const QString &url) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->urlIsSupported(url)) {
            Logger::log("ServicePluginManager::getPluginByUrl(). PluginFound: " + pair.config->id());
            return pair.plugin;
        }
    }
    
    Logger::log("ServicePluginManager::getPluginByUrl(). No Plugin found");
    return 0;
}

ServicePlugin* ServicePluginManager::createPluginById(const QString &id, QObject *parent) const {
    if (ServicePlugin *plugin = getPluginById(id)) {
        return plugin->createPlugin(parent);
    }

    return 0;
}

ServicePlugin* ServicePluginManager::createPluginByUrl(const QString &url, QObject *parent) const {
    if (ServicePlugin *plugin = getPluginByUrl(url)) {
        return plugin->createPlugin(parent);
    }

    return 0;
}

bool ServicePluginManager::urlIsSupported(const QString &url) const {
    if (getConfigByUrl(url)) {
        return true;
    }

    return false;
}

QNetworkAccessManager* ServicePluginManager::networkAccessManager() {
    if (!m_nam) {
        m_nam = new QNetworkAccessManager(this);
    }

    return m_nam;
}

int ServicePluginManager::load() {
    Logger::log("ServicePluginManager::load(): Loading plugins modified since "
                + m_lastLoaded.toString(Qt::ISODate));
    int count = 0;
    QDir dir(SERVICE_PLUGIN_PATH);
    
    foreach (const QFileInfo &info, dir.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Time)) {
        if (info.lastModified() > m_lastLoaded) {
            ServicePluginConfig *config = getConfigByFilePath(info.absoluteFilePath());

            if (!config) {
                config = new ServicePluginConfig(this);
                
                if (config->load(info.absoluteFilePath())) {
                    if (config->pluginType() == "js") {
                        JavaScriptServicePlugin *js =
                        new JavaScriptServicePlugin(config->id(), config->pluginFilePath(), this);
                        m_plugins << ServicePluginPair(config, js);
                        ++count;
                        Logger::log("ServicePluginManager::load(). JavaScript plugin loaded: " + config->id());
                    }
                    else {
                        QPluginLoader loader(config->pluginFilePath());
                        QObject *obj = loader.instance();
                        
                        if (obj) {
                            if (ServicePlugin *plugin = qobject_cast<ServicePlugin*>(obj)) {
                                plugin->setNetworkAccessManager(networkAccessManager());
                                m_plugins << ServicePluginPair(config, plugin);
                                ++count;
                                Logger::log("ServicePluginManager::load(). Qt Plugin loaded: " + config->id());
                            }
                            else {
                                loader.unload();
                                Logger::log("ServicePluginManager::load(). Error loading Qt plugin: "
                                            + config->id());
                            }
                        }
                        else {
                            Logger::log("ServicePluginManager::load(). Qt plugin is NULL: " + config->id());
                        }
                    }
                }
                else {
                    delete config;
                }
            }
        }
        else {
            break;
        }
    }

    Logger::log(QString("ServicePluginManager::load() %1 new plugins loaded").arg(count));

    if (count > 0) {
        qSort(m_plugins.begin(), m_plugins.end(), displayNameLessThan);
    }

    m_lastLoaded = QDateTime::currentDateTime();
    return count;
}
