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

#ifndef DOWNLOADREQUESTMODEL_H
#define DOWNLOADREQUESTMODEL_H

#include "downloadrequester.h"
#include "urlchecker.h"
#include <QAbstractListModel>
#include <QNetworkRequest>

struct DownloadResult
{
    DownloadResult() :
        fileName(QString()),
        request(QNetworkRequest()),
        method(QByteArray("GET")),
        data(QByteArray())
    {
    }

    DownloadResult(const QString &f, const QNetworkRequest &r, const QByteArray &m = QByteArray("GET"),
            const QByteArray &d = QByteArray()) :
        fileName(f),
        request(r),
        method(m),
        data(d)
    {
    }
    
    QString fileName;
    QNetworkRequest request;
    QByteArray method;
    QByteArray data;
};

typedef QList<DownloadResult> DownloadResultList;

struct DownloadRequest
{
    DownloadRequest() :
        checked(false),
        ok(false)
    {
    }

    DownloadRequest(const QString &u) :
        url(u),
        checked(false),
        ok(false)
    {
    }

    QString url;
    bool checked;
    bool ok;
    DownloadResultList results;
};

typedef QList<DownloadRequest> DownloadRequestList;

class DownloadRequestModel : public QAbstractListModel
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
    Q_PROPERTY(DownloadResultList results READ results NOTIFY statusChanged)
    Q_PROPERTY(QString resultsString READ resultsString NOTIFY statusChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)

    Q_ENUMS(Roles Status)
    
public:
    enum Roles {
        UrlRole = Qt::UserRole + 1,
        IsCheckedRole,
        IsOkRole,
        ResultsRole,
        ResultsStringRole
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

    ~DownloadRequestModel();

    static DownloadRequestModel* instance();

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

    DownloadResultList results() const;
    QString resultsString() const;

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
    void append(const QString &url);
    void append(const QStringList &urls);
    bool remove(int row);
    void cancel();
    void clear();
    
    bool submitCaptchaResponse(const QString &response);
    bool submitSettingsResponse(const QVariantMap &settings);

private Q_SLOTS:
    void onCheckerStatusChanged(UrlChecker::Status s);
    void onRequesterStatusChanged(DownloadRequester::Status s);
    void onCaptchaRequest(int captchaType, const QByteArray &captchaData);
    void onCaptchaTimeoutChanged(int timeout);
    void onRequestedSettingsTimeoutChanged(int timeout);
    void onSettingsRequest(const QString &title, const QVariantList &settings);
    void onWaitTimeChanged(int wait);
    void onUrlChecked(const UrlResult &result);
    void onUrlChecked(const UrlResultList &results, const QString &packageName);
    void onDownloadRequest(const QNetworkRequest &request, const QByteArray &method, const QByteArray &data);
    void onCheckerError(const QString &errorString);
    void onRequesterError(const QString &errorString);
    
Q_SIGNALS:
    void captchaRequest(int captchaType, const QByteArray &captchaData);
    void captchaTimeoutChanged(int timeout);
    void countChanged(int count);
    void progressChanged(int progress);
    void requestedSettingsTimeoutChanged(int timeout);
    void settingsRequest(const QString &title, const QVariantList &settings);
    void statusChanged(DownloadRequestModel::Status status);
    void waitTimeChanged(int wait);
    
private:
    DownloadRequestModel();

    UrlChecker* checker();
    DownloadRequester* requester();

    void setCaptchaData(int captchaType, const QByteArray &captchaData);
    void clearCaptchaData();

    void setRequestedSettings(const QString &title, const QVariantList &settings);
    void clearRequestedSettings();

    void setStatus(Status s);

    void nextUrlCheck();
    void nextDownloadRequest();

    static DownloadRequestModel *self;

    UrlChecker *m_checker;
    DownloadRequester *m_requester;
        
    DownloadRequestList m_items;
    UrlResultList m_urls;
    
    QHash<int, QByteArray> m_roles;

    int m_captchaType;
    QByteArray m_captchaData;

    QString m_requestedSettingsTitle;
    QVariantList m_requestedSettings;

    int m_timeRemaining;
    
    Status m_status;

    int m_index;
};

Q_DECLARE_METATYPE(DownloadRequest)
Q_DECLARE_METATYPE(DownloadRequestList)
Q_DECLARE_METATYPE(DownloadResult)
Q_DECLARE_METATYPE(DownloadResultList)

#endif // DOWNLOADREQUESTMODEL_H
