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

#ifndef SEARCHMODEL_H
#define SEARCHMODEL_H

#include "searchresult.h"
#include <QAbstractListModel>

class SearchPlugin;
class QTimer;

class SearchModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)

    Q_ENUMS(Status)
    
public:
    enum Roles {
        NameRole = Qt::DisplayRole,
        IconFilePathRole = Qt::UserRole + 1,
        DescriptionRole,
        UrlRole
    };

    enum Status {
        Idle = 0,
        Active,
        AwaitingSettingsResponse,
        Completed,
        Canceled,
        Error
    };
    
    explicit SearchModel(QObject *parent = 0);
    
    QString errorString() const;

    Status status() const;
    QString statusString() const;
    
#if QT_VERSION >= 0x050000
    virtual QHash<int, QByteArray> roleNames() const;
#endif
    
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    
    virtual bool canFetchMore(const QModelIndex &parent = QModelIndex()) const;
    virtual void fetchMore(const QModelIndex &parent = QModelIndex());
    
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    
    Q_INVOKABLE virtual QVariant data(int row, const QByteArray &role) const;
    Q_INVOKABLE virtual QVariantMap itemData(int row) const;
    
    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
                                  Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;
    Q_INVOKABLE virtual int match(int start, const QByteArray &role, const QVariant &value,
                                  int flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;    

public Q_SLOTS:
    void search(const QString &query, const QString &pluginId);
    void cancel();
    void clear();
    void reload();
    
    bool submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:    
    void onSearchCompleted(const SearchResultList &results, const QString &next);
    void onSearchError(const QString &errorString);
    void onSearchSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback);
    
Q_SIGNALS:
    void countChanged(int count);
    void settingsRequest(const QString &title, const QVariantList &settings);
    void statusChanged(SearchModel::Status status);
    
private:
    void setErrorString(const QString &e);
    
    void setStatus(Status s);

    SearchPlugin* plugin();
    
    SearchPlugin *m_plugin;
    QString m_pluginId;
    
    QString m_query;
    QString m_errorString;
    
    SearchResultList m_items;
    QString m_next;
    
    Status m_status;
    
    QByteArray m_callback;
    
    QHash<int, QByteArray> m_roles;
};

#endif // SEARCHMODEL_H
