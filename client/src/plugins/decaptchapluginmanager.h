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

#ifndef DECAPTCHAPLUGINMANAGER_H
#define DECAPTCHAPLUGINMANAGER_H

#include "decaptchapluginconfig.h"

typedef QList<DecaptchaPluginConfig*> DecaptchaPluginList;

class Request;

class DecaptchaPluginManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    ~DecaptchaPluginManager();

    static DecaptchaPluginManager* instance();
    
    int count() const;

    DecaptchaPluginList plugins() const;

public Q_SLOTS:
    DecaptchaPluginConfig* getConfigById(const QString &id) const;

    void load();

private Q_SLOTS:
    void onRequestFinished(Request *request);

Q_SIGNALS:
    void countChanged(int count);
    void error(const QString &errorString);

private:
    DecaptchaPluginManager();

    static DecaptchaPluginManager *self;

    DecaptchaPluginList m_plugins;
};

#endif // DECAPTCHAPLUGINMANAGER_H
