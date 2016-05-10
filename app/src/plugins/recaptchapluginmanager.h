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

#ifndef RECAPTCHAPLUGINMANAGER_H
#define RECAPTCHAPLUGINMANAGER_H

#include "recaptchaplugin.h"
#include "recaptchapluginconfig.h"

class RecaptchaPluginManager : public QObject
{
    Q_OBJECT

public:
    ~RecaptchaPluginManager();

    static RecaptchaPluginManager* instance();

    QList<RecaptchaPluginConfig*> configs() const;
    QList<RecaptchaPlugin*> plugins() const;

public Q_SLOTS:
    RecaptchaPluginConfig* getConfigById(const QString &id) const;

    RecaptchaPlugin* getPlugin(RecaptchaPluginConfig *config) const;
    RecaptchaPlugin* getPluginById(const QString &id) const;

    RecaptchaPlugin* createPluginById(const QString &id, QObject *parent = 0) const;

    void load();

private:
    RecaptchaPluginManager();

    QNetworkAccessManager* networkAccessManager();

    static RecaptchaPluginManager *self;

    QNetworkAccessManager *m_nam;
    
    QHash<RecaptchaPluginConfig*, RecaptchaPlugin*> m_plugins;
};

#endif // RECAPTCHAPLUGINMANAGER_H
