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

#ifndef SERVICEPLUGINMANAGER_H
#define SERVICEPLUGINMANAGER_H

#include "servicepluginconfig.h"

typedef QList<ServicePluginConfig*> ServicePluginList;

class Request;

class ServicePluginManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    ~ServicePluginManager();

    static ServicePluginManager* instance();
    
    int count() const;

    ServicePluginList plugins() const;

public Q_SLOTS:
    ServicePluginConfig* getConfigById(const QString &id) const;
    ServicePluginConfig* getConfigByUrl(const QString &url) const;

    bool urlIsSupported(const QString &url) const;

    void load();

Q_SIGNALS:
    void countChanged(int count);
    void error(const QString &errorString);

private Q_SLOTS:
    void onRequestFinished(Request *request);

private:
    ServicePluginManager();

    static ServicePluginManager *self;

    ServicePluginList m_plugins;
};

#endif // SERVICEPLUGINMANAGER_H
