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

#include "urlchecker.h"
#include "transferitem.h"
#include <QAbstractListModel>

struct UrlCheck
{
    UrlCheck() :
        createSubfolder(false),
        priority(TransferItem::NormalPriority),
        customCommandOverrideEnabled(false),
        startAutomatically(false),
        checked(false),
        ok(false)
    {
    }
    
    UrlCheck(const QString &u, const QString &cat = QString(), bool cs = false, int p = TransferItem::NormalPriority,
           const QString &com = QString(), bool ov = false, bool sa = false) :
        url(u),
        category(cat),
        createSubfolder(cs),
        priority(p),
        customCommand(com),
        customCommandOverrideEnabled(ov),
        startAutomatically(sa),
        checked(false),
        ok(false)
    {
    }
    
    QString url;
    QString category;
    bool createSubfolder;
    int priority;
    QString customCommand;
    bool customCommandOverrideEnabled;
    bool startAutomatically;
    bool checked;
    bool ok;
};

typedef QList<UrlCheck> UrlCheckList;

class UrlCheckModel : public QAbstractListModel
{
    Q_OBJECT
    
    Q_PROPERTY(int captchaType READ captchaType NOTIFY statusChanged)
    Q_PROPERTY(QByteArray captchaData READ captchaData NOTIFY statusChanged)
    Q_PROPERTY(int captchaTimeout READ captchaTimeout NOTIFY captchaTimeoutChanged)
    Q_PROPERTY(QString captchaTimeoutString READ captchaTimeoutString NOTIFY captchaTimeoutChanged)
    Q_PROPERTY(int count READ rowCount NOTIFY countChanged)
    Q_PROPERTY(int progress READ progress NOTIFY progressChanged)
    Q_PROPERTY(QVariantList requestedSettings READ requestedSettings NOTIFY statusChanged)
    Q_PROPERTY(int requestedSettingsTimeout READ requestedSettingsTimeout NOTIFY requestedSettingsTimeoutChanged)
    Q_PROPERTY(QString requestedSettingsTimeoutString READ requestedSettingsTimeoutString
               NOTIFY requestedSettingsTimeoutChanged)
    Q_PROPERTY(QString requestedSettingsTitle READ requestedSettingsTitle NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)

    Q_ENUMS(Roles Status)
    
public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
        CategoryRole,
        CreateSubfolderRole,
        PriorityRole,
        CustomCommandRole,
        CustomCommandOverrideEnabledRole,
        StartAutomaticallyRole,
        IsCheckedRole,
        IsOkRole
    };

    enum Status {
        Idle = 0,
        Active,
        RetrievingCaptchaChallenge,
        AwaitingCaptchaResponse,
        RetrievingCaptchaResponse,
        SubmittingCaptchaResponse,
        ReportingCaptchaResponse,
        AwaitingSettingsResponse,
        SubmittingSettingsResponse,
        WaitingActive,
        WaitingInactive,
        Completed,
        Canceled
    };

    ~UrlCheckModel();

    static UrlCheckModel* instance();

    int captchaType() const;
    QString captchaTypeString() const;
    QByteArray captchaData() const;
    int captchaTimeout() const;
    QString captchaTimeoutString() const;

    int progress() const;
    
    QVariantList requestedSettings() const;
    int requestedSettingsTimeout() const;
    QString requestedSettingsTimeoutString() const;
    QString requestedSettingsTitle() const;

    Status status() const;
    QString statusString() const;

    int waitTime() const;
    QString waitTimeString() const;
    
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
    void append(const QString &url, const QString &category = QString(), bool createSubfolder = false,
            int priority = TransferItem::NormalPriority, const QString &customCommand = QString(),
            bool overrideGlobalCommand = false, bool startAutomatically = false);
    void append(const QStringList &urls, const QString &category = QString(), bool createSubfolder = false,
            int priority = TransferItem::NormalPriority, const QString &customCommand = QString(),
            bool overrideGlobalCommand = false, bool startAutomatically = false);
    bool remove(int row);
    void cancel();
    void clear();
    
    bool submitCaptchaResponse(const QString &response);
    bool submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onStatusChanged(UrlChecker::Status s);
    void onUrlChecked(const UrlResult &result);
    void onUrlChecked(const UrlResultList &results, const QString &packageName);
    void onError(const QString &errorString);
    
Q_SIGNALS:
    void captchaRequest(int captchaType, const QByteArray &captchaData);
    void captchaTimeoutChanged(int timeout);
    void countChanged(int count);
    void progressChanged(int progress);
    void requestedSettingsTimeoutChanged(int timeout);
    void settingsRequest(const QString &title, const QVariantList &settings);
    void statusChanged(UrlCheckModel::Status status);
    void waitTimeChanged(int wait);
    
private:
    UrlCheckModel();

    UrlChecker* checker();

    void setStatus(Status s);

    void next();

    static UrlCheckModel *self;

    UrlChecker *m_checker;
        
    UrlCheckList m_items;
    
    QHash<int, QByteArray> m_roles;
    
    Status m_status;

    int m_index;
};

Q_DECLARE_METATYPE(UrlCheck)
Q_DECLARE_METATYPE(UrlCheckList)

#endif // URLCHECKMODEL_H
