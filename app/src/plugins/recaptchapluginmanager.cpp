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

#include "recaptchapluginmanager.h"
#include "definitions.h"
#include "javascriptpluginengine.h"
#include "javascriptrecaptchaplugin.h"
#include "logger.h"
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QPluginLoader>

static bool displayNameLessThan(const RecaptchaPluginPair &pair, const RecaptchaPluginPair &other) {
    return QString::localeAwareCompare(pair.config->displayName(), other.config->displayName()) < 0;
}

RecaptchaPluginManager* RecaptchaPluginManager::self = 0;

RecaptchaPluginManager::RecaptchaPluginManager() :
    QObject(),
    m_nam(0),
    m_lastLoaded(QDateTime::fromTime_t(0))
{
}

RecaptchaPluginManager::~RecaptchaPluginManager() {
    self = 0;
}

RecaptchaPluginManager* RecaptchaPluginManager::instance() {
    return self ? self : self = new RecaptchaPluginManager;
}

int RecaptchaPluginManager::count() const {
    return m_plugins.size();
}

RecaptchaPluginList RecaptchaPluginManager::plugins() const {
    return m_plugins;
}

RecaptchaPluginConfig* RecaptchaPluginManager::getConfigById(const QString &id) const {
    foreach (const RecaptchaPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("RecaptchaPluginManager::getConfigById(). Config found: " + id, Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("RecaptchaPluginManager::getConfigById(). No config found for id " + id, Logger::HighVerbosity);
    return 0;
}

RecaptchaPluginConfig* RecaptchaPluginManager::getConfigByFilePath(const QString &filePath) const {
    foreach (const RecaptchaPluginPair &pair, m_plugins) {
        if (pair.config->filePath() == filePath) {
            Logger::log("RecaptchaPluginManager::getConfigByFilePath(). Config found: " + pair.config->id(),
                        Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("RecaptchaPluginManager::getConfigByFilePath(). No config found for filePath " + filePath,
                Logger::HighVerbosity);
    return 0;
}

RecaptchaPluginFactory* RecaptchaPluginManager::getFactoryById(const QString &id) const {
    foreach (const RecaptchaPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("RecaptchaPluginManager::getFactoryById(). Factory found: " + id, Logger::HighVerbosity);
            return pair.factory;
        }
    }
    
    Logger::log("RecaptchaPluginManager::getFactoryById(). No factory found for id ", Logger::HighVerbosity);
    return 0;
}

RecaptchaPlugin* RecaptchaPluginManager::createPluginById(const QString &id, QObject *parent) {
    if (RecaptchaPluginFactory *factory = getFactoryById(id)) {
        if (RecaptchaPlugin *plugin = factory->createPlugin(parent)) {
            plugin->setNetworkAccessManager(networkAccessManager());
            return plugin;
        }
    }

    return 0;
}

QNetworkAccessManager* RecaptchaPluginManager::networkAccessManager() {
    return m_nam ? m_nam : m_nam = new QNetworkAccessManager(this);
}

int RecaptchaPluginManager::load() {
    Logger::log("RecaptchaPluginManager::load(): Loading plugins modified since "
                + m_lastLoaded.toString(Qt::ISODate), Logger::LowVerbosity);
    int count = 0;
    QDir dir;
    
    foreach (const QString &path, RECAPTCHA_PLUGIN_PATHS) {
        dir.setPath(path);
        
        foreach (const QFileInfo &info, dir.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Time)) {
            if (info.created() > m_lastLoaded) {
                RecaptchaPluginConfig *config = getConfigByFilePath(info.absoluteFilePath());
                
                if (!config) {
                    config = new RecaptchaPluginConfig(this);
                    
                    if (config->load(info.absoluteFilePath())) {
                        if (config->pluginType() == "js") {
                            JavaScriptRecaptchaPluginFactory *js =
                                new JavaScriptRecaptchaPluginFactory(config->pluginFilePath(),
                                        JavaScriptPluginEngine::instance(), this);
                            m_plugins << RecaptchaPluginPair(config, js);
                            ++count;
                            Logger::log("RecaptchaPluginManager::load(). JavaScript plugin loaded: "
                                        + config->id(), Logger::MediumVerbosity);
                        }
                        else {
                            QPluginLoader loader(config->pluginFilePath());
                            QObject *obj = loader.instance();
                            
                            if (obj) {
                                if (RecaptchaPluginFactory *factory = qobject_cast<RecaptchaPluginFactory*>(obj)) {
                                    m_plugins << RecaptchaPluginPair(config, factory);
                                    ++count;
                                    Logger::log("RecaptchaPluginManager::load(). Qt Plugin loaded: "
                                                + config->id(), Logger::MediumVerbosity);
                                }
                                else {
                                    loader.unload();
                                    Logger::log("RecaptchaPluginManager::load(). Error loading Qt plugin: "
                                                + config->id());
                                }
                            }
                            else {
                                Logger::log("RecaptchaPluginManager::load(). Qt plugin is NULL: " + config->id());
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

    Logger::log(QString("RecaptchaPluginManager::load() %1 plugins loaded").arg(count), Logger::LowVerbosity);

    if (count > 0) {
        qSort(m_plugins.begin(), m_plugins.end(), displayNameLessThan);
        emit countChanged(m_plugins.size());
    }

    m_lastLoaded = QDateTime::currentDateTime();
    return count;
}
