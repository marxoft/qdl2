/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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
#include "request.h"

ServicePluginManager* ServicePluginManager::self = 0;

ServicePluginManager::ServicePluginManager() :
    QObject()
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
    foreach (ServicePluginConfig *config, m_plugins) {
        if (config->id() == id) {
            return config;
        }
    }
    
    return 0;
}

ServicePluginConfig* ServicePluginManager::getConfigByUrl(const QString &url) const {
    foreach (ServicePluginConfig *config, m_plugins) {
        if (config->urlIsSupported(url)) {
            return config;
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

void ServicePluginManager::load() {
    Request *request = new Request(this);
    request->get("/services/getPlugins");
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
}

void ServicePluginManager::onRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        qDeleteAll(m_plugins);
        m_plugins.clear();
        const QVariantList plugins = request->result().toList();

        foreach (const QVariant &plugin, plugins) {
            ServicePluginConfig *config = new ServicePluginConfig(this);
            config->load(plugin.toMap());
            m_plugins << config;
        }

        emit countChanged(count());
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}
