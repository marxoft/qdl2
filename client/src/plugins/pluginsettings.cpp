/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
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

#include "pluginsettings.h"
#include "decaptchapluginmanager.h"
#include "recaptchapluginmanager.h"
#include "request.h"
#include "servicepluginmanager.h"

PluginSettings* PluginSettings::self = 0;

PluginSettings::PluginSettings() :
    QObject()
{
}

PluginSettings::~PluginSettings() {
    self = 0;
}

PluginSettings* PluginSettings::instance() {
    return self ? self : self = new PluginSettings;
}

void PluginSettings::getSettings(const QString &pluginId) {
    QString path;

    if (DecaptchaPluginManager::instance()->getConfigById(pluginId)) {
        path = QString("/decaptcha/getPluginSettings");
    }
    else if (RecaptchaPluginManager::instance()->getConfigById(pluginId)) {
        path = QString("/recaptcha/getPluginSettings");
    }
    else {
        path = QString("/services/getPluginSettings");
    }

    QVariantMap params;
    params["id"] = pluginId;
    Request *request = new Request(this);
    request->get(path, params);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
}

void PluginSettings::setSettings(const QString &pluginId, const QVariantMap &settings) {
    QString path;

    if (DecaptchaPluginManager::instance()->getConfigById(pluginId)) {
        path = QString("/decaptcha/setPluginSettings");
    }
    else if (RecaptchaPluginManager::instance()->getConfigById(pluginId)) {
        path = QString("/recaptcha/setPluginSettings");
    }
    else {
        path = QString("/services/setPluginSettings");
    }

    QVariantMap params;
    params["id"] = pluginId;
    Request *request = new Request(this);
    request->put(path, params, settings);
    connect(request, SIGNAL(finished(Request*)), this, SLOT(onRequestFinished(Request*)));
}

void PluginSettings::onRequestFinished(Request *request) {
    if (request->status() == Request::Finished) {
        if (request->operation() == Request::GetOperation) {
            emit ready(request->params().value("id").toString(), request->result().toList());
        }
    }
    else if (request->status() == Request::Error) {
        emit error(request->errorString());
    }

    request->deleteLater();
}
