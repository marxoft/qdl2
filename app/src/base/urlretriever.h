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

#ifndef URLRETRIEVER_H
#define URLRETRIEVER_H

#include <QObject>
#include <QStringList>
#include <QUrl>

class UrlProcessor;
class QNetworkAccessManager;
class QNetworkReply;

class UrlRetriever : public QObject
{
    Q_OBJECT

    Q_PROPERTY(QString url READ url NOTIFY urlChanged)
    Q_PROPERTY(QString pluginId READ pluginId NOTIFY pluginIdChanged)
    Q_PROPERTY(Status status READ status NOTIFY statusChanged)
    Q_PROPERTY(QString statusString READ statusString NOTIFY statusChanged)
    Q_PROPERTY(QString errorString READ errorString NOTIFY finished)
    Q_PROPERTY(QStringList results READ results NOTIFY finished)

    Q_ENUMS(Status)

public:
    enum Status {
        Idle = 0,
        Active,
        Completed,
        Canceled,
        Failed
    };
    
    explicit UrlRetriever(QObject *parent = 0);
    ~UrlRetriever();

    QString url() const;

    QString pluginId() const;

    Status status() const;
    QString statusString() const;

    QString errorString() const;

    QStringList results() const;

public Q_SLOTS:
    void start(const QString &url, const QString &pluginId);
    void cancel();

private Q_SLOTS:
    void onReplyFinished();
    void onProcessorFinished(const QStringList &urls);

Q_SIGNALS:
    void urlChanged(const QString &url);
    void pluginIdChanged(const QString &id);
    void statusChanged(UrlRetriever::Status status);
    
    void finished(UrlRetriever *retriever);

private:
    void setUrl(const QString &u);

    void setPluginId(const QString &id);

    void setStatus(Status s);

    void setErrorString(const QString &e);

    void setResults(const QStringList &r);

    void followRedirect(const QUrl &url);

    UrlProcessor *m_processor;
    QNetworkAccessManager *m_nam;
    QNetworkReply *m_reply;

    QString m_url;
    QString m_pluginId;
    QString m_errorString;

    QStringList m_results;

    Status m_status;

    int m_redirects;
};

class UrlProcessor : public QObject
{
    Q_OBJECT

    friend class UrlRetriever;

private:
    UrlProcessor();
    ~UrlProcessor();

    void processUrls(const QString &baseUrl, const QString &response, const QString &pluginId);

Q_SIGNALS:
    void finished(const QStringList &urls);
};
  
#endif // URLRETRIEVER_H
