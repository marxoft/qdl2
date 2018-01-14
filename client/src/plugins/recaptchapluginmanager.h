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

#ifndef RECAPTCHAPLUGINMANAGER_H
#define RECAPTCHAPLUGINMANAGER_H

#include "recaptchapluginconfig.h"

typedef QList<RecaptchaPluginConfig*> RecaptchaPluginList;

class Request;

class RecaptchaPluginManager : public QObject
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ count NOTIFY countChanged)

public:
    ~RecaptchaPluginManager();

    static RecaptchaPluginManager* instance();
    
    int count() const;

    RecaptchaPluginList plugins() const;

public Q_SLOTS:
    RecaptchaPluginConfig* getConfigById(const QString &id) const;

    void load();

private Q_SLOTS:
    void onRequestFinished(Request *request);

Q_SIGNALS:
    void countChanged(int count);
    void error(const QString &errorString);

private:
    RecaptchaPluginManager();

    static RecaptchaPluginManager *self;

    RecaptchaPluginList m_plugins;
};

#endif // RECAPTCHAPLUGINMANAGER_H
