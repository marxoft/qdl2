/*
 * Copyright (C) 2018 Stuart Howarth <showarth@marxoft.co.uk>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef DOWNLOADREQUESTDIALOG_H
#define DOWNLOADREQUESTDIALOG_H

#include "downloadrequestmodel.h"
#include <QDialog>

class QDialogButtonBox;
class QGridLayout;
class QLabel;
class QProgressBar;
class QTreeView;

class DownloadRequestDialog : public QDialog
{
    Q_OBJECT

    Q_PROPERTY(QString resultsString READ resultsString)

public:
    explicit DownloadRequestDialog(QWidget *parent = 0);

    QString resultsString() const;

public Q_SLOTS:
    void addUrl(const QString &url);
    void addUrls(const QStringList &urls);
    
    void clear();

private Q_SLOTS:
    void showCaptchaDialog(int captchaType, const QByteArray &captchaData);
    void showPluginSettingsDialog(const QString &title, const QVariantList &settings);

    void updateStatusLabel();
    void onStatusChanged(DownloadRequestModel::Status status);
    
private:
    QTreeView *m_view;
    
    QProgressBar *m_progressBar;

    QLabel *m_statusLabel;

    QDialogButtonBox *m_buttonBox;

    QGridLayout *m_layout;
};

#endif // DOWNLOADREQUESTDIALOG_H
