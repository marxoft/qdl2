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

#ifndef URLCHECKMODEL_H
#define URLCHECKMODEL_H

#include "urlresult.h"
#include <QAbstractListModel>

class ServicePlugin;
class QTimer;

struct UrlCheck
{
    UrlCheck() :
        checked(false),
        ok(false)
    {
    }
    
    UrlCheck(const QString &u) :
        url(u),
        checked(false),
        ok(false)
    {
    }
    
    QString url;
    QString fileName;
    
    bool checked;
    bool ok;
};

class UrlCheckModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QVariantList requestedSettings READ requestedSettings NOTIFY statusChanged)
    Q_PROPERTY(int requestedSettingsTimeout READ requestedSettingsTimeout NOTIFY requestedSettingsTimeoutChanged)
    Q_PROPERTY(QString requestedSettingsTimeoutString READ requestedSettingsTimeoutString NOTIFY requestedSettingsTimeoutChanged)
    Q_PROPERTY(QString requestedSettingsTitle READ requestedSettingsTitle NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)

    Q_ENUMS(Status)
    
public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
        FileNameRole,
        IsCheckedRole,
        IsOkRole
    };

    enum Status {
        Idle = 0,
        Active,
        AwaitingSettingsResponse,
        Completed,
        Canceled
    };

    ~UrlCheckModel();

    static UrlCheckModel* instance();

    int progress() const;
    
    QVariantList requestedSettings() const;
    int requestedSettingsTimeout() const;
    QString requestedSettingsTimeoutString() const;
    QString requestedSettingsTitle() const;

    Status status() const;
    QString statusString() const;
    
#if QT_VERSION >= 0x050000
    virtual QHash<int, QByteArray> roleNames() const;
#endif
    
    virtual int rowCount(const QModelIndex &parent = QModelIndex()) const;
    virtual int columnCount(const QModelIndex &parent = QModelIndex()) const;
    
    virtual QVariant headerData(int section, Qt::Orientation orientation = Qt::Horizontal,
                                int role = Qt::DisplayRole) const;
    
    virtual QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;
    virtual QMap<int, QVariant> itemData(const QModelIndex &index) const;
    
    Q_INVOKABLE virtual QVariant data(int row, const QByteArray &role) const;
    Q_INVOKABLE virtual QVariantMap itemData(int row) const;
    
    virtual QModelIndexList match(const QModelIndex &start, int role, const QVariant &value, int hits = 1,
                                  Qt::MatchFlags flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;
    Q_INVOKABLE virtual int match(int start, const QByteArray &role, const QVariant &value,
                                  int flags = Qt::MatchFlags(Qt::MatchExactly | Qt::MatchWrap)) const;    

public Q_SLOTS:
    void append(const QString &url);
    void append(const QStringList &urls);
    bool remove(int row);
    void cancel();
    void clear();
    
    bool submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void updateRequestedSettingsTimeout();
    
    void onUrlChecked(const UrlResult &result);
    void onUrlChecked(const UrlResultList &results, const QString &packageName);
    void onUrlCheckError(const QString &errorString);
    void onUrlCheckSettingsRequest(const QString &title, const QVariantList &settings, const QByteArray &callback);
    
Q_SIGNALS:
    void countChanged(int count);
    void progressChanged(int progress);
    void requestedSettingsTimeoutChanged(int timeout);
    void settingsRequest(const QString &title, const QVariantList &settings);
    void statusChanged(UrlCheckModel::Status status);
    
private:
    UrlCheckModel();
    
    void setRequestedSettings(const QString &title, const QVariantList &settings, const QByteArray &callback);
    void clearRequestedSettings();
    
    void startRequestedSettingsTimer();
    void stopRequestedSettingsTimer();

    void setStatus(Status s);

    ServicePlugin* getCurrentPlugin() const;
    
    void next();

    static UrlCheckModel *self;
        
    QList<UrlCheck> m_items;
    
    QHash<int, QByteArray> m_roles;
    
    QTimer *m_timer;
    
    QString m_requestedSettingsTitle;
    QVariantList m_requestedSettings;
    QByteArray m_requestedSettingsCallback;
    int m_requestedSettingsTimeout;

    Status m_status;

    int m_index;
};

#endif // URLCHECKMODEL_H
