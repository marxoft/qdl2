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

#include "decaptchapluginmanager.h"
#include "request.h"

DecaptchaPluginManager* DecaptchaPluginManager::self = 0;

DecaptchaPluginManager::DecaptchaPluginManager() :
    QObject()
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
    foreach (DecaptchaPluginConfig *config, m_plugins) {
        if (config->id() == id) {
            return config;
        }
    }
    
    return 0;
}

void DecaptchaPluginManager::load() {
    Request *request = new Request(this);
    request->get("/decaptcha/getPlugins");
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
}

void DecaptchaPluginManager::onRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        qDeleteAll(m_plugins);
        m_plugins.clear();
        const QVariantList plugins = request->result().toList();

        foreach (const QVariant &plugin, plugins) {
            DecaptchaPluginConfig *config = new DecaptchaPluginConfig(this);
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
