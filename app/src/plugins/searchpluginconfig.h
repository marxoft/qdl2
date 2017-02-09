/*
 * Copyright (C) 2017 Stuart Howarth <showarth@marxoft.co.uk>
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

#ifndef SEARCHPLUGINCONFIG_H
#define SEARCHPLUGINCONFIG_H

#include <QObject>
#include <QVariantList>

class SearchPluginConfig : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString displayName READ displayName NOTIFY changed)
    Q_PROPERTY(QString filePath READ filePath NOTIFY changed)
    Q_PROPERTY(QString iconFilePath READ iconFilePath NOTIFY changed)
    Q_PROPERTY(QString id READ id NOTIFY changed)
    Q_PROPERTY(QString pluginFilePath READ pluginFilePath NOTIFY changed)
    Q_PROPERTY(QString pluginType READ pluginType NOTIFY changed)
    Q_PROPERTY(QVariantList settings READ settings NOTIFY changed)
    Q_PROPERTY(int version READ version NOTIFY changed)

public:
    explicit SearchPluginConfig(QObject *parent = 0);

    QString displayName() const;
    
    QString filePath() const;

    QString iconFilePath() const;
    
    QString id() const;

    QString pluginFilePath() const;
    
    QString pluginType() const;
        
    QVariantList settings() const;
    
    int version() const;

public Q_SLOTS:
    bool load(const QString &filePath);

Q_SIGNALS:
    void changed();

private:
    QString m_displayName;
    QString m_filePath;
    QString m_iconFilePath;
    QString m_id;
    QString m_pluginFilePath;
    QString m_pluginType;
        
    QVariantList m_settings;
    
    int m_version;
};

#endif // SEARCHPLUGINCONFIG_H
