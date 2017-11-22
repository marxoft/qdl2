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
#include "javascriptpluginengine.h"
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
    m_nam(0),
    m_lastLoaded(QDateTime::fromTime_t(0))
{
}

ServicePluginManager::~ServicePluginManager() {
    self = 0;
}

ServicePluginManager* ServicePluginManager::instance() {
    return self ? self : self = new ServicePluginManager;
}

int ServicePluginManager::count() const {
    return m_plugins.size();
}

ServicePluginList ServicePluginManager::plugins() const {
    return m_plugins;
}

ServicePluginConfig* ServicePluginManager::getConfigById(const QString &id) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("ServicePluginManager::getConfigById(). Config found: " + id, Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("ServicePluginManager::getConfigById(). No config found for id " + id, Logger::HighVerbosity);
    return 0;
}

ServicePluginConfig* ServicePluginManager::getConfigByFilePath(const QString &filePath) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->filePath() == filePath) {
            Logger::log("ServicePluginManager::getConfigByFilePath(). Config found: " + pair.config->id(),
                        Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("ServicePluginManager::getConfigByFilePath(). No config found for filePath " + filePath,
                Logger::HighVerbosity);
    return 0;
}

ServicePluginConfig* ServicePluginManager::getConfigByUrl(const QString &url) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->urlIsSupported(url)) {
            Logger::log("ServicePluginManager::getConfigByUrl(). Config found: " + pair.config->id(),
                        Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("ServicePluginManager::getConfigByUrl(). No config found for URL " + url, Logger::HighVerbosity);
    return 0;
}

ServicePluginFactory* ServicePluginManager::getFactoryById(const QString &id) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("ServicePluginManager::getFactoryById(). Factory found: " + id, Logger::HighVerbosity);
            return pair.factory;
        }
    }
    
    Logger::log("ServicePluginManager::getFactoryById(). No factory found for id " + id, Logger::HighVerbosity);
    return 0;
}

ServicePluginFactory* ServicePluginManager::getFactoryByUrl(const QString &url) const {
    foreach (const ServicePluginPair &pair, m_plugins) {
        if (pair.config->urlIsSupported(url)) {
            Logger::log("ServicePluginManager::getFactoryByUrl(). Factory found: " + pair.config->id(),
                        Logger::HighVerbosity);
            return pair.factory;
        }
    }
    
    Logger::log("ServicePluginManager::getFactoryByUrl(). No factory found for URL " + url, Logger::HighVerbosity);
    return 0;
}

ServicePlugin* ServicePluginManager::createPluginById(const QString &id, QObject *parent) {
    if (ServicePluginFactory *factory = getFactoryById(id)) {
        if (ServicePlugin *plugin = factory->createPlugin(parent)) {
            plugin->setNetworkAccessManager(networkAccessManager());
            return plugin;
        }
    }

    return 0;
}

ServicePlugin* ServicePluginManager::createPluginByUrl(const QString &url, QObject *parent) {
    if (ServicePluginFactory *factory = getFactoryByUrl(url)) {
        if (ServicePlugin *plugin = factory->createPlugin(parent)) {
            plugin->setNetworkAccessManager(networkAccessManager());
            return plugin;
        }
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
    return m_nam ? m_nam :  m_nam = new QNetworkAccessManager(this);
}

int ServicePluginManager::load() {
    Logger::log("ServicePluginManager::load(): Loading plugins modified since "
                + m_lastLoaded.toString(Qt::ISODate), Logger::LowVerbosity);
    int count = 0;
    QDir dir;
    
    foreach (const QString &path, SERVICE_PLUGIN_PATHS) {
        dir.setPath(path);
        
        foreach (const QFileInfo &info, dir.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Time)) {
            if (info.created() > m_lastLoaded) {
                ServicePluginConfig *config = getConfigByFilePath(info.absoluteFilePath());
                
                if (!config) {
                    config = new ServicePluginConfig(this);
                    
                    if (config->load(info.absoluteFilePath())) {
                        if (config->pluginType() == "js") {
                            JavaScriptServicePluginFactory *js =
                                new JavaScriptServicePluginFactory(config->pluginFilePath(),
                                        JavaScriptPluginEngine::instance(), this);
                            m_plugins << ServicePluginPair(config, js);
                            ++count;
                            Logger::log("ServicePluginManager::load(). JavaScript plugin loaded: " + config->id(),
                                        Logger::MediumVerbosity);
                        }
                        else {
                            QPluginLoader loader(config->pluginFilePath());
                            QObject *obj = loader.instance();
                            
                            if (obj) {
                                if (ServicePluginFactory *factory = qobject_cast<ServicePluginFactory*>(obj)) {
                                    m_plugins << ServicePluginPair(config, factory);
                                    ++count;
                                    Logger::log("ServicePluginManager::load(). Qt Plugin loaded: " + config->id(),
                                                Logger::MediumVerbosity);
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
    }

    Logger::log(QString("ServicePluginManager::load() %1 plugins loaded").arg(count), Logger::LowVerbosity);

    if (count > 0) {
        qSort(m_plugins.begin(), m_plugins.end(), displayNameLessThan);
        emit countChanged(m_plugins.size());
    }

    m_lastLoaded = QDateTime::currentDateTime();
    return count;
}
