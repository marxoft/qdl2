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

#include "decaptchapluginmanager.h"
#include "definitions.h"
#include "javascriptdecaptchaplugin.h"
#include "javascriptpluginengine.h"
#include "logger.h"
#include <QDir>
#include <QFileInfo>
#include <QNetworkAccessManager>
#include <QPluginLoader>

static bool displayNameLessThan(const DecaptchaPluginPair &pair, const DecaptchaPluginPair &other) {
    return QString::localeAwareCompare(pair.config->displayName(), other.config->displayName()) < 0;
}

DecaptchaPluginManager* DecaptchaPluginManager::self = 0;

DecaptchaPluginManager::DecaptchaPluginManager() :
    QObject(),
    m_nam(0),
    m_lastLoaded(QDateTime::fromTime_t(0))
{
}

DecaptchaPluginManager::~DecaptchaPluginManager() {
    self = 0;
}

DecaptchaPluginManager* DecaptchaPluginManager::instance() {
    return self ? self : self = new DecaptchaPluginManager;
}

int DecaptchaPluginManager::count() const {
    return m_plugins.size();
}

DecaptchaPluginList DecaptchaPluginManager::plugins() const {
    return m_plugins;
}

DecaptchaPluginConfig* DecaptchaPluginManager::getConfigById(const QString &id) const {
    foreach (const DecaptchaPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("DecaptchaPluginManager::getConfigById(). Config found: " + id, Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("DecaptchaPluginManager::getConfigById(). No config found for id " + id, Logger::HighVerbosity);
    return 0;
}

DecaptchaPluginConfig* DecaptchaPluginManager::getConfigByFilePath(const QString &filePath) const {
    foreach (const DecaptchaPluginPair &pair, m_plugins) {
        if (pair.config->filePath() == filePath) {
            Logger::log("DecaptchaPluginManager::getConfigByFilePath(). Config found: " + pair.config->id(),
                        Logger::HighVerbosity);
            return pair.config;
        }
    }
    
    Logger::log("DecaptchaPluginManager::getConfigByFilePath(). No config found for filePath " + filePath,
                Logger::HighVerbosity);
    return 0;
}

DecaptchaPluginFactory* DecaptchaPluginManager::getFactoryById(const QString &id) const {
    foreach (const DecaptchaPluginPair &pair, m_plugins) {
        if (pair.config->id() == id) {
            Logger::log("DecaptchaPluginManager::getFactoryById(). Factory found: " + id, Logger::HighVerbosity);
            return pair.factory;
        }
    }
    
    Logger::log("DecaptchaPluginManager::getFactoryById(). No factory found for id " + id, Logger::HighVerbosity);
    return 0;
}

DecaptchaPlugin* DecaptchaPluginManager::createPluginById(const QString &id, QObject *parent) {
    if (DecaptchaPluginFactory *factory = getFactoryById(id)) {
        if (DecaptchaPlugin* plugin = factory->createPlugin(parent)) {
            plugin->setNetworkAccessManager(networkAccessManager());
            return plugin;
        }
    }

    return 0;
}

QNetworkAccessManager* DecaptchaPluginManager::networkAccessManager() {
    return m_nam ? m_nam : m_nam = new QNetworkAccessManager(this);
}

int DecaptchaPluginManager::load() {
    Logger::log("DecaptchaPluginManager::load(): Loading plugins modified since "
                + m_lastLoaded.toString(Qt::ISODate), Logger::LowVerbosity);
    int count = 0;
    QDir dir;
    
    foreach (const QString &path, DECAPTCHA_PLUGIN_PATHS) {
        dir.setPath(path);
        
        foreach (const QFileInfo &info, dir.entryInfoList(QStringList() << "*.json", QDir::Files, QDir::Time)) {
            if (info.created() > m_lastLoaded) {
                DecaptchaPluginConfig *config = getConfigByFilePath(info.absoluteFilePath());
                
                if (!config) {
                    config = new DecaptchaPluginConfig(this);
                    
                    if (config->load(info.absoluteFilePath())) {
                        if (config->pluginType() == "js") {
                            JavaScriptDecaptchaPluginFactory *js =
                                new JavaScriptDecaptchaPluginFactory(config->pluginFilePath(),
                                        JavaScriptPluginEngine::instance(), this);
                            m_plugins << DecaptchaPluginPair(config, js);
                            ++count;
                            Logger::log("DecaptchaPluginManager::load(). JavaScript plugin loaded: "
                                        + config->id(), Logger::MediumVerbosity);
                        }
                        else {
                            QPluginLoader loader(config->pluginFilePath());
                            QObject *obj = loader.instance();
                            
                            if (obj) {
                                if (DecaptchaPluginFactory *factory = qobject_cast<DecaptchaPluginFactory*>(obj)) {
                                    m_plugins << DecaptchaPluginPair(config, factory);
                                    ++count;
                                    Logger::log("DecaptchaPluginManager::load(). Qt Plugin loaded: "
                                                + config->id(), Logger::MediumVerbosity);
                                }
                                else {
                                    loader.unload();
                                    Logger::log("DecaptchaPluginManager::load(). Error loading Qt plugin: "
                                                + config->id());
                                }
                            }
                            else {
                                Logger::log("DecaptchaPluginManager::load(). Qt plugin is NULL: " + config->id());
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
    
    Logger::log(QString("DecaptchaPluginManager::load() %1 plugins loaded").arg(count), Logger::LowVerbosity);

    if (count > 0) {
        qSort(m_plugins.begin(), m_plugins.end(), displayNameLessThan);
        emit countChanged(m_plugins.size());
    }

    m_lastLoaded = QDateTime::currentDateTime();
    return count;
}
