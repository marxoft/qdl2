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

#ifndef DECAPTCHAPLUGINMANAGER_H
#define DECAPTCHAPLUGINMANAGER_H

#include "decaptchaplugin.h"
#include "decaptchapluginconfig.h"

class DecaptchaPluginManager : public QObject
{
    Q_OBJECT

public:
    ~DecaptchaPluginManager();

    static DecaptchaPluginManager* instance();

    QList<DecaptchaPluginConfig*> configs() const;
    QList<DecaptchaPlugin*> plugins() const;

public Q_SLOTS:
    DecaptchaPluginConfig* getConfigById(const QString &id) const;

    DecaptchaPlugin* getPlugin(DecaptchaPluginConfig *config) const;
    DecaptchaPlugin* getPluginById(const QString &id) const;

    DecaptchaPlugin* createPluginById(const QString &id, QObject *parent = 0) const;

    void load();

private:
    DecaptchaPluginManager();

    QNetworkAccessManager* networkAccessManager();

    static DecaptchaPluginManager *self;

    QNetworkAccessManager *m_nam;

    QHash<DecaptchaPluginConfig*, DecaptchaPlugin*> m_plugins;
};

#endif // DECAPTCHAPLUGINMANAGER_H
