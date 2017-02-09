/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "searchpluginmanager.h"
#include "definitions.h"
#include "javascriptsearchplugin.h"
#include "logger.h"
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QPluginLoader>

static bool displayNameLessThan(const SearchPluginPair &pair, const SearchPluginPair &other) {
    return QString::localeAwareCompare(pair.config->displayName(), other.config->displayName()) < 0;
}

SearchPluginManager* SearchPluginManager::self = 0;

SearchPluginManager::SearchPluginManager() :
    QObject(),
    m_nam(0),
    m_lastLoaded(QDateTime::fromTime_t(0))
{
}

SearchPluginManager::~SearchPluginManager() {
    self = 0;
}

SearchPluginManager* SearchPluginManager::instance() {
    return self ? self : self = new SearchPluginManager;
}

SearchPluginList SearchPluginManager::plugins() const {
    return m_plugins;
}

SearchPluginConfig* SearchPluginManager::getConfigById(const QString &id) const {
    foreach (const SearchPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("SearchPluginManager::getConfigById(). PluginFound: " + id, Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("SearchPluginManager::getConfigById(). No Plugin found for id " + id, Logger::HighVerbosity);
    return 0;
}

SearchPluginConfig* SearchPluginManager::getConfigByFilePath(const QString &filePath) const {
    foreach (const SearchPluginPair &pair, m_plugins) {
        if (pair.config->filePath() == filePath) {
            Logger::log("SearchPluginManager::getConfigByFilePath(). PluginFound: " + pair.config->id(),
                        Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("SearchPluginManager::getConfigByFilePath(). No Plugin found for filePath " + filePath,
                Logger::HighVerbosity);
    return 0;
}

SearchPlugin* SearchPluginManager::getPluginById(const QString &id) const {
    foreach (const SearchPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("SearchPluginManager::getPluginById(). PluginFound: " + id, Logger::HighVerbosity);
            return pair.plugin;
        }
    }
    
    Logger::log("SearchPluginManager::getPluginById(). No Plugin found for id ", Logger::HighVerbosity);
    return 0;
}

SearchPlugin* SearchPluginManager::createPluginById(const QString &id, QObject *parent) const {
    if (SearchPlugin *plugin = getPluginById(id)) {
        return plugin->createPlugin(parent);
    }

    return 0;
}

QNetworkAccessManager* SearchPluginManager::networkAccessManager() {
    return m_nam ? m_nam : m_nam = new QNetworkAccessManager(this);
}

int SearchPluginManager::load() {
    Logger::log("SearchPluginManager::load(): Loading plugins modified since "
                + m_lastLoaded.toString(Qt::ISODate), Logger::LowVerbosity);
    int count = 0;
    QDir dir;
    
    foreach (const QString &path, SEARCH_PLUGIN_PATHS) {
        dir.setPath(path);
        
        foreach (const QFileInfo &info, dir.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Time)) {
            if (info.lastModified() > m_lastLoaded) {
                SearchPluginConfig *config = getConfigByFilePath(info.absoluteFilePath());
                
                if (!config) {
                    config = new SearchPluginConfig(this);
                    
                    if (config->load(info.absoluteFilePath())) {
                        if (config->pluginType() == "js") {
                            JavaScriptSearchPlugin *js =
                            new JavaScriptSearchPlugin(config->id(), config->pluginFilePath(), this);
                            js->setNetworkAccessManager(networkAccessManager());
                            m_plugins << SearchPluginPair(config, js);
                            ++count;
                            Logger::log("SearchPluginManager::load(). JavaScript plugin loaded: "
                                        + config->id(), Logger::MediumVerbosity);
                        }
                        else {
                            QPluginLoader loader(config->pluginFilePath());
                            QObject *obj = loader.instance();
                            
                            if (obj) {
                                if (SearchPlugin *plugin = qobject_cast<SearchPlugin*>(obj)) {
                                    plugin->setNetworkAccessManager(networkAccessManager());
                                    m_plugins << SearchPluginPair(config, plugin);
                                    ++count;
                                    Logger::log("SearchPluginManager::load(). Qt Plugin loaded: "
                                                + config->id(), Logger::MediumVerbosity);
                                }
                                else {
                                    loader.unload();
                                    Logger::log("SearchPluginManager::load(). Error loading Qt plugin: "
                                                + config->id());
                                }
                            }
                            else {
                                Logger::log("SearchPluginManager::load(). Qt plugin is NULL: " + config->id());
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

    Logger::log(QString("SearchPluginManager::load() %1 plugins loaded").arg(count), Logger::LowVerbosity);

    if (count > 0) {
        qSort(m_plugins.begin(), m_plugins.end(), displayNameLessThan);
    }

    m_lastLoaded = QDateTime::currentDateTime();
    return count;
}
